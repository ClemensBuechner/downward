#! /usr/bin/env python
# -*- coding: utf-8 -*-

import os

from lab.environments import LocalEnvironment, BaselSlurmEnvironment
from lab.reports import Attribute, arithmetic_mean

from average_report import AverageAlgorithmReport
import common_setup
from common_setup import IssueConfig, IssueExperiment

ISSUE = "cegar-pdb-goals"
ARCHIVE_PATH = f"buechner/downward/{ISSUE}"
DIR = os.path.dirname(os.path.abspath(__file__))
BENCHMARKS_DIR = os.environ["DOWNWARD_BENCHMARKS"]
REVISIONS = [
    f"{ISSUE}-base",
    f"{ISSUE}-v1",
]
GLOBAL_DRIVER_OPTIONS = []
BUILDS = ["release"]

SUITE = common_setup.DEFAULT_OPTIMAL_SUITE
ENVIRONMENT = BaselSlurmEnvironment(
    partition="infai_3",
    email="clemens.buechner@unibas.ch",
    export=["PATH"])
"""
If your experiments sometimes have GCLIBX errors, you can use the
below "setup" parameter instead of the above use "export" parameter for
setting environment variables which "load" the right modules. ("module
load" doesn't do anything else than setting environment variables.)

paths obtained via:
$ module purge
$ module -q load CMake/3.15.3-GCCcore-8.3.0
$ module -q load GCC/8.3.0
$ echo $PATH
$ echo $LD_LIBRARY_PATH
"""
# setup='export PATH=/scicore/soft/apps/binutils/2.32-GCCcore-8.3.0/bin:/scicore/soft/apps/CMake/3.15.3-GCCcore-8.3.0/bin:/scicore/soft/apps/cURL/7.66.0-GCCcore-8.3.0/bin:/scicore/soft/apps/bzip2/1.0.8-GCCcore-8.3.0/bin:/scicore/soft/apps/ncurses/6.1-GCCcore-8.3.0/bin:/scicore/soft/apps/GCCcore/8.3.0/bin:/infai/sieverss/repos/bin:/infai/sieverss/local:/export/soft/lua_lmod/centos7/lmod/lmod/libexec:/usr/local/bin:/usr/bin:/usr/local/sbin:/usr/sbin:$PATH\nexport LD_LIBRARY_PATH=/scicore/soft/apps/binutils/2.32-GCCcore-8.3.0/lib:/scicore/soft/apps/cURL/7.66.0-GCCcore-8.3.0/lib:/scicore/soft/apps/bzip2/1.0.8-GCCcore-8.3.0/lib:/scicore/soft/apps/zlib/1.2.11-GCCcore-8.3.0/lib:/scicore/soft/apps/ncurses/6.1-GCCcore-8.3.0/lib:/scicore/soft/apps/GCCcore/8.3.0/lib64:/scicore/soft/apps/GCCcore/8.3.0/lib'
SEEDS = range(2023, 2033)

if common_setup.is_test_run():
    SUITE = IssueExperiment.DEFAULT_TEST_SUITE
    ENVIRONMENT = LocalEnvironment(processes=4)
    SEEDS = range(2023, 2025)

CONFIG_NICKS = []
for seed in SEEDS:
    CONFIG_NICKS += [
        (f"cpdbs-s{seed}", ["--search", f"astar(cpdbs(patterns=multiple_cegar(random_seed={seed})))"], []),
        (f"zopdbs-s{seed}", ["--search", f"astar(zopdbs(patterns=multiple_cegar(random_seed={seed})))"], []),
    ]

CONFIGS = [
    IssueConfig(
        config_nick,
        config,
        build_options=[build],
        driver_options=GLOBAL_DRIVER_OPTIONS + driver_opts,
    )
    for build in BUILDS
    for config_nick, config, driver_opts in CONFIG_NICKS
]

exp = IssueExperiment(
    revisions=REVISIONS,
    configs=CONFIGS,
    environment=ENVIRONMENT,
)

exp.add_suite(BENCHMARKS_DIR, SUITE)

exp.add_parser(exp.EXITCODE_PARSER)
exp.add_parser(exp.TRANSLATOR_PARSER)
exp.add_parser(exp.SINGLE_SEARCH_PARSER)
exp.add_parser(exp.PLANNER_PARSER)
exp.add_parser("parser.py")

ATTRIBUTES = IssueExperiment.DEFAULT_TABLE_ATTRIBUTES + [
    Attribute("num_iterations_multiple", min_wins=True),
    Attribute("avg_time_per_generator", min_wins=True),
    Attribute("num_abstractions", absolute=True, min_wins=False),
    Attribute("multiple_computation_time", min_wins=True),
    Attribute("blacklisting_enabled", min_wins=False),
]

exp.add_step("build", exp.build)
exp.add_step("start", exp.start_runs)
exp.add_fetcher(name="fetch")

exp.add_report(
    AverageAlgorithmReport(
        algo_name_seeds=[f"-s{seed}" for seed in SEEDS],
        num_seeds=len(SEEDS),
        attributes=[
            "avg_time_per_generator", "cost", "coverage", "initial_h_value",
            "expansions_until_last_jump",
            "num_abstractions", "score_evaluations", "score_expansions",
            "score_generated", "score_memory", "score_search_time",
            "score_total_time", "search_time", "total_time",
        ],
    ),
    outfile=os.path.join(exp.eval_dir, "average-eval", "properties"),
    name="report-average",
)
# exp.add_fetcher(f"{exp.eval_dir}/average-eval", name="fetch-average")
exp.configs = []
for rev in REVISIONS:
    exp.configs += [
        f"{rev}-cpdbs",
        f"{rev}-zopdbs",
    ]
exp.add_comparison_table_step(attributes=ATTRIBUTES)

exp.add_archive_step(ARCHIVE_PATH)
# exp.add_archive_eval_dir_step(ARCHIVE_PATH)

exp.run_steps()
