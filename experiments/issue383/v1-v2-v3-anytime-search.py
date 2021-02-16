#! /usr/bin/env python
# -*- coding: utf-8 -*-


import os

from lab.environments import LocalEnvironment, BaselSlurmEnvironment
from lab.reports import Attribute

import common_setup
from common_setup import IssueConfig, IssueExperiment


def make_comparison_table():
    report = common_setup.ComparativeReport(
        algorithm_pairs=[
            ("issue383-base-lama-no-syn-pref-false", "issue383-v1-lama-no-syn-pref-false"),
            ("issue383-base-lama-no-syn-pref-false", "issue383-v2-lama-no-syn-pref-false"),
            ("issue383-base-lama-no-syn-pref-false", "issue383-v3-lama-no-syn-pref-false"),
            ("issue383-base-lama-no-syn-pref-true", "issue383-v1-lama-no-syn-pref-true"),
            ("issue383-base-lama-no-syn-pref-true", "issue383-v2-lama-no-syn-pref-true"),
            ("issue383-base-lama-no-syn-pref-true", "issue383-v3-lama-no-syn-pref-true"),
        ], attributes=ATTRIBUTES,
    )
    outfile = os.path.join(
        exp.eval_dir, "%s-compare.%s" % (exp.name, report.output_format)
    )
    report(exp.eval_dir, outfile)
    exp.add_report(report)


DIR = os.path.dirname(os.path.abspath(__file__))
SCRIPT_NAME = os.path.splitext(os.path.basename(__file__))[0]
BENCHMARKS_DIR = os.environ["DOWNWARD_BENCHMARKS"]
REVISIONS = ["issue383-base", "issue383-v1", "issue383-v2", "issue383-v3"]
ATTRIBUTES = IssueExperiment.DEFAULT_TABLE_ATTRIBUTES + [
    Attribute("landmarks", min_wins=False),
    Attribute("simple_landmarks", min_wins=False),
    Attribute("disjunctive_landmarks", min_wins=False),
    Attribute("conjunctive_landmarks", min_wins=False),
    Attribute("orderings", min_wins=False),
    Attribute("natural_orders", min_wins=False),
    Attribute("necessary_orders", min_wins=False),
    Attribute("greedy_necessary_orders", min_wins=False),
    Attribute("reasonable_orders", min_wins=False),
    Attribute("obedient_reasonable_orders", min_wins=False),
]

CONFIGS = [
    IssueConfig(f"lama-no-syn-pref-{pref}", [
        "--if-unit-cost", "--evaluator",
        f"hlm=lmcount(lm_rhw(reasonable_orders=true), pref={pref})",
        "--evaluator", "hff=ff()",
        "--search", """iterated([
                        lazy_greedy([hff,hlm], preferred=[hff,hlm]),
                        lazy_wastar([hff,hlm], preferred=[hff,hlm], w=5),
                        lazy_wastar([hff,hlm], preferred=[hff,hlm], w=3),
                        lazy_wastar([hff,hlm], preferred=[hff,hlm], w=2),
                        lazy_wastar([hff,hlm], preferred=[hff,hlm], w=1)
                        ], repeat_last=true, continue_on_fail=true)""",
        "--if-non-unit-cost",
        "--evaluator",
        f"hlm1=lmcount(lm_rhw(reasonable_orders=true), transform=adapt_costs(one), pref={pref})",
        "--evaluator", "hff1=ff(transform=adapt_costs(one))",
        "--evaluator",
        f"hlm2=lmcount(lm_rhw(reasonable_orders=true), transform=adapt_costs(plusone), pref={pref})",
        "--evaluator", "hff2=ff(transform=adapt_costs(plusone))",
        "--search", """iterated([
                        lazy_greedy([hff1,hlm1], preferred=[hff1,hlm1],
                                    cost_type=one, reopen_closed=false),
                        lazy_greedy([hff2,hlm2], preferred=[hff2,hlm2],
                                    reopen_closed=false),
                        lazy_wastar([hff2,hlm2], preferred=[hff2,hlm2], w=5),
                        lazy_wastar([hff2,hlm2], preferred=[hff2,hlm2], w=3),
                        lazy_wastar([hff2,hlm2], preferred=[hff2,hlm2], w=2),
                        lazy_wastar([hff2,hlm2], preferred=[hff2,hlm2], w=1)
                        ], repeat_last=true, continue_on_fail=true)""",
        "--always"])
    for pref in ["true", "false"]
]

if common_setup.is_test_run() or not common_setup.is_running_on_cluster():
    SUITE = IssueExperiment.DEFAULT_TEST_SUITE
    ENVIRONMENT = LocalEnvironment(processes=2)
else:
    SUITE = common_setup.DEFAULT_SATISFICING_SUITE
    ENVIRONMENT = BaselSlurmEnvironment(
        partition="infai_2",
        email="clemens.buechner@unibas.ch",
        export=["PATH", "DOWNWARD_BENCHMARKS"],
    )

exp = common_setup.IssueExperiment(
    revisions=REVISIONS,
    configs=CONFIGS,
    environment=ENVIRONMENT,
)

exp.add_suite(BENCHMARKS_DIR, SUITE)

exp.add_parser(exp.EXITCODE_PARSER)
exp.add_parser(exp.PLANNER_PARSER)
exp.add_parser(exp.ANYTIME_SEARCH_PARSER)
exp.add_parser("landmark_parser.py")

exp.add_step("build", exp.build)
exp.add_step("start", exp.start_runs)
exp.add_fetcher(name="fetch")
exp.add_step("comparison table", make_comparison_table)

exp.add_parse_again_step()

exp.run_steps()
