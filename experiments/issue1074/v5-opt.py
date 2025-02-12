#! /usr/bin/env python3

import os

from lab.environments import LocalEnvironment, BaselSlurmEnvironment
from lab.reports import Attribute

from downward.reports.compare import ComparativeReport

import common_setup
from common_setup import IssueConfig, IssueExperiment
from landmark_parser import LandmarkParser

ISSUE = "issue1074"
ARCHIVE_PATH = f"ai/downward/{ISSUE}"
DIR = os.path.dirname(os.path.abspath(__file__))
BENCHMARKS_DIR = os.environ["DOWNWARD_BENCHMARKS"]
REVISIONS = [
    f"{ISSUE}-base",
    f"{ISSUE}-v5",
]
GLOBAL_DRIVER_OPTIONS = []
BUILDS = ["release"]
CONFIG_NICKS = []
FACTORIES = [
    ("lm-exhaust", "lm_exhaust()"),
    ("lm-exhaust-unary", "lm_exhaust(use_unary_relaxation=true)"),
    ("lm-exhaust-causal", "lm_exhaust(only_causal_landmarks=true)"),
]
for name, factory in FACTORIES:
    cp = "optimal" if name == "lm-exhaust" else "uniform"
    heuristic = f"landmark_cost_partitioning({factory}, cost_partitioning={cp})"
    CONFIG_NICKS.append((name, ["--evaluator", f"lmc={heuristic}", "--search",
                               "astar(lmc, lazy_evaluator=lmc)"]))
CONFIGS = [
    IssueConfig(
        config_nick,
        config,
        build_options=[build],
        driver_options=GLOBAL_DRIVER_OPTIONS,
    )
    for build in BUILDS
    for config_nick, config in CONFIG_NICKS
] + [
    IssueConfig("bjolp", [], build_options=[build],
                driver_options=GLOBAL_DRIVER_OPTIONS + [
                    "--alias", "seq-opt-bjolp"])
    for build in BUILDS
]

SUITE = common_setup.DEFAULT_OPTIMAL_SUITE
ENVIRONMENT = BaselSlurmEnvironment(
    partition="infai_2",
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

UNSUPPORTED = []
def collect_unsupported(run):
    if run["error"] == "search-unsupported":
        UNSUPPORTED.append(
            (run["algorithm"].split(":")[0], run["domain"], run["problem"]))
    return True

def filter_unsupported(run):
    for algo, domain, problem in UNSUPPORTED:
        if algo in run["algorithm"] and domain == run["domain"] and problem == run["problem"]:
            return False
    return True

if common_setup.is_test_run():
    SUITE = IssueExperiment.DEFAULT_TEST_SUITE + ["assembly:prob01.pddl"]
    ENVIRONMENT = LocalEnvironment(processes=4)

exp = IssueExperiment(
    revisions=REVISIONS,
    configs=CONFIGS,
    environment=ENVIRONMENT,
)
exp.add_suite(BENCHMARKS_DIR, SUITE)

exp.add_parser(exp.EXITCODE_PARSER)
exp.add_parser(exp.SINGLE_SEARCH_PARSER)
exp.add_parser(exp.PLANNER_PARSER)
exp.add_parser(LandmarkParser())

ATTRIBUTES = IssueExperiment.DEFAULT_TABLE_ATTRIBUTES + [
    Attribute("landmarks", min_wins=False),
    Attribute("landmarks_disjunctive", min_wins=False),
    Attribute("landmarks_conjunctive", min_wins=False),
    Attribute("orderings", min_wins=False),
    Attribute("lmgraph_generation_time", min_wins=True),
]

exp.add_step("build", exp.build)
exp.add_step("start", exp.start_runs)
exp.add_step("parse", exp.parse)
exp.add_fetcher(name="fetch")

def make_comparison_tables():
    compared_configs = [
        (f"{REVISIONS[0]}-lm-exhaust", f"{REVISIONS[1]}-lm-exhaust", "lm-exhaust"),
        (f"{REVISIONS[0]}-lm-exhaust-causal", f"{REVISIONS[1]}-lm-exhaust-unary", "lm-exhaust-unary"),
        (f"{REVISIONS[0]}-bjolp", f"{REVISIONS[1]}-bjolp", "bjolp"),
    ]
    report = ComparativeReport(compared_configs, attributes=ATTRIBUTES,
                               filter=[collect_unsupported, filter_unsupported])
    outfile = os.path.join(
        exp.eval_dir, f"{exp.name}-compare.{report.output_format}")
    report(exp.eval_dir, outfile)
exp.add_step("make-comparison-tables", make_comparison_tables)

exp.add_scatter_plot_step(relative=True, attributes=["total_time", "memory"])

exp.add_archive_step(ARCHIVE_PATH)
exp.add_archive_eval_dir_step(ARCHIVE_PATH)

exp.run_steps()

