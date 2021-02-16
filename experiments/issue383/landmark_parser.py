#! /usr/bin/env python

import re

from lab.parser import Parser

parser = Parser()
parser.add_pattern(
    "landmarks", r"Discovered (\d+) landmarks, of which \d+ are simple, \d+ are disjunctive, and \d+ are conjunctive.")
parser.add_pattern(
    "landmarks_simple",
    r"Discovered \d+ landmarks, of which (\d+) are simple, \d+ are disjunctive, and \d+ are conjunctive")
parser.add_pattern(
    "landmarks_disjunctive",
    r"Discovered \d+ landmarks, of which \d+ are simple, (\d+) are disjunctive, and \d+ are conjunctive.")
parser.add_pattern(
    "landmarks_conjunctive",
    r"Discovered \d+ landmarks, of which \d+ are simple, \d+ are disjunctive, and (\d+) are conjunctive.")

parser.add_pattern("orderings", r"Discovered (\d+) landmark orderings.")
parser.add_pattern("orderings_natural", 
                   r"Found (\d+) natural landmark orderings.")
parser.add_pattern("orderings_greedy_necessary", 
                   r"Found (\d+) greedy-necessary landmark orderings.")
parser.add_pattern("orderings_necessary", 
                   r"Found (\d+) necessary landmark orderings.")
parser.add_pattern("orderings_reasonable", 
                   r"Found (\d+) reasonable landmark orderings.")
parser.add_pattern("orderings_obedient_reasonable", 
                   r"Found (\d+) obedient-reasonable landmark orderings.")

parser.parse()
