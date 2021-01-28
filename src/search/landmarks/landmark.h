#ifndef LANDMARKS_LANDMARK_H
#define LANDMARKS_LANDMARK_H

#include "../global_state.h"

namespace landmarks {
class Landmark {
public:
    virtual ~Landmark() = default;
    virtual bool is_true_in_state(const GlobalState &state) const = 0;
    virtual bool is_true_in_state(const State &state) const = 0;
};
}

#endif
