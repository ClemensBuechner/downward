#include "landmark_status_manager.h"

#include "landmark.h"

using namespace std;

namespace landmarks {
static vector<LandmarkNode *> get_goal_ids(const LandmarkGraph &graph) {
    vector<LandmarkNode *> goal_lm_ids;
    for (const unique_ptr<LandmarkNode> &node : graph.get_nodes()) {
        if (node->get_landmark().is_true_in_goal) {
            goal_lm_ids.push_back(node.get());
        }
    }
    return goal_lm_ids;
}

static vector<pair<LandmarkNode *, LandmarkNode *>> get_orderings_of_type(
    const LandmarkGraph &graph, const EdgeType &type) {
    vector<pair<LandmarkNode *, LandmarkNode *>> orderings;
    for (auto &node : graph.get_nodes()) {
        for (auto &parent : node->parents) {
            // TODO: Could be strengthened by *parent.second >= type*.
            if (parent.second == type) {
                orderings.emplace_back(parent.first, node.get());
            }
        }
    }
    return orderings;
}

/*
  By default we mark all landmarks past, since we do an intersection when
  computing new landmark information.
*/
LandmarkStatusManager::LandmarkStatusManager(
    LandmarkGraph &graph,
    bool progress_goals,
    bool progress_greedy_necessary_orderings,
    bool progress_reasonable_orderings)
    : lm_graph(graph),
      goal_landmark_ids(progress_goals ? get_goal_ids(graph) : vector<LandmarkNode *>{}),
      greedy_necessary_orderings(
          progress_greedy_necessary_orderings
          ? get_orderings_of_type(graph, EdgeType::GREEDY_NECESSARY)
          : vector<pair<LandmarkNode *, LandmarkNode *>>{}),
      reasonable_orderings(
          progress_reasonable_orderings
          ? get_orderings_of_type(graph, EdgeType::REASONABLE)
          : vector<pair<LandmarkNode *, LandmarkNode *>>{}),
      past_landmarks(vector<bool>(graph.get_num_landmarks(), true)),
      future_landmarks(vector<bool>(graph.get_num_landmarks(), false)) {
}

BitsetView LandmarkStatusManager::get_past_landmarks(const State &state) {
    return past_landmarks[state];
}

BitsetView LandmarkStatusManager::get_future_landmarks(const State &state) {
    return future_landmarks[state];
}

void LandmarkStatusManager::progress_initial_state(const State &initial_state) {
    BitsetView past = get_past_landmarks(initial_state);
    BitsetView fut = get_future_landmarks(initial_state);

    for (auto &node : lm_graph.get_nodes()) {
        int id = node->get_id();
        const Landmark &lm = node->get_landmark();
        if (lm.is_true_in_state(initial_state)) {
            assert(past.test(id));
            for (auto &parent : node->parents) {
                /*
                  We assume here that no natural orderings A->B are generated
                  such that B holds in the initial state. Such orderings would
                  only be valid in unsolvable problems.
                */
                assert(parent.second <= EdgeType::REASONABLE);
                /*
                  Reasonable orderings A->B for which both A and B hold in the
                  initial state are immediately satisfied. We assume such
                  orderings are not generated in the first place.
                */
                assert(!parent.first->get_landmark().is_true_in_state(
                           initial_state));
                utils::unused_variable(parent);
                fut.set(id);
            }
        } else {
            past.reset(id);
            fut.set(id);
        }
    }
}

void LandmarkStatusManager::progress(
    const State &parent_ancestor_state, OperatorID,
    const State &ancestor_state) {
    if (ancestor_state == parent_ancestor_state) {
        // This can happen, e.g., in Satellite-01.
        return;
    }

    const BitsetView parent_past = get_past_landmarks(parent_ancestor_state);
    BitsetView past = get_past_landmarks(ancestor_state);

    const BitsetView parent_fut = get_future_landmarks(parent_ancestor_state);
    BitsetView fut = get_future_landmarks(ancestor_state);

    assert(past.size() == lm_graph.get_num_landmarks());
    assert(parent_past.size() == lm_graph.get_num_landmarks());
    assert(fut.size() == lm_graph.get_num_landmarks());
    assert(parent_fut.size() == lm_graph.get_num_landmarks());

    progress_basic(parent_past, parent_fut, past, fut, ancestor_state);
    progress_goals(ancestor_state, fut);
    progress_greedy_necessary_orderings(ancestor_state, past, fut);
    progress_reasonable_orderings(past, fut);
}

void LandmarkStatusManager::progress_basic(
    const BitsetView &parent_past, const BitsetView &parent_fut,
    BitsetView &past, BitsetView &fut, const State &ancestor_state) {
    for (auto &node : lm_graph.get_nodes()) {
        int id = node->get_id();
        const Landmark &lm = node->get_landmark();
        if (parent_fut.test(id)) {
            /*
              A future landmark remains future if it is not achieved by the
              current transition. If it additionally wasn't past in the parent,
              it remains not past.
            */
            if (!lm.is_true_in_state(ancestor_state)) {
                fut.set(id);
                if (!parent_past.test(id)) {
                    past.reset(id);
                }
            }
        }
    }
}

void LandmarkStatusManager::progress_goals(const State &ancestor_state,
                                           BitsetView &fut) {
    for (const LandmarkNode *lm_node : goal_landmark_ids) {
        const Landmark &lm = lm_node->get_landmark();
        assert(lm.is_true_in_goal);
        if (!lm.is_true_in_state(ancestor_state)) {
            fut.set(lm_node->get_id());
        }
    }
}

void LandmarkStatusManager::progress_greedy_necessary_orderings(
    const State &ancestor_state, const BitsetView &past, BitsetView &fut) {
    // TODO: We could avoid some .test() calls by doing one "parent" at a time.
    for (auto &[tail, head] : greedy_necessary_orderings) {
        const Landmark &lm = tail->get_landmark();
        if (!past.test(head->get_id())
            && !lm.is_true_in_state(ancestor_state)) {
            fut.set(tail->get_id());
        }
    }
}

void LandmarkStatusManager::progress_reasonable_orderings(
    const BitsetView &past, BitsetView &fut) {
    // TODO: We could avoid some .test() calls by doing one "child" at a time.
    for (auto &[tail, head] : reasonable_orderings) {
        if (!past.test(tail->get_id())) {
            fut.set(head->get_id());
        }
    }
}

bool LandmarkStatusManager::landmark_is_past(
    int id, const State &ancestor_state) const {
    return past_landmarks[ancestor_state].test(id);
}

bool LandmarkStatusManager::landmark_is_future(
    int id, const State &ancestor_state) const {
    return future_landmarks[ancestor_state].test(id);
}
}
