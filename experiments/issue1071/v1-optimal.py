#! /usr/bin/env python
# -*- coding: utf-8 -*-

import common_setup
import os

from common_setup import IssueConfig, IssueExperiment
from lab.environments import LocalEnvironment, BaselSlurmEnvironment
from lab.reports import Attribute

ISSUE = "issue1071"
ARCHIVE_PATH = f"ai/downward/{ISSUE}"


def make_comparison_table():
    report = common_setup.ComparativeReport(
        algorithm_pairs=[
            (f"{ISSUE}-base-seq-opt-bjolp", f"{ISSUE}-v1-seq-opt-bjolp"),
            (f"{ISSUE}-base-seq-opt-bjolp-opt", f"{ISSUE}-v1-seq-opt-bjolp-opt"),
            (f"{ISSUE}-base-lm-exhaust", f"{ISSUE}-v1-lm-exhaust"),
        ], attributes=ATTRIBUTES,
    )
    outfile = os.path.join(
        exp.eval_dir, f"{exp.name}-compare.{report.output_format}"
    )
    report(exp.eval_dir, outfile)
    exp.add_report(report)

REVISIONS = [
    f"{ISSUE}-v1",
]

CONFIGS = [
    IssueConfig("seq-opt-bjolp", [],
                driver_options=["--alias", "seq-opt-bjolp"]),
    IssueConfig("lm-exhaust",
                ["--evaluator", "lmc=lmcp(lm_exhaust())",
                 "--search", "astar(lmc,lazy_evaluator=lmc)"]),
    IssueConfig("seq-opt-bjolp-opt",
                ["--evaluator",
                 "lmc=lmcp(lm_merged([lm_rhw(),lm_hm(m=1)]), optimal=true)",
                 "--search", "astar(lmc,lazy_evaluator=lmc)"]),
]

BENCHMARKS_DIR = os.environ["DOWNWARD_BENCHMARKS"]

if common_setup.is_running_on_cluster():
    SUITE = common_setup.DEFAULT_OPTIMAL_SUITE
    ENVIRONMENT = BaselSlurmEnvironment(
        partition="infai_2",
        email="clemens.buechner@unibas.ch",
        setup="export PATH=/scicore/soft/apps/binutils/2.32-GCCcore-8.3.0/bin:/scicore/soft/apps/CMake/3.15.3-GCCcore-8.3.0/bin:/scicore/soft/apps/cURL/7.66.0-GCCcore-8.3.0/bin:/scicore/soft/apps/bzip2/1.0.8-GCCcore-8.3.0/bin:/scicore/soft/apps/ncurses/6.1-GCCcore-8.3.0/bin:/scicore/soft/apps/GCCcore/8.3.0/bin:/infai/buecle01/local:/export/soft/lua_lmod/centos7/lmod/lmod/libexec:/usr/local/bin:/usr/bin:/usr/local/sbin:/usr/sbin:$PATH\nexport LD_LIBRARY_PATH=/scicore/soft/apps/binutils/2.32-GCCcore-8.3.0/lib:/scicore/soft/apps/cURL/7.66.0-GCCcore-8.3.0/lib:/scicore/soft/apps/bzip2/1.0.8-GCCcore-8.3.0/lib:/scicore/soft/apps/zlib/1.2.11-GCCcore-8.3.0/lib:/scicore/soft/apps/ncurses/6.1-GCCcore-8.3.0/lib:/scicore/soft/apps/GCCcore/8.3.0/lib64:/scicore/soft/apps/GCCcore/8.3.0/lib",
    )
else:
    SUITE = common_setup.IssueExperiment.DEFAULT_TEST_SUITE
    ENVIRONMENT = LocalEnvironment(processes=2)

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

exp.add_step("build", exp.build)
exp.add_step("start", exp.start_runs)
exp.add_fetcher("data/issue1071-base-optimal-eval", name="fetch base")
exp.add_fetcher(name="fetch", merge=True)
exp.add_step("comparison table", make_comparison_table)
exp.add_parse_again_step()

exp.add_archive_step(ARCHIVE_PATH)

exp.run_steps()
