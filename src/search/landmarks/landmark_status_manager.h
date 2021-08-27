#ifndef LANDMARKS_LANDMARK_STATUS_MANAGER_H
#define LANDMARKS_LANDMARK_STATUS_MANAGER_H

#include "landmark_graph.h"

#include "../per_state_bitset.h"

namespace landmarks {
class LandmarkGraph;
class LandmarkNode;

enum landmark_status {
    lm_accepted = 0, lm_not_accepted = 1, lm_needed_again = 2
};

class LandmarkStatusManager {
    PerStateBitset accepted_lms;
    std::vector<landmark_status> lm_status;
    bool use_reasonable_orders;

    LandmarkGraph &lm_graph;

    void collect_needed_again_relatives(
        const LandmarkNode *node, const State &state);

    bool landmark_is_leaf(const LandmarkNode &node,
                          const BitsetView &accepted) const;
    bool landmark_needed_again(int id, const State &state);
public:
    explicit LandmarkStatusManager(
        LandmarkGraph &graph, bool use_reasonable_orders);

    BitsetView get_accepted_landmarks(const State &state);

    void update_lm_status(const State &ancestor_state);
    bool dead_end_exists();

    void set_landmarks_for_initial_state(const State &initial_state);
    bool update_accepted_landmarks(const State &parent_ancestor_state,
                                   OperatorID op_id,
                                   const State &ancestor_state);

    /*
      TODO:
      The status of a landmark is actually dependent on the state. This
      is not represented in the functions below. Furthermore, the status
      manager only stores the status for one particular state at a time.

      At the day of writing this comment, this works as
      *update_accepted_landmarks()* is always called before the status
      information is used (by calling *get_landmark_status()*).

      It would be a good idea to ensure that the status for the
      desired state is returned at all times, or an error is thrown
      if the desired information does not exist.
     */
    bool landmark_accepted(int id) const {
        return lm_status[id] != lm_not_accepted;
    }
    bool landmark_required(int id) const {
        return lm_status[id] != lm_accepted;
    }
};
}

#endif
