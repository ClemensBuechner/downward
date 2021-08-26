#include "landmark_status_manager.h"

#include "landmark.h"

#include "../utils/logging.h"

using namespace std;

namespace landmarks {
/*
  By default we mark all landmarks as accepted, since we do an intersection when
  computing new landmark information.
*/
LandmarkStatusManager::LandmarkStatusManager(LandmarkGraph &graph)
    : accepted_lms(vector<bool>(graph.get_num_landmarks(), true)),
      lm_status(graph.get_num_landmarks(), lm_not_accepted),
      lm_graph(graph) {
}

BitsetView LandmarkStatusManager::get_accepted_landmarks(const State &state) {
    return accepted_lms[state];
}

void LandmarkStatusManager::set_landmarks_for_initial_state(
    const State &initial_state) {
    BitsetView accepted = get_accepted_landmarks(initial_state);
    // This is necessary since the default is "true for all" (see comment above).
    accepted.reset();

    int inserted = 0;
    int num_goal_lms = 0;
    for (auto &lm_node : lm_graph.get_nodes()) {
        const Landmark &landmark = lm_node->get_landmark();
        if (landmark.is_true_in_goal) {
            ++num_goal_lms;
        }

        if (!lm_node->parents.empty()) {
            continue;
        }
        if (landmark.conjunctive) {
            bool lm_true = true;
            for (const FactPair &fact : landmark.facts) {
                if (initial_state[fact.var].get_value() != fact.value) {
                    lm_true = false;
                    break;
                }
            }
            if (lm_true) {
                accepted.set(lm_node->get_id());
                ++inserted;
            }
        } else {
            for (const FactPair &fact : landmark.facts) {
                if (initial_state[fact.var].get_value() == fact.value) {
                    accepted.set(lm_node->get_id());
                    ++inserted;
                    break;
                }
            }
        }
    }
    utils::g_log << inserted << " initial landmarks, "
                 << num_goal_lms << " goal landmarks" << endl;
}

bool LandmarkStatusManager::update_accepted_landmarks(
    const State &parent_ancestor_state, OperatorID /*op_id*/,
    const State &ancestor_state) {
    if (ancestor_state == parent_ancestor_state) {
        // This can happen, e.g., in Satellite-01.
        return false;
    }

    const BitsetView parent_accepted = get_accepted_landmarks(
        parent_ancestor_state);
    BitsetView accepted = get_accepted_landmarks(ancestor_state);

    int num_landmarks = lm_graph.get_num_landmarks();
    assert(accepted.size() == num_landmarks);
    assert(parent_accepted.size() == num_landmarks);

    /*
       Set all landmarks not accepted by this parent as "not accepted".
       Over multiple paths, this has the effect of computing the intersection
       of "accepted" for the parents. It is important here that upon first visit,
       all elements in "accepted" are true because true is the neutral element
       of intersection.

       In the case where the landmark we are setting to false here is actually
       achieved right now, it is set to "true" again below.
    */
    accepted.intersect(parent_accepted);

    // Mark landmarks accepted right now as "accepted" (if they are "leaves").
    for (int id = 0; id < num_landmarks; ++id) {
        if (!accepted.test(id)) {
            LandmarkNode *node = lm_graph.get_node(id);
            if (node->get_landmark().is_true_in_state(ancestor_state)) {
                if (landmark_is_leaf(*node, accepted)) {
                    accepted.set(id);
                }
            }
        }
    }

    return true;
}

void LandmarkStatusManager::update_lm_status(const State &ancestor_state) {
    const BitsetView accepted = get_accepted_landmarks(ancestor_state);

    const int num_landmarks = lm_graph.get_num_landmarks();
    /* This first loop is necessary as setup for the *needed again*
       check in the second loop. */
    for (int id = 0; id < num_landmarks; ++id) {
        lm_status[id] = accepted.test(id) ? lm_accepted : lm_not_accepted;
    }
    for (int id = 0; id < num_landmarks; ++id) {
        if (lm_status[id] == lm_accepted
            && landmark_needed_again(id, ancestor_state)) {
            lm_status[id] = lm_needed_again;
        }
    }
}

bool LandmarkStatusManager::dead_end_exists() {
    for (auto &node : lm_graph.get_nodes()) {
        int id = node->get_id();

        /*
          This dead-end detection works for the following case:
          X is a goal, it is true in the initial state, and has no achievers.
          Some action A has X as a delete effect. Then using this,
          we can detect that applying A leads to a dead-end.

          Note: this only tests for reachability of the landmark from the initial state.
          A (possibly) more effective option would be to test reachability of the landmark
          from the current state.
        */

        const Landmark &landmark = node->get_landmark();
        if (!landmark.is_derived) {
            if ((lm_status[id] == lm_not_accepted) &&
                landmark.first_achievers.empty()) {
                return true;
            }
            if ((lm_status[id] == lm_needed_again) &&
                landmark.possible_achievers.empty()) {
                return true;
            }
        }
    }
    return false;
}

bool LandmarkStatusManager::landmark_needed_again(
    int id, const State &state) {
    LandmarkNode *node = lm_graph.get_node(id);
    const Landmark &landmark = node->get_landmark();
    if (landmark.is_true_in_state(state)) {
        return false;
    } else if (landmark.is_true_in_goal) {
        return true;
    } else {
        /*
          For all A ->_gn B, if B is not accepted and A currently not
          true, since A is a necessary precondition for actions
          achieving B for the first time, it must become true again.
        */
        for (const auto &child : node->children) {
            if (child.second >= EdgeType::GREEDY_NECESSARY
                && lm_status[child.first->get_id()] == lm_not_accepted) {
                return true;
            }
        }
        return false;
    }
}

bool LandmarkStatusManager::landmark_is_leaf(const LandmarkNode &node,
                                             const BitsetView &accepted) const {
    //Note: this is the same as !check_node_orders_disobeyed
    for (const auto &parent : node.parents) {
        LandmarkNode *parent_node = parent.first;
        // Note: no condition on edge type here
        if (!accepted.test(parent_node->get_id())) {
            return false;
        }
    }
    return true;
}
}
