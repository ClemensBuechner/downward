#include "landmark_factory_merged.h"

#include "landmark.h"
#include "landmark_graph.h"

#include "../plugins/plugin.h"

#include <set>

using namespace std;
using utils::ExitCode;

namespace landmarks {
class LandmarkNode;

LandmarkFactoryMerged::LandmarkFactoryMerged(
    const vector<shared_ptr<LandmarkFactory>> &lm_factories,
    utils::Verbosity verbosity)
    : LandmarkFactory(verbosity),
      landmark_factories(lm_factories) {
}

LandmarkNode *LandmarkFactoryMerged::get_matching_landmark(
    const Landmark &landmark) const {
    if (!landmark.is_disjunctive && !landmark.is_conjunctive) {
        const FactPair &atom = landmark.atoms[0];
        if (landmark_graph->contains_simple_landmark(atom))
            return &landmark_graph->get_simple_landmark_node(atom);
        else
            return nullptr;
    } else if (landmark.is_disjunctive) {
        const utils::HashSet<FactPair> atoms(
            landmark.atoms.begin(), landmark.atoms.end());
        if (landmark_graph->contains_identical_disjunctive_landmark(atoms))
            return &landmark_graph->get_disjunctive_landmark_node(landmark.atoms[0]);
        else
            return nullptr;
    } else if (landmark.is_conjunctive) {
        cerr << "Don't know how to handle conjunctive landmarks yet" << endl;
        utils::exit_with(ExitCode::SEARCH_UNSUPPORTED);
    }
    return nullptr;
}

void LandmarkFactoryMerged::generate_landmarks(
    const shared_ptr<AbstractTask> &task) {
    if (log.is_at_least_normal()) {
        log << "Merging " << landmark_factories.size()
            << " landmark graphs" << endl;
    }

    vector<shared_ptr<LandmarkGraph>> landmark_graphs;
    landmark_graphs.reserve(landmark_factories.size());
    achievers_calculated = true;
    for (const shared_ptr<LandmarkFactory> &landmark_factory : landmark_factories) {
        landmark_graphs.push_back(
            landmark_factory->compute_landmark_graph(task));
        achievers_calculated &= landmark_factory->achievers_are_calculated();
    }

    if (log.is_at_least_normal()) {
        log << "Adding simple landmarks" << endl;
    }
    for (size_t i = 0; i < landmark_graphs.size(); ++i) {
        // TODO: loop over landmarks instead
        for (const auto &node : *landmark_graphs[i]) {
            const Landmark &landmark = node->get_landmark();
            if (landmark.is_conjunctive) {
                cerr << "Don't know how to handle conjunctive landmarks yet" << endl;
                utils::exit_with(ExitCode::SEARCH_UNSUPPORTED);
            } else if (landmark.is_disjunctive) {
                continue;
            } else if (!landmark_graph->contains_landmark(landmark.atoms[0])) {
                Landmark copy(landmark);
                landmark_graph->add_landmark(move(copy));
            }
        }
    }

    if (log.is_at_least_normal()) {
        log << "Adding disjunctive landmarks" << endl;
    }
    for (const shared_ptr<LandmarkGraph> &graph_to_merge : landmark_graphs) {
        for (const auto &node : *graph_to_merge) {
            const Landmark &landmark = node->get_landmark();
            if (landmark.is_disjunctive) {
                /*
                  TODO: It seems that disjunctive landmarks are only added if
                  none of the atoms it is made of is also there as a simple
                  landmark. This should either be more general (add only if none
                  of its subset is already there) or it should be done only upon
                  request (e.g., heuristics that consider orders might want to
                  keep all landmarks).
                */
                bool exists =
                    any_of(landmark.atoms.begin(), landmark.atoms.end(),
                           [&](const FactPair &atom) {
                               return landmark_graph->contains_landmark(atom);
                           });
                if (!exists) {
                    Landmark copy(landmark);
                    landmark_graph->add_landmark(move(copy));
                }
            }
        }
    }

    if (log.is_at_least_normal()) {
        log << "Adding orderings" << endl;
    }
    for (size_t i = 0; i < landmark_graphs.size(); ++i) {
        for (const auto &from_orig : *landmark_graphs[i]) {
            LandmarkNode *from = get_matching_landmark(from_orig->get_landmark());
            if (from) {
                for (const auto &to : from_orig->children) {
                    const LandmarkNode *to_orig = to.first;
                    OrderingType type = to.second;
                    LandmarkNode *to_node = get_matching_landmark(to_orig->get_landmark());
                    if (to_node) {
                        add_ordering(*from, *to_node, type);
                    } else {
                        if (log.is_at_least_normal()) {
                            log << "Discarded to ordering" << endl;
                        }
                    }
                }
            } else {
                if (log.is_at_least_normal()) {
                    log << "Discarded from ordering" << endl;
                }
            }
        }
    }
    postprocess();
}

void LandmarkFactoryMerged::postprocess() {
    landmark_graph->set_landmark_ids();
}

bool LandmarkFactoryMerged::supports_conditional_effects() const {
    return all_of(landmark_factories.begin(), landmark_factories.end(),
                  [&](const shared_ptr<LandmarkFactory> &landmark_factory) {
                      return landmark_factory->supports_conditional_effects();
                  });
}

class LandmarkFactoryMergedFeature
    : public plugins::TypedFeature<LandmarkFactory, LandmarkFactoryMerged> {
public:
    LandmarkFactoryMergedFeature() : TypedFeature("lm_merged") {
        document_title("Merged Landmarks");
        document_synopsis(
            "Merges the landmarks and orderings from the parameter landmarks");

        add_list_option<shared_ptr<LandmarkFactory>>("lm_factories");
        add_landmark_factory_options_to_feature(*this);

        document_note(
            "Precedence",
            "Fact landmarks take precedence over disjunctive landmarks, "
            "orderings take precedence in the usual manner "
            "(gn > nat > reas > o_reas). ");
        document_note(
            "Note",
            "Does not currently support conjunctive landmarks");

        document_language_support(
            "conditional_effects",
            "supported if all components support them");
    }

    virtual shared_ptr<LandmarkFactoryMerged> create_component(
        const plugins::Options &opts,
        const utils::Context &context) const override {
        plugins::verify_list_non_empty<shared_ptr<LandmarkFactory>>(
            context, opts, "lm_factories");
        return plugins::make_shared_from_arg_tuples<LandmarkFactoryMerged>(
            opts.get_list<shared_ptr<LandmarkFactory>>("lm_factories"),
            get_landmark_factory_arguments_from_options(opts));
    }
};

static plugins::FeaturePlugin<LandmarkFactoryMergedFeature> _plugin;
}
