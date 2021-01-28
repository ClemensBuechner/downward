#ifndef LANDMARKS_LANDMARK_H
#define LANDMARKS_LANDMARK_H

#include "../task_proxy.h"

#include <set>

namespace landmarks {
class Landmark {
public:
    virtual ~Landmark() = default;
};

class FactLandmark : public Landmark {
public:
    virtual bool is_true_in_state(const GlobalState &state) const = 0;
    virtual bool is_true_in_state(const State &state) const = 0;
    virtual std::set<int> get_first_achievers() const = 0;
    virtual std::set<int> get_possible_achievers() const = 0;
};

class AtomicFactLandmark : public FactLandmark {
    FactPair &fact;
public:
    bool is_true_in_state(const GlobalState &state) const override;
    bool is_true_in_state(const State &state) const override;
};

class DisjunctiveFactLandmark : public FactLandmark {
    std::vector<FactPair> facts;
public:
    virtual bool is_true_in_state(const GlobalState &state) const override;
    virtual bool is_true_in_state(const State &state) const override;
};

class ConjunctiveFactLandmark : public FactLandmark {
    std::vector<FactPair> facts;
public:
    virtual bool is_true_in_state(const GlobalState &state) const override;
    virtual bool is_true_in_state(const State &state) const override;
};
}

#endif
