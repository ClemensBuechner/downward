#ifndef LANDMARKS_LANDMARK_FACTORY_REASONABLE_ORDERS_HPS_H
#define LANDMARKS_LANDMARK_FACTORY_REASONABLE_ORDERS_HPS_H

#include "landmark_factory.h"

namespace landmarks {
class LandmarkFactoryReasonableOrdersHPS : public LandmarkFactory {
    std::shared_ptr<LandmarkFactory> landmark_factory;

    virtual void generate_landmarks(
        const std::shared_ptr<AbstractTask> &task) override;

    void approximate_goal_orderings(
        const TaskProxy &task_proxy, LandmarkNode &node) const;
    std::unordered_set<LandmarkNode *> collect_reasonable_ordering_candidates(
        const LandmarkNode &node);
    void insert_reasonable_orderings(
        const TaskProxy &task_proxy,
        const std::unordered_set<LandmarkNode *> &candidates,
        LandmarkNode &node, const Landmark &landmark) const;
    void approximate_reasonable_orderings(const TaskProxy &task_proxy);
    bool interferes(
        const TaskProxy &task_proxy, const Landmark &landmark_a,
        const Landmark &landmark_b) const;
    void collect_ancestors(
        std::unordered_set<LandmarkNode *> &result, LandmarkNode &node);
    bool effect_always_happens(
        const VariablesProxy &variables, const EffectsProxy &effects,
        std::set<FactPair> &eff) const;
public:
    LandmarkFactoryReasonableOrdersHPS(
        const std::shared_ptr<LandmarkFactory> &lm_factory,
        utils::Verbosity verbosity);

    virtual bool supports_conditional_effects() const override;
};
}

#endif
