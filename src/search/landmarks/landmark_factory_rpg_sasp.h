#ifndef LANDMARKS_LANDMARK_FACTORY_RPG_SASP_H
#define LANDMARKS_LANDMARK_FACTORY_RPG_SASP_H

#include "landmark_factory_relaxation.h"

#include "../utils/hash.h"

#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace landmarks {
class LandmarkFactoryRpgSasp : public LandmarkFactoryRelaxation {
    const bool disjunctive_landmarks;
    const bool use_orders;
    const bool only_causal_landmarks;
    std::list<LandmarkNode *> open_landmarks;
    std::vector<std::vector<int>> disjunction_classes;

    std::unordered_map<const LandmarkNode *, utils::HashSet<FactPair>> forward_orders;

    // dtg_successors[var_id][val] contains all successor values of val in the
    // domain transition graph for the variable
    std::vector<std::vector<std::unordered_set<int>>> dtg_successors;

    void build_dtg_successors(const TaskProxy &task_proxy);
    void add_dtg_successor(int var_id, int pre, int post);
    void find_forward_orders(const VariablesProxy &variables,
                             const std::vector<std::vector<bool>> &reached,
                             LandmarkNode *node);
    void add_landmark_forward_orderings();

    void get_greedy_preconditions_for_landmark(
        const TaskProxy &task_proxy, const Landmark &landmark,
        const OperatorProxy &op,
        std::unordered_map<int, int> &result) const;
    void compute_shared_preconditions(
        const TaskProxy &task_proxy,
        std::unordered_map<int, int> &shared_pre,
        std::vector<std::vector<bool>> &reached, const Landmark &landmark);
    void compute_disjunctive_preconditions(
        const TaskProxy &task_proxy,
        std::vector<utils::HashSet<FactPair>> &disjunctive_pre,
        std::vector<std::vector<bool>> &reached,
        const Landmark &landmark);

    virtual void generate_relaxed_landmarks(
        const std::shared_ptr<AbstractTask> &task,
        Exploration &exploration) override;
    void found_simple_landmark_and_ordering(const FactPair &atom, LandmarkNode &node,
                                            OrderingType type);
    void found_disjunctive_landmark_and_ordering(const TaskProxy &task_proxy,
                                                 const utils::HashSet<FactPair> &atoms,
                                                 LandmarkNode &node,
                                                 OrderingType type);
    void approximate_lookahead_orders(
        const TaskProxy &task_proxy,
        const std::vector<std::vector<bool>> &reached, LandmarkNode *node);
    bool domain_connectivity(const State &initial_state,
                             const FactPair &landmark,
                             const std::unordered_set<int> &exclude);

    void build_disjunction_classes(const TaskProxy &task_proxy);

    void discard_disjunctive_landmarks();
public:
    LandmarkFactoryRpgSasp(
        bool disjunctive_landmarks, bool use_orders,
        bool only_causal_landmarks, utils::Verbosity verbosity);

    virtual bool supports_conditional_effects() const override;
};
}

#endif
