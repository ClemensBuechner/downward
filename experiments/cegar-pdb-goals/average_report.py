# -*- coding: utf-8 -*-

from downward.reports import PlanningReport
from lab import tools
from lab.reports import geometric_mean

DEBUG = False


class AverageAlgorithmReport(PlanningReport):
    """
    This currently only works for some hard-coded attributes.

    This version is based on the fact of all data being in just one property's file.
    It generates average values for every config (of all random seeds)
    """
    def __init__(self, algo_name_seeds, num_seeds, **kwargs):
        PlanningReport.__init__(self, **kwargs)
        self.algo_name_seeds = algo_name_seeds
        self.num_seeds = num_seeds

    def get_text(self):
        if not self.outfile.endswith("properties"):
            raise ValueError("outfile must be a path to a properties file")
        # remove seeds to get set of all distinct config names ignoring seeds as differences
        algo_infixes = set()
        for algo in self.algorithms:
            for seed in self.algo_name_seeds:
                if seed in algo:
                    algo_infixes.add(algo.replace(seed, ''))
                    break
        if DEBUG:
            print("Distinct algo names ignoring seeds: ")
            print(algo_infixes)
        props = tools.Properties(self.outfile)
        for domain, problem in self.problem_runs.keys():
            if DEBUG:
                print(domain, problem)
            for algo in algo_infixes:
                if DEBUG:
                    print("Consider ", algo)
                properties_key = algo + '-' + domain + '-' + problem
                average_algo_dict = {}
                average_algo_dict['algorithm'] = algo
                average_algo_dict['domain'] = domain
                average_algo_dict['problem'] = problem
                average_algo_dict['id'] = [algo, domain, problem]
                for attribute in self.attributes:
                    if DEBUG:
                        print("Consider ", attribute)
                    values = []
                    for seed in self.algo_name_seeds:
                        revision, rest = algo.split("-", 1)
                        real_algo = f"{revision}-{rest}{seed}"
                        if DEBUG:
                            print("Composed algo ", real_algo)
                        real_algo_run = self.runs[(domain, problem, real_algo)]
                        values.append(real_algo_run.get(attribute))
                    if DEBUG:
                        print(values)
                    values_without_none = [value for value in values if value is not None]
                    stddev = None
                    if attribute in [
                            'avg_time_per_generator', 'cost', 'coverage',
                            'initial_h_value', 'expansions_until_last_jump',
                            'num_abstractions',
                        ] or 'score' in attribute:
                        # if 'score' not in attribute:
                            # assert len(values_without_none) == 10 # does not hold for scores
                        average_value = sum(values_without_none)/float(len(values))
                        if len(values_without_none) > 1:
                            stddev = 0
                            for val in values_without_none:
                                stddev += (val - average_value)**2
                            stddev /= len(values_without_none) - 1
                    elif 'time' in attribute or 'expansions' in attribute:
                        if len(values_without_none) == self.num_seeds:
                            average_value = geometric_mean(values_without_none)
                        else:
                            average_value = None
                    else:
                        print(f"Don't know how to handle {attribute}")
                        exit(1)
                    average_algo_dict[attribute] = average_value
                    average_algo_dict[f"{attribute}-stddev"] = stddev
                props[properties_key] = average_algo_dict
        props.write()
        return str(props)
