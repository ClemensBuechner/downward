#! /usr/bin/env python3

import os

from lab.environments import LocalEnvironment, BaselSlurmEnvironment
from lab.reports import Attribute

import common_setup
from common_setup import IssueConfig, IssueExperiment

from landmark_parser import LandmarkParser

ISSUE = "issue1072"
ARCHIVE_PATH = f"ai/downward/{ISSUE}"
DIR = os.path.dirname(os.path.abspath(__file__))
REPO_DIR = os.environ["DOWNWARD_REPO"]
BENCHMARKS_DIR = os.environ["DOWNWARD_BENCHMARKS"]
REVISIONS = [
    f"{ISSUE}-base",
    f"{ISSUE}-v5",
]
GLOBAL_DRIVER_OPTIONS = []
BUILDS = ["release"]
CONFIG_NICKS = [
    ("lama", ["--search",
              "--if-unit-cost",
                 "let(hlm, landmark_sum(lm_reasonable_orders_hps(lm_rhw()),pref=true),"
                 "let(hff, ff(),"
                 """iterated([
                     lazy_greedy([hff,hlm],preferred=[hff,hlm]),
                     lazy_wastar([hff,hlm],preferred=[hff,hlm],w=5),
                     lazy_wastar([hff,hlm],preferred=[hff,hlm],w=3),
                     lazy_wastar([hff,hlm],preferred=[hff,hlm],w=2),
                     lazy_wastar([hff,hlm],preferred=[hff,hlm],w=1)
                  ],repeat_last=true,continue_on_fail=true)))""",
                 "--if-non-unit-cost",
                 "let(hlm1, landmark_sum(lm_reasonable_orders_hps(lm_rhw()),transform=adapt_costs(one),pref=true),"
                 "let(hff1, ff(transform=adapt_costs(one)),"
                 "let(hlm2, landmark_sum(lm_reasonable_orders_hps(lm_rhw()),transform=adapt_costs(plusone),pref=true),"
                 "let(hff2, ff(transform=adapt_costs(plusone)),"
                 """iterated([
                     lazy_greedy([hff1,hlm1],preferred=[hff1,hlm1], cost_type=one,reopen_closed=false),
                     lazy_greedy([hff2,hlm2],preferred=[hff2,hlm2], reopen_closed=false),
                     lazy_wastar([hff2,hlm2],preferred=[hff2,hlm2],w=5),
                     lazy_wastar([hff2,hlm2],preferred=[hff2,hlm2],w=3),
                     lazy_wastar([hff2,hlm2],preferred=[hff2,hlm2],w=2),
                     lazy_wastar([hff2,hlm2],preferred=[hff2,hlm2],w=1)
                 ],repeat_last=true,continue_on_fail=true)))))""",
                 # Append --always to be on the safe side if we want to append
                 # additional options later.
                 "--always"],
    []),
    ("conj", ["--search",
              "--if-unit-cost",
                 "let(hlm, landmark_sum(lm_reasonable_orders_hps(lm_hm(m=2)),pref=true),"
                 """iterated([
                     lazy_greedy([hlm],preferred=[hlm]),
                     lazy_wastar([hlm],preferred=[hlm],w=5),
                     lazy_wastar([hlm],preferred=[hlm],w=3),
                     lazy_wastar([hlm],preferred=[hlm],w=2),
                     lazy_wastar([hlm],preferred=[hlm],w=1)
                  ],repeat_last=true,continue_on_fail=true))""",
                 "--if-non-unit-cost",
                 "let(hlm1, landmark_sum(lm_reasonable_orders_hps(lm_rhw()),transform=adapt_costs(one),pref=true),"
                 "let(hlm2, landmark_sum(lm_reasonable_orders_hps(lm_rhw()),transform=adapt_costs(plusone),pref=true),"
                 """iterated([
                     lazy_greedy([hlm1],preferred=[hlm1], cost_type=one,reopen_closed=false),
                     lazy_greedy([hlm2],preferred=[hlm2], reopen_closed=false),
                     lazy_wastar([hlm2],preferred=[hlm2],w=5),
                     lazy_wastar([hlm2],preferred=[hlm2],w=3),
                     lazy_wastar([hlm2],preferred=[hlm2],w=2),
                     lazy_wastar([hlm2],preferred=[hlm2],w=1)
                 ],repeat_last=true,continue_on_fail=true)))""",
                 # Append --always to be on the safe side if we want to append
                 # additional options later.
                 "--always"],
    []),
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

SUITE = common_setup.DEFAULT_SATISFICING_SUITE
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

if common_setup.is_test_run():
    SUITE = IssueExperiment.DEFAULT_TEST_SUITE
    ENVIRONMENT = LocalEnvironment(processes=4)

exp = IssueExperiment(
    revisions=REVISIONS,
    configs=CONFIGS,
    environment=ENVIRONMENT,
)
exp.add_suite(BENCHMARKS_DIR, SUITE)

exp.add_parser(exp.EXITCODE_PARSER)
exp.add_parser(exp.ANYTIME_SEARCH_PARSER)
exp.add_parser(exp.PLANNER_PARSER)
exp.add_parser(LandmarkParser())

ATTRIBUTES = IssueExperiment.DEFAULT_TABLE_ATTRIBUTES + [
    Attribute("landmarks", min_wins=False),
    Attribute("landmarks_disjunctive", min_wins=False),
    Attribute("landmarks_conjunctive", min_wins=False),
    Attribute("orderings", min_wins=False),
    Attribute("lmgraph_generation_time"),
]

exp.add_step("build", exp.build)
exp.add_step("start", exp.start_runs)
exp.add_step("parse", exp.parse)
exp.add_fetcher(name="fetch")

exp.add_comparison_table_step(
    attributes=IssueExperiment.DEFAULT_TABLE_ATTRIBUTES + [
        Attribute("landmarks", min_wins=False),
        Attribute("landmarks_disjunctive", min_wins=False),
        Attribute("landmarks_conjunctive", min_wins=False),
        Attribute("orderings", min_wins=False),
        Attribute("lmgraph_generation_time", min_wins=True),
    ]
)
exp.add_scatter_plot_step(relative=True, attributes=["total_time", "memory"])

exp.add_archive_step(ARCHIVE_PATH)
# exp.add_archive_eval_dir_step(ARCHIVE_PATH)

exp.run_steps()
