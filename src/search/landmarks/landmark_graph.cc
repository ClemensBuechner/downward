#include "landmark_graph.h"

#include "landmark.h"

#include "../utils/memory.h"

#include <cassert>
#include <list>
#include <set>
#include <sstream>
#include <vector>

using namespace std;

namespace landmarks {
LandmarkGraph::LandmarkGraph()
    : num_conjunctive_landmarks(0), num_disjunctive_landmarks(0) {
}

int LandmarkGraph::get_num_edges() const {
    int total = 0;
    for (auto &node : nodes)
        total += node->children.size();
    return total;
}

LandmarkNode *LandmarkGraph::get_node(int i) const {
    return nodes[i].get();
}

LandmarkNode *LandmarkGraph::get_node(const FactPair &fact) const {
    /* Return pointer to landmark node that corresponds to the given fact,
       or nullptr if no such landmark exists. */
    LandmarkNode *node_p = nullptr;
    auto it = simple_landmarks_to_nodes.find(fact);
    if (it != simple_landmarks_to_nodes.end())
        node_p = it->second;
    else {
        auto it2 = disjunctive_landmarks_to_nodes.find(fact);
        if (it2 != disjunctive_landmarks_to_nodes.end())
            node_p = it2->second;
    }
    return node_p;
}
}
