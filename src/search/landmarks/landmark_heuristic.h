#ifndef LANDMARKS_LANDMARK_HEURISTIC_H
#define LANDMARKS_LANDMARK_HEURISTIC_H

# include "../heuristic.h"

class ConstBitsetView;

namespace successor_generator {
class SuccessorGenerator;
}

namespace landmarks {
class LandmarkFactory;
class LandmarkGraph;
class LandmarkNode;
class LandmarkStatusManager;

enum class InterestingIf {LEGACY, FUTURE, PARENTS_PAST};

class LandmarkHeuristic : public Heuristic {
protected:
    std::shared_ptr<LandmarkGraph> lm_graph;
    const bool use_preferred_operators;
    const InterestingIf interesting_landmarks;

    std::unique_ptr<LandmarkStatusManager> lm_status_manager;
    std::unique_ptr<successor_generator::SuccessorGenerator> successor_generator;

    void initialize(const plugins::Options &opts);
    void compute_landmark_graph(const plugins::Options &opts);

    virtual int get_heuristic_value(const State &ancestor_state) = 0;

    void generate_preferred_operators(
        const State &state, ConstBitsetView &past,
        ConstBitsetView &future);

    bool landmark_is_interesting(
        const State &state, ConstBitsetView &past, ConstBitsetView &future,
        const landmarks::LandmarkNode &lm_node, bool all_lms_reached);
    virtual int compute_heuristic(const State &ancestor_state) override;
public:
    explicit LandmarkHeuristic(const plugins::Options &opts);

    virtual void get_path_dependent_evaluators(
        std::set<Evaluator *> &evals) override {
        evals.insert(this);
    }

    static void add_options_to_feature(plugins::Feature &feature);

    virtual void notify_initial_state(const State &initial_state) override;
    virtual void notify_state_transition(const State &parent_state,
                                         OperatorID op_id,
                                         const State &state) override;
};
}

#endif
