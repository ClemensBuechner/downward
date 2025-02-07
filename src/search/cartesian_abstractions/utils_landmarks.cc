#include "utils_landmarks.h"

#include "../plugins/plugin.h"
#include "../landmarks/landmark.h"
#include "../landmarks/landmark_factory_h_m.h"
#include "../landmarks/landmark_graph.h"
#include "../utils/logging.h"
#include "../utils/memory.h"

#include <algorithm>

using namespace std;
using namespace landmarks;

namespace cartesian_abstractions {
static FactPair get_fact(const Landmark &landmark) {
    // We assume that the given Landmarks are from an h^m landmark graph with m=1.
    assert(landmark.facts.size() == 1);
    return landmark.facts[0];
}

shared_ptr<LandmarkGraph> get_landmark_graph(
    const shared_ptr<AbstractTask> &task) {
    LandmarkFactoryHM lm_graph_factory(
        1, false, true, utils::Verbosity::SILENT);

    return lm_graph_factory.compute_lm_graph(task);
}

vector<FactPair> get_fact_landmarks(const LandmarkGraph &graph) {
    vector<FactPair> facts;
    facts.reserve(graph.get_num_landmarks());
    for (const auto &node : graph) {
        facts.push_back(get_fact(node->get_landmark()));
    }
    sort(facts.begin(), facts.end());
    return facts;
}

utils::HashMap<FactPair, LandmarkNode *> get_fact_to_landmark_map(
    const shared_ptr<LandmarkGraph> &graph) {
    // All landmarks are simple, i.e., each has exactly one fact.
    assert(all_of(graph->begin(), graph->end(), [](auto &node) {
                      return node->get_landmark().facts.size() == 1;
                  }));
    utils::HashMap<FactPair, landmarks::LandmarkNode *> fact_to_landmark_map;
    for (const auto &node : *graph) {
        const FactPair &fact = node->get_landmark().facts[0];
        fact_to_landmark_map[fact] = node.get();
    }
    return fact_to_landmark_map;
}

VarToValues get_prev_landmarks(const LandmarkNode *node) {
    VarToValues groups;
    vector<const LandmarkNode *> open;
    unordered_set<const LandmarkNode *> closed;
    for (const auto &[parent, type] : node->parents) {
        open.push_back(parent);
    }
    while (!open.empty()) {
        const LandmarkNode *ancestor = open.back();
        open.pop_back();
        if (closed.find(ancestor) != closed.end())
            continue;
        closed.insert(ancestor);
        FactPair ancestor_fact = get_fact(ancestor->get_landmark());
        groups[ancestor_fact.var].push_back(ancestor_fact.value);
        for (const auto &[parent, type] : ancestor->parents) {
            open.push_back(parent);
        }
    }
    return groups;
}
}
