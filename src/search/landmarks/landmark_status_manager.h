#ifndef LANDMARKS_LANDMARK_STATUS_MANAGER_H
#define LANDMARKS_LANDMARK_STATUS_MANAGER_H

#include "landmark_graph.h"

#include "../per_state_bitset.h"

namespace landmarks {
class LandmarkGraph;
class LandmarkNode;

enum landmark_status {lm_reached = 0, lm_not_reached = 1, lm_needed_again = 2};

class LandmarkStatusManager {
    PerStateBitset reached_lms;
    std::vector<landmark_status> lm_status;

    LandmarkGraph &lm_graph;

    bool landmark_is_leaf(const LandmarkNode &node, const BitsetView &reached) const;
    bool landmark_needed_again(int id, const GlobalState &state);
public:
    explicit LandmarkStatusManager(LandmarkGraph &graph);

    BitsetView get_reached_landmarks(const GlobalState &state);

    bool update_lm_status(const GlobalState &global_state);

    void set_landmarks_for_initial_state(const GlobalState &initial_state);
    bool update_reached_lms(const GlobalState &parent_global_state,
                            OperatorID op_id,
                            const GlobalState &global_state);

    /*
      TODO:
      The status of a landmark is actually dependent on the state. This
      is not represented in the function below. Furthermore, the status
      manager only stores the status for one particular state at a time.

      At the day of writing this comment, this works as
      *update_reached_lms()* is always called before the status
      information is used (by calling *get_landmark_status()*).

      It would be a good idea to ensure that the status for the
      desired state is returned at all times, or an error is thrown
      if the desired information does not exist.
    */
    landmark_status get_landmark_status(size_t id) const {
        assert(0 <= id && id < lm_graph.number_of_landmarks());
        return lm_status[id];
    }
};
}

#endif
