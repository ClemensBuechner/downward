#! /usr/bin/env python
# -*- coding: utf-8 -*-


import os

from lab.environments import LocalEnvironment, BaselSlurmEnvironment

import common_setup
from common_setup import IssueConfig, IssueExperiment

DIR = os.path.dirname(os.path.abspath(__file__))
SCRIPT_NAME = os.path.splitext(os.path.basename(__file__))[0]
BENCHMARKS_DIR = os.environ["DOWNWARD_BENCHMARKS"]
REVISIONS = ["issue988-base", "issue1000-v1", "issue1000-v2",
             "issue1000-v3", "issue1000-v4", "issue1000-v5"]

CONFIGS = [
    IssueConfig("lama-first", [],
                driver_options=["--alias", "lama-first"]),
]

SUITE = common_setup.DEFAULT_OPTIMAL_SUITE
ENVIRONMENT = BaselSlurmEnvironment(
    partition="infai_2",
    email="clemens.buechner@unibas.ch",
    export=["PATH", "DOWNWARD_BENCHMARKS"],
)

if common_setup.is_test_run():
    SUITE = IssueExperiment.DEFAULT_TEST_SUITE
    ENVIRONMENT = LocalEnvironment(processes=2)

exp = common_setup.IssueExperiment(
    revisions=REVISIONS,
    configs=CONFIGS,
    environment=ENVIRONMENT,
)

exp.add_suite(BENCHMARKS_DIR, SUITE)

exp.add_parser(exp.ANYTIME_SEARCH_PARSER)
exp.add_parser(exp.EXITCODE_PARSER)
exp.add_parser(exp.PLANNER_PARSER)
exp.add_parser(exp.SINGLE_SEARCH_PARSER)

exp.add_step("build", exp.build)
exp.add_step("start", exp.start_runs)
exp.add_fetcher(name="fetch")
exp.add_report(common_setup.ComparativeReport(
    algorithm_pairs=[
        ("issue988-base-lama-first", "issue1000-v1-lama-first"),
        ("issue988-base-lama-first", "issue1000-v2-lama-first"),
        ("issue988-base-lama-first", "issue1000-v3-lama-first"),
        ("issue988-base-lama-first", "issue1000-v4-lama-first"),
        ("issue988-base-lama-first", "issue1000-v5-lama-first"),
    ], attributes=IssueExperiment.DEFAULT_TABLE_ATTRIBUTES,

))
exp.add_scatter_plot_step(relative=True, attributes=["total_time"])

exp.run_steps()
