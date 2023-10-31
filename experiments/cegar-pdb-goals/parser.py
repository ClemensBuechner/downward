#! /usr/bin/env python

import re

from lab.parser import Parser

parser = Parser()
parser.add_pattern(
    "abstraction_size", r"Final abstraction size: (\d+)", type=int)
parser.add_pattern(
    "abstracted_variables_total", r"Total variables: (\d+)", type=int)
parser.add_pattern(
    "abstracted_variables_trivial", r"Trivial variables: (\d+)", type=int)
parser.add_pattern(
    "abstracted_variables_complete", r"Complete variables: (\d+)", type=int)
parser.add_pattern(
    "average_domain_ratio", r"Average domain size: (.+)", type=float)
parser.add_pattern(
    "cegar_iterations", r"Number of CEGAR iterations: (\d+)", type=int)


def stagnation_reached(content, props):
    if "stagnation limit reached" in content:
        props["stagnation_reached"] = 1
    else:
        props["stagnation_reached"] = 0


def stagnation_terminated(content, props):
    if "stagnation limit reached*terminating" in content:
        props["stagnation_terminated"] = 1
    else:
        props["stagnation_terminated"] = 0


parser.add_function(stagnation_reached)
parser.add_function(stagnation_terminated)

# multiple
parser.add_pattern(
    "num_iterations_multiple",
    r"multiple CEGAR domain abstraction collection generator number of iterations: (\d+)",
    type=int)
parser.add_pattern(
    "avg_time_per_generator",
    r"multiple CEGAR domain abstraction collection generator average time per generator: (.+)",
    type=float)
parser.add_pattern(
    "num_abstractions",
    r"multiple CEGAR domain abstraction collection generator number of abstractions: (\d+)",
    type=int)
parser.add_pattern(
    "multiple_computation_time",
    r"multiple CEGAR domain abstraction collection generator computation time: (.+)s",
    type=float)

parser.add_pattern("num_pdbs", "Number of patterns: (\d+)", type=int)

parser.parse()
