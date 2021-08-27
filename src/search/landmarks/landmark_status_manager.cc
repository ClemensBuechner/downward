#include "landmark_status_manager.h"

#include "landmark.h"

#include "../utils/logging.h"

using namespace std;

namespace landmarks {
/*
  By default we mark all landmarks as accepted, since we do an intersection when
  computing new landmark information.
*/
LandmarkStatusManager::LandmarkStatusManager(
    LandmarkGraph &graph, bool use_reasonable_orders)
    : accepted_lms(vector<bool>(graph.get_num_landmarks(), true)),
      lm_status(graph.get_num_landmarks(), lm_not_accepted),
      use_reasonable_orders(use_reasonable_orders),
      lm_graph(graph) {
}

BitsetView LandmarkStatusManager::get_accepted_landmarks(const State &state) {
    return accepted_lms[state];
}

void LandmarkStatusManager::set_landmarks_for_initial_state(
    const State &initial_state) {
    BitsetView accepted = get_accepted_landmarks(initial_state);

    int num_init_lms = lm_graph.get_num_landmarks();
    int num_goal_lms = 0;
    for (auto &lm_node: lm_graph.get_nodes()) {
        const Landmark &landmark = lm_node->get_landmark();
        if (landmark.is_true_in_goal) {
            ++num_goal_lms;
        }

        if (!landmark.is_true_in_state(initial_state)
            || !lm_node->parents.empty()) {
            accepted.reset(lm_node->get_id());
            --num_init_lms;
        }
    }
    utils::g_log << num_init_lms << " initial landmarks, "
                 << num_goal_lms << " goal landmarks" << endl;
}

bool LandmarkStatusManager::update_accepted_landmarks(
    const State &parent_ancestor_state, OperatorID,
    const State &ancestor_state) {
    if (ancestor_state == parent_ancestor_state) {
        // This can happen, e.g., in Satellite-01.
        return false;
    }

    const BitsetView parent_accepted =
        get_accepted_landmarks(parent_ancestor_state);
    BitsetView accepted = get_accepted_landmarks(ancestor_state);

    int num_landmarks = lm_graph.get_num_landmarks();
    assert(accepted.size() == num_landmarks);
    assert(parent_accepted.size() == num_landmarks);

    /*
      Consider all landmarks that are not accepted by this parent. If they are
      accepted in the current state (either because they hold in this state or
      they were accepted on all previously considered paths), they are not
      accepted anymore (unless they hold in this state).

      Over multiple paths, this has the effect of computing the intersection of
      "accepted" for all parents. It is important here that upon first visit,
      all elements in "accepted" are true because true is the neutral element of
      intersection.
    */
    for (int id = 0; id < num_landmarks; ++id) {
        if (!parent_accepted.test(id) && accepted.test(id)) {
            /* TODO: It may be very inefficient to check this for all landmarks
                separately (and potentially multiple times?). */
            if (!lm_graph.get_node(id)->get_landmark().is_true_in_state(
                ancestor_state)) {
                accepted.reset(id);
            }
        }
    }
    return true;
}

void LandmarkStatusManager::collect_needed_again_relatives(
    const LandmarkNode *node, const State &state) {
    assert(lm_status[node->get_id()] == lm_not_accepted);

    /*
      For all A -gn-> B, if B is not accepted and A currently not true,
      since A is a necessary precondition for actions achieving B for
      the first time, it must become true again.
    */
    for (auto &edge: node->parents) {
        const LandmarkNode *parent = edge.first;
        int parent_id = parent->get_id();
        if (edge.second >= EdgeType::GREEDY_NECESSARY
            && lm_status[parent_id] == lm_accepted
            && !parent->get_landmark().is_true_in_state(state)) {
            lm_status[parent_id] = lm_needed_again;
        }
    }

    if (use_reasonable_orders) {
        /*
          For all A -r-> B where A is not first added but B is, B must
          be destroyed to achieve A (definition of reasonable
          orderings). Hence, B is needed again.
        */
        for (auto &edge: node->children) {
            const LandmarkNode *child = edge.first;
            int child_id = child->get_id();
            if (edge.second == EdgeType::REASONABLE
                && lm_status[child_id == lm_accepted]) {
                lm_status[child_id] = lm_needed_again;
            }
        }
    }
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
        LandmarkNode *node = lm_graph.get_node(id);
        if (lm_status[id] == lm_not_accepted) {
            collect_needed_again_relatives(node, ancestor_state);
        } else if (lm_status[id] == lm_accepted) {
            Landmark &landmark = node->get_landmark();
            if (landmark.is_true_in_goal
                && !landmark.is_true_in_state(ancestor_state)) {
                lm_status[id] = lm_needed_again;
            }
        }
    }
}

bool LandmarkStatusManager::dead_end_exists() {
    for (auto &node: lm_graph.get_nodes()) {
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
        for (const auto &child: node->children) {
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
    for (const auto &parent: node.parents) {
        LandmarkNode *parent_node = parent.first;
        // Note: no condition on edge type here
        if (!accepted.test(parent_node->get_id())) {
            return false;
        }
    }
    return true;
}
}
