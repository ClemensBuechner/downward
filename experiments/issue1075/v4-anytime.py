#! /usr/bin/env python3

import os

from lab.environments import LocalEnvironment, BaselSlurmEnvironment
from lab.reports import Attribute

import common_setup
from common_setup import IssueConfig, IssueExperiment

ARCHIVE_PATH = "ai/downward/issue1075"
DIR = os.path.dirname(os.path.abspath(__file__))
REPO_DIR = os.environ["DOWNWARD_REPO"]
BENCHMARKS_DIR = os.environ["DOWNWARD_BENCHMARKS"]
REVISIONS = [
    "issue1075-v4",
]

CONFIGS = []

for method in ["legacy", "future", "parents_past"]:
    for simple in ["true", "false"]:
        CONFIGS += [
            IssueConfig(f"lama-pref-method={method}-simple={simple}",
                        ["--if-unit-cost",
                         "--evaluator",
                         f"hlm=landmark_sum(lm_reasonable_orders_hps(lm_rhw()),pref=true,interesting_if={method},prefer_simple_landmarks={simple})",
                         "--evaluator", "hff=ff()",
                         "--search", """iterated([
                         lazy_greedy([hff,hlm],preferred=[hff,hlm]),
                         lazy_wastar([hff,hlm],preferred=[hff,hlm],w=5),
                         lazy_wastar([hff,hlm],preferred=[hff,hlm],w=3),
                         lazy_wastar([hff,hlm],preferred=[hff,hlm],w=2),
                         lazy_wastar([hff,hlm],preferred=[hff,hlm],w=1)
                         ],repeat_last=true,continue_on_fail=true)""",
                         "--if-non-unit-cost",
                         "--evaluator",
                         f"hlm1=landmark_sum(lm_reasonable_orders_hps(lm_rhw()),transform=adapt_costs(one),pref=true,interesting_if={method},prefer_simple_landmarks={simple})",
                         "--evaluator", "hff1=ff(transform=adapt_costs(one))",
                         "--evaluator",
                         f"hlm2=landmark_sum(lm_reasonable_orders_hps(lm_rhw()),transform=adapt_costs(plusone),pref=true,interesting_if={method},prefer_simple_landmarks={simple})",
                         "--evaluator", "hff2=ff(transform=adapt_costs(plusone))",
                         "--search", """iterated([
                         lazy_greedy([hff1,hlm1],preferred=[hff1,hlm1],
                                     cost_type=one,reopen_closed=false),
                         lazy_greedy([hff2,hlm2],preferred=[hff2,hlm2],
                                     reopen_closed=false),
                         lazy_wastar([hff2,hlm2],preferred=[hff2,hlm2],w=5),
                         lazy_wastar([hff2,hlm2],preferred=[hff2,hlm2],w=3),
                         lazy_wastar([hff2,hlm2],preferred=[hff2,hlm2],w=2),
                         lazy_wastar([hff2,hlm2],preferred=[hff2,hlm2],w=1)
                         ],repeat_last=true,continue_on_fail=true)""",
                         # Append --always to be on the safe side if we want to append
                         # additional options later.
                         "--always"])
        ]


SUITE = common_setup.DEFAULT_SATISFICING_SUITE
ENVIRONMENT = BaselSlurmEnvironment(
    partition="infai_2",
    email="clemens.buechner@unibas.ch",
    export=["PATH", "DOWNWARD_BENCHMARKS"])

if common_setup.is_test_run():
    SUITE = IssueExperiment.DEFAULT_TEST_SUITE
    ENVIRONMENT = LocalEnvironment(processes=3)

exp = IssueExperiment(
    revisions=REVISIONS,
    configs=CONFIGS,
    environment=ENVIRONMENT,
)
exp.add_suite(BENCHMARKS_DIR, SUITE)

exp.add_parser(exp.EXITCODE_PARSER)
exp.add_parser(exp.ANYTIME_SEARCH_PARSER)
exp.add_parser(exp.PLANNER_PARSER)
exp.add_parser("landmark_parser.py")

ATTRIBUTES = IssueExperiment.DEFAULT_TABLE_ATTRIBUTES + [
    Attribute("landmarks", min_wins=False),
    Attribute("landmarks_disjunctive", min_wins=False),
    Attribute("landmarks_conjunctive", min_wins=False),
    Attribute("orderings", min_wins=False),
    Attribute("lmgraph_generation_time", min_wins=True),
]

exp.add_step('build', exp.build)
exp.add_step('start', exp.start_runs)
exp.add_fetcher(name='fetch', merge=True)

exp.add_absolute_report_step(attributes=ATTRIBUTES)
exp.add_scatter_plot_step(relative=True, attributes=["cost"])
exp.add_scatter_plot_step(relative=False, attributes=["cost"])

exp.add_archive_step(ARCHIVE_PATH)

exp.run_steps()