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
            IssueConfig(
                f"lama-first-method={method}-simple={simple}",
                ["--evaluator",
                 "hlm=landmark_sum(lm_factory=lm_reasonable_orders_hps("
                 f"lm_rhw()),transform=adapt_costs(one),pref=true,interesting_if={method}, prefer_simple_landmarks={simple})",
                 "--evaluator", "hff=ff(transform=adapt_costs(one))",
                 "--search",
                 "lazy_greedy([hff,hlm],preferred=[hff,hlm],cost_type=one,reopen_closed=false,boost=0)"]),
            # IssueConfig(
            #     f"lm_hm-method={method}-simple={simple}",
            #     ["--evaluator",
            #      f"hlm=landmark_sum(lm_factory=lm_hm(m=1),pref=true,interesting_if={method}, prefer_simple_landmarks={simple})",
            #      "--search",
            #      "eager_greedy([hlm],preferred=[hlm])"]),
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
exp.add_parser(exp.SINGLE_SEARCH_PARSER)
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
exp.add_scatter_plot_step(relative=True, attributes=["search_time", "cost"])
exp.add_scatter_plot_step(relative=False, attributes=["search_time", "cost"])

exp.add_archive_step(ARCHIVE_PATH)

exp.run_steps()