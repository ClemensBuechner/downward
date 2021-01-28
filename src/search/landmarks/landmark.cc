#include "landmark.h"

#include <algorithm>

using namespace std;

namespace landmarks {
bool AtomicFactLandmark::is_true_in_state(
    const GlobalState &state) const {

    return state[fact.var] == fact.value;
}

bool AtomicFactLandmark::is_true_in_state(const State &state) const {
    return state[fact.var].get_value() == fact.value;
}

bool DisjunctiveFactLandmark::is_true_in_state(
    const GlobalState &state) const {

    return any_of(facts.begin(), facts.end(),
                  [state](const FactPair &fact) {
                      return state[fact.var] == fact.value;
                  });
}

bool DisjunctiveFactLandmark::is_true_in_state(
    const State &state) const {

    return any_of(facts.begin(), facts.end(),
                  [state](const FactPair &fact) {
                      return state[fact.var].get_value() == fact.value;
                  });
}

bool ConjunctiveFactLandmark::is_true_in_state(
    const GlobalState &state) const {

    return all_of(facts.begin(), facts.end(),
                  [state](const FactPair &fact) {
                      return state[fact.var] == fact.value;
                  });
}

bool ConjunctiveFactLandmark::is_true_in_state(
    const State &state) const {

    return all_of(facts.begin(), facts.end(),
                  [state](const FactPair &fact) {
                      return state[fact.var].get_value() == fact.value;
                  });
}
}