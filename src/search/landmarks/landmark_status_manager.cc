#include "landmark_status_manager.h"

#include "landmark.h"

#include "../utils/logging.h"

using namespace std;

namespace landmarks {
static bool has_orders_of_type(LandmarkGraph &lm_graph, EdgeType type) {
    for (const unique_ptr<LandmarkNode> &node : lm_graph.get_nodes()) {
        for (pair<LandmarkNode *, EdgeType> parent : node->parents) {
            if (parent.second == type) {
                return true;
            }
        }
    }
    return false;
}

/*
  By default we mark all landmarks as reached, since we do an intersection when
  computing new landmark information.
*/
LandmarkStatusManager::LandmarkStatusManager(LandmarkGraph &graph)
    : reached_lms(vector<bool>(graph.get_num_landmarks(), true)),
      lm_status(graph.get_num_landmarks(), NOT_REACHED),
      lm_graph(graph),
      consider_reasonable_orders(
          has_orders_of_type(lm_graph, EdgeType::REASONABLE)) {
}

BitsetView LandmarkStatusManager::get_reached_landmarks(const State &state) {
    return reached_lms[state];
}

void LandmarkStatusManager::set_landmarks_for_initial_state(
    const State &initial_state) {
    BitsetView reached = get_reached_landmarks(initial_state);
    // This is necessary since the default is "true for all" (see comment above).
    reached.reset();

    int num_init_lms = 0;
    int num_goal_lms = 0;
    for (auto &lm_node : lm_graph.get_nodes()) {
        const Landmark &landmark = lm_node->get_landmark();
        if (landmark.is_true_in_goal) {
            ++num_goal_lms;
        }
        if (landmark.is_true_in_state(initial_state)) {
            reached.set(lm_node->get_id());
            ++num_init_lms;
        }
    }
    utils::g_log << num_init_lms << " initial landmarks, "
                 << num_goal_lms << " goal landmarks" << endl;
}

bool LandmarkStatusManager::update_reached_landmarks(
    const State &parent_ancestor_state, OperatorID /*op_id*/,
    const State &ancestor_state) {
    if (ancestor_state == parent_ancestor_state) {
        // This can happen, e.g., in Satellite-01.
        return false;
    }

    const BitsetView parent_reached =
        get_reached_landmarks(parent_ancestor_state);
    BitsetView reached = get_reached_landmarks(ancestor_state);

    int num_landmarks = lm_graph.get_num_landmarks();
    assert(reached.size() == num_landmarks);
    assert(parent_reached.size() == num_landmarks);

    /*
       Set all landmarks not reached by this parent as "not reached".
       Over multiple paths, this has the effect of computing the intersection
       of "reached" for the parents. It is important here that upon first visit,
       all elements in "reached" are true because true is the neutral element
       of intersection.

       In the case where the landmark we are setting to false here is actually
       achieved right now, it is set to "true" again below.
    */
    reached.intersect(parent_reached);

    // Mark landmarks reached right now as "reached".
    for (int id = 0; id < num_landmarks; ++id) {
        if (!reached.test(id)) {
            LandmarkNode *node = lm_graph.get_node(id);
            /*
              TODO: It may be very inefficient to check this for all landmarks
               separately (and potentially multiple times since we do it again
               in *mark_needed_again_relatives*).
            */
            if (node->get_landmark().is_true_in_state(ancestor_state)) {
                reached.set(id);
            }
        }
    }

    return true;
}

void LandmarkStatusManager::mark_needed_again_relatives(
    const LandmarkNode *node, const State &state) {
    assert(lm_status[node->get_id()] == NOT_REACHED);

    /*
      For all *parent* -gn-> *node*, if *node* is not reached and *parent*
      currently not true, since *parent* is a necessary precondition for actions
      achieving *node* for the first time, it must become true again.
    */
    for (auto &edge : node->parents) {
        const LandmarkNode *parent = edge.first;
        int parent_id = parent->get_id();
        if (edge.second >= EdgeType::GREEDY_NECESSARY
            && lm_status[parent_id] == REACHED
            && !parent->get_landmark().is_true_in_state(state)) {
            lm_status[parent_id] = NEEDED_AGAIN;
        }
    }

    // TODO: the same check could also be done for greedy-necessary orderings.
    if (consider_reasonable_orders) {
        /*
          For all *node* -r-> *child* where *child* was first added but *node*
          has never been added, by definition of reasonable orderings *child*
          must be made false to reach *node* and is hence needed again.
        */
        for (auto &edge : node->children) {
            const LandmarkNode *child = edge.first;
            int child_id = child->get_id();
            if (edge.second == EdgeType::REASONABLE
                && lm_status[child_id == REACHED]) {
                lm_status[child_id] = NEEDED_AGAIN;
            }
        }
    }
}

void LandmarkStatusManager::update_lm_status(const State &ancestor_state) {
    const BitsetView reached = get_reached_landmarks(ancestor_state);

    const int num_landmarks = lm_graph.get_num_landmarks();
    /* This first loop is necessary as setup for the *needed again*
       check in the second loop. */
    for (int id = 0; id < num_landmarks; ++id) {
        lm_status[id] = reached.test(id) ? REACHED : NOT_REACHED;
    }
    for (int id = 0; id < num_landmarks; ++id) {
        LandmarkNode *node = lm_graph.get_node(id);
        LandmarkStatus status = lm_status[id];
        if (status == NOT_REACHED) {
            mark_needed_again_relatives(node, ancestor_state);
        } else if (status == REACHED) {
            Landmark &landmark = node->get_landmark();
            if (landmark.is_true_in_goal
                && landmark.is_true_in_state(ancestor_state)) {
                lm_status[id] = NEEDED_AGAIN;
            }
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
            if ((lm_status[id] == NOT_REACHED) &&
                landmark.first_achievers.empty()) {
                return true;
            }
            if ((lm_status[id] == NEEDED_AGAIN) &&
                landmark.possible_achievers.empty()) {
                return true;
            }
        }
    }
    return false;
}
}
