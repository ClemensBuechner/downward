#ifndef MERGE_AND_SHRINK_MERGE_STRATEGY_FACTORY_SCCS_H
#define MERGE_AND_SHRINK_MERGE_STRATEGY_FACTORY_SCCS_H

#include "merge_strategy_factory.h"

namespace merge_and_shrink {
class MergeTreeFactory;
class MergeSelector;

enum class OrderOfSCCs {
    TOPOLOGICAL,
    REVERSE_TOPOLOGICAL,
    DECREASING,
    INCREASING
};

class MergeStrategyFactorySCCs : public MergeStrategyFactory {
    OrderOfSCCs order_of_sccs;
    std::shared_ptr<MergeTreeFactory> merge_tree_factory;
    std::shared_ptr<MergeSelector> merge_selector;
protected:
    virtual std::string type() const override;
    virtual void dump_strategy_specific_options() const override;
public:
    MergeStrategyFactorySCCs(
        const OrderOfSCCs &order_of_sccs,
        const std::shared_ptr<MergeTreeFactory> merge_tree,
        const std::shared_ptr<MergeSelector> merge_selector,
        const std::string &name,
        utils::Verbosity verbosity);
    virtual std::unique_ptr<MergeStrategy> compute_merge_strategy(
        const TaskProxy &task_proxy,
        const FactoredTransitionSystem &fts) override;
    virtual bool requires_init_distances() const override;
    virtual bool requires_goal_distances() const override;
};
}

#endif
