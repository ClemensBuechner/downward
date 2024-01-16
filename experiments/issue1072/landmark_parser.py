import re

from lab.parser import Parser

PATTERNS = [
    ["lmgraph_generation_time", r"Landmark graph generation time: (.+)s", float],
    ["landmarks", r"Landmark graph contains (\d+) landmarks, of which \d+ are disjunctive, \d+ are conjunctive, and \d+ are derived.", int],
    ["landmarks_disjunctive", r"Landmark graph contains \d+ landmarks, of which (\d+) are disjunctive, \d+ are conjunctive, and \d+ are derived.", int],
    ["landmarks_conjunctive", r"Landmark graph contains \d+ landmarks, of which \d+ are disjunctive, (\d+) are conjunctive, and \d+ are derived.", int],
    ["orderings", r"Landmark graph contains (\d+) orderings.", int],
]

class LandmarkParser(Parser):
    def __init__(self):
        Parser.__init__(self)
        for name, pattern, typ in PATTERNS:
            self.add_pattern(name, pattern, type=typ)
