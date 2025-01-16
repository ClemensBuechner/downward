#ifndef LANDMARKS_LANDMARK_GRAPH_H
#define LANDMARKS_LANDMARK_GRAPH_H

#include "landmark.h"

#include "../task_proxy.h"

#include "../utils/hash.h"
#include "../utils/memory.h"

#include <cassert>
#include <list>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace landmarks {
enum class EdgeType {
    /*
      NOTE: The code relies on the fact that larger numbers are stronger in the
      sense that, e.g., every greedy-necessary ordering is also natural and
      reasonable. (It is a sad fact of terminology that necessary is indeed a
      special case of greedy-necessary, i.e., every necessary ordering is
      greedy-necessary, but not vice versa.)
    */
    NECESSARY = 3,
    GREEDY_NECESSARY = 2,
    NATURAL = 1,
    REASONABLE = 0
};

class LandmarkNode {
    int id;
    Landmark landmark;
public:
    LandmarkNode(Landmark &&landmark)
        : id(-1), landmark(std::move(landmark)) {
    }

    std::unordered_map<LandmarkNode *, EdgeType> parents;
    std::unordered_map<LandmarkNode *, EdgeType> children;

    int get_id() const {
        return id;
    }

    // TODO: Should possibly not be changeable
    void set_id(int new_id) {
        assert(id == -1 || new_id == id);
        id = new_id;
    }

    // TODO: Remove this function once the LM-graph is constant after creation.
    Landmark &get_landmark() {
        return landmark;
    }

    const Landmark &get_landmark() const {
        return landmark;
    }
};

class LandmarkGraph {
public:
    /*
      TODO: get rid of this by removing get_nodes() and instead offering
      functions begin() and end() with an iterator class, so users of the
      LandmarkGraph can do loops like this:
        for (const LandmarkNode &n : graph) {...}
     */
    using Nodes = std::vector<std::unique_ptr<LandmarkNode>>;
private:
    int num_conjunctive_landmarks;
    int num_disjunctive_landmarks;

    utils::HashMap<FactPair, LandmarkNode *> simple_landmarks_to_nodes;
    utils::HashMap<FactPair, LandmarkNode *> disjunctive_landmarks_to_nodes;
    Nodes nodes;

    void remove_node_occurrences(LandmarkNode *node);

public:
    /* This is needed only by landmark graph factories and will disappear
       when moving landmark graph creation there. */
    LandmarkGraph(
        Nodes &&nodes);

    // needed by both landmarkgraph-factories and non-landmarkgraph-factories
    const Nodes &get_nodes() const {
        return nodes;
    }
    // needed by both landmarkgraph-factories and non-landmarkgraph-factories
    int get_num_landmarks() const {
        return nodes.size();
    }
    // needed by both landmarkgraph-factories and non-landmarkgraph-factories
    int get_num_edges() const;

    // only needed by non-landmarkgraph-factories
    LandmarkNode *get_node(int index) const;
    // only needed by non-landmarkgraph-factories
    LandmarkNode *get_node(const FactPair &fact) const;
};
}

#endif
