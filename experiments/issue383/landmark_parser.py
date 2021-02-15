#! /usr/bin/env python

import re

from lab.parser import Parser

parser = Parser()
parser.add_pattern(
    "landmarks", r"Discovered (\d+) landmarks, of which \d+ are simple, \d+ are disjunctive, and \d+ are conjunctive.")
parser.add_pattern(
    "simple_landmarks",
    r"Discovered \d+ landmarks, of which (\d+) are simple, \d+ are disjunctive, and \d+ are conjunctive")
parser.add_pattern(
    "disjunctive_landmarks",
    r"Discovered \d+ landmarks, of which \d+ are simple, (\d+) are disjunctive, and \d+ are conjunctive.")
parser.add_pattern(
    "conjunctive_landmarks",
    r"Discovered \d+ landmarks, of which \d+ are simple, \d+ are disjunctive, and (\d+) are conjunctive.")

parser.add_pattern("orderings", r"Discovered (\d+) landmark orderings.")
parser.add_pattern("natural_orders", r"Found (\d+) natural landmark orderings.")
parser.add_pattern("greedy_necessary_orders", r"Found (\d+) greedy-necessary landmark orderings.")
parser.add_pattern("necessary_orders", r"Found (\d+) necessary landmark orderings.")
parser.add_pattern("reasonable_orders", r"Found (\d+) reasonable landmark orderings.")
parser.add_pattern("obedient_reasonable_orders", r"Found (\d+) obedient-reasonable landmark orderings.")

parser.parse()
