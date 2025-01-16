#ifndef LANDMARKS_LANDMARK_FACTORY_H
#define LANDMARKS_LANDMARK_FACTORY_H

#include "landmark_graph.h"

#include "../utils/logging.h"

#include <list>
#include <map>
#include <memory>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class TaskProxy;

namespace plugins {
class Options;
class Feature;
}

namespace landmarks {
class LandmarkFactory {
    AbstractTask *lm_graph_task;
    std::vector<std::vector<std::vector<int>>> operators_eff_lookup;

    virtual void generate_landmarks(const std::shared_ptr<AbstractTask> &task) = 0;
    void generate_operators_lookups(const TaskProxy &task_proxy);

    // Moved here from landmark graph.
    void remove_node_occurrences(LandmarkNode *node);

protected:
    int num_conjunctive_landmarks;
    int num_disjunctive_landmarks;

    utils::HashMap<FactPair, LandmarkNode *> simple_landmarks_to_nodes;
    utils::HashMap<FactPair, LandmarkNode *> disjunctive_landmarks_to_nodes;
    std::vector<std::unique_ptr<LandmarkNode>> nodes;

    explicit LandmarkFactory(utils::Verbosity verbosity);
    mutable utils::LogProxy log;
    std::shared_ptr<LandmarkGraph> lm_graph;
    bool achievers_calculated = false;

    void edge_add(LandmarkNode &from, LandmarkNode &to, EdgeType type);

    void discard_all_orderings();

    bool is_landmark_precondition(const OperatorProxy &op,
                                  const Landmark &landmark) const;

    const std::vector<int> &get_operators_including_eff(const FactPair &eff) const {
        return operators_eff_lookup[eff.var][eff.value];
    }

    // Moved here from landmark graph.
    LandmarkNode &get_simple_landmark(const FactPair &fact) const;
    LandmarkNode &get_disjunctive_landmark(const FactPair &fact) const;

    int get_num_disjunctive_landmarks() const {
        return num_disjunctive_landmarks;
    }
    int get_num_conjunctive_landmarks() const {
        return num_conjunctive_landmarks;
    }

    bool contains_simple_landmark(const FactPair &lm) const;
    bool contains_disjunctive_landmark(const FactPair &lm) const;
    bool contains_overlapping_disjunctive_landmark(const std::set<FactPair> &lm) const;
    bool contains_identical_disjunctive_landmark(const std::set<FactPair> &lm) const;
    bool contains_landmark(const FactPair &fact) const;

    LandmarkNode &add_landmark_to_graph(Landmark &&landmark);
    void remove_node(LandmarkNode *node);
    void remove_node_if(
        const std::function<bool (const LandmarkNode &)> &remove_node_condition);

    void set_landmark_ids();

public:
    virtual ~LandmarkFactory() = default;
    LandmarkFactory(const LandmarkFactory &) = delete;

    std::shared_ptr<LandmarkGraph> compute_lm_graph(const std::shared_ptr<AbstractTask> &task);

    virtual bool supports_conditional_effects() const = 0;

    bool achievers_are_calculated() const {
        return achievers_calculated;
    }

};

extern void add_landmark_factory_options_to_feature(plugins::Feature &feature);
extern std::tuple<utils::Verbosity> get_landmark_factory_arguments_from_options(
    const plugins::Options &opts);
extern void add_use_orders_option_to_feature(plugins::Feature &feature);
extern bool get_use_orders_arguments_from_options(
    const plugins::Options &opts);
extern void add_only_causal_landmarks_option_to_feature(plugins::Feature &feature);
extern bool get_only_causal_landmarks_arguments_from_options(
    const plugins::Options &opts);
}

#endif
