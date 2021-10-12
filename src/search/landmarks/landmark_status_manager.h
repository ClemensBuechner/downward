#ifndef LANDMARKS_LANDMARK_STATUS_MANAGER_H
#define LANDMARKS_LANDMARK_STATUS_MANAGER_H

#include "landmark_graph.h"

#include "../per_state_bitset.h"

namespace landmarks {
class LandmarkGraph;
class LandmarkNode;

enum landmark_status {REACHED = 0, NOT_REACHED = 1, NEEDED_AGAIN = 2};

class LandmarkStatusManager {
    PerStateBitset reached_lms;
    std::vector<landmark_status> lm_status;

    LandmarkGraph &lm_graph;
    const bool consider_reasonable_orders;

    void mark_needed_again_relatives(
        const LandmarkNode *node, const State &state);
public:
    explicit LandmarkStatusManager(LandmarkGraph &graph);

    BitsetView get_reached_landmarks(const State &state);

    void update_lm_status(const State &ancestor_state);
    bool dead_end_exists();

    void set_landmarks_for_initial_state(const State &initial_state);
    bool update_reached_landmarks(const State &parent_ancestor_state,
                                  OperatorID op_id,
                                  const State &ancestor_state);

    /*
      TODO:
      The status of a landmark is actually dependent on the state. This
      is not represented in the function below. Furthermore, the status
      manager only stores the status for one particular state at a time.

      At the day of writing this comment, this works as
      *update_reached_landmarks()* is always called before the status
      information is used (by calling *get_landmark_status()*).

      It would be a good idea to ensure that the status for the
      desired state is returned at all times, or an error is thrown
      if the desired information does not exist.
     */
    landmark_status get_landmark_status(size_t id) const {
        assert(utils::in_bounds(id, lm_status));
        return lm_status[id];
    }
};
}

#endif
