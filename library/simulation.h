//
// Created by Mathias on 09-05-2021.
//

#ifndef SP_EXAM_PROJECT_SIMULATION_H
#define SP_EXAM_PROJECT_SIMULATION_H

#include <string>
#include <utility>
#include <vector>
#include <optional>
#include <map>
#include <numeric>
#include <random>
#include <algorithm>
#include <functional>
#include "symbol_table.h"

namespace StochasticSimulation {

    struct basic_reaction;
    class reactant_collection;

    struct reactant {
        std::string name;
        size_t amount;
        size_t required{1}; //TODO: This should properly be changed

        reactant(std::string name, size_t initial_amount):
            name(std::move(name)),
            amount(initial_amount)
        {}

        basic_reaction operator>>=(reactant other);

        reactant_collection operator+(reactant other);

        reactant operator*(size_t req) {
            required = req;
            return *this;
        }

    };

    class reactant_collection: public std::vector<reactant> {
    public:
        using std::vector<reactant>::vector;
        basic_reaction operator>>=(reactant other);
        basic_reaction operator>>=(reactant_collection other);
    };

    struct basic_reaction {
        reactant_collection from;
        reactant_collection to;
    };

    class reaction {
    private:
        std::default_random_engine generator;
    public:
        basic_reaction basicReaction;
        std::optional<std::vector<reactant>> catalysts;
        double_t rate{};
        double_t delay{-1};

        reaction(basic_reaction basicReaction, std::initializer_list<reactant> catalysts, double rate):
            basicReaction(std::move(basicReaction)),
            catalysts(catalysts),
            rate(rate)
        {}

        reaction(basic_reaction basicReaction, double rate):
                basicReaction(std::move(basicReaction)),
                catalysts{},
                rate(rate)
        {}

        void compute_delay(symbol_table<std::shared_ptr<reactant>>& reactants) {
            size_t reactant_amount{1};
            size_t catalyst_amount{1};

            for (auto& reactant: basicReaction.from) {
                reactant_amount *= reactants.get(reactant.name)->amount;
            }
            if (catalysts.has_value()) {
                for (auto& catalyst: catalysts.value()) {
                    catalyst_amount *= reactants.get(catalyst.name)->amount;
                }
            }

            double_t rate_k = rate * reactant_amount * catalyst_amount;

            if (rate_k > 0) {
                delay = std::exponential_distribution<double_t>(rate_k)(generator);
            } else {
                delay = -1;
            }
        }

        friend std::ostream &operator<<(std::ostream &s, const reaction &reaction);
    };

    class vessel_t {
    private:
        std::vector<reaction> reactions{};
        symbol_table<std::shared_ptr<reactant>> reactants;
    public:
        reactant operator()(std::string name, size_t initial_amount) {
            std::shared_ptr<reactant> newReactant(new reactant(std::move(name), initial_amount));

            reactants.put(newReactant->name, newReactant);

            return *newReactant;
        }

        reaction operator()(basic_reaction basicReaction, std::initializer_list<reactant> catalysts, double rate) {
            reaction newReaction{std::move(basicReaction), catalysts, rate};

            // Add to vessel reactions
            reactions.push_back(newReaction);

            return newReaction;
        }

        reaction operator()(basic_reaction basicReaction, reactant catalyst, double_t rate) {
            reaction newReaction{std::move(basicReaction), {std::move(catalyst)}, rate};

            // Add to vessel reactions
            reactions.push_back(newReaction);

            return newReaction;
        }

        reaction operator()(basic_reaction basicReaction, double rate) {
            reaction newReaction{std::move(basicReaction), rate};

            // Add to vessel reactions
            reactions.push_back(newReaction);

            return newReaction;
        }

        void visualize_reactions() {
            //TODO: this
            system("dot -Tpng -o ../graph.png ../graph.dot");
        }

        symbol_table<std::shared_ptr<reactant>> getState() {
            return reactants;
        }

        void do_simulation(double_t end_time, const std::function<void(symbol_table<std::shared_ptr<reactant>>, double_t)>& monitor) {
            double_t t{0};

            while (t <= end_time) {
                for (reaction& reaction: reactions) {
                    reaction.compute_delay(reactants);
                }

//                std::sort(reactions.begin(), reactions.end(), [](reaction& a, reaction& b){return a.delay < b.delay;});
//                auto lowerBound = std::lower_bound(reactions.begin(), reactions.end(), 0, [](reaction& a, double_t value){return a.delay < value;});
//                auto r = *std::min_element(reactions.begin(), reactions.end(), [](reaction& a, reaction& b) {
//                    return a.delay < b.delay);
//                });

                auto r = reactions.front();

                for (auto& reaction: reactions) {
                    if (reaction.delay == -1) {
                        continue;
                    } else if (reaction.delay < r.delay) {
                        r = reaction;
                    } else if (r.delay == -1 && reaction.delay != -1) {
                        r = reaction;
                    }
                }

                if (r.delay == -1) {
                    return;
                }

                t += r.delay;

                if (
                    std::all_of(r.basicReaction.from.begin(), r.basicReaction.from.end(), [&reactants = reactants](reactant& e){return reactants.get(e.name)->amount >= e.required;}) &&
                    (
                        r.catalysts->empty() ||
                        std::all_of(r.catalysts.value().begin(), r.catalysts.value().end(), [&reactants = reactants](reactant& e){return reactants.get(e.name)->amount >= e.required;})
                    )
                ) {
                    for (auto& reactant: r.basicReaction.from) {
                        reactants.get(reactant.name)->amount -= reactant.required;
                    }
                    for (auto& reactant: r.basicReaction.to) {
                        reactants.get(reactant.name)->amount += reactant.required;
                    }
                }

//                std::cout << "selected: " << r << std::endl;

                monitor(reactants, t);
            }
        }

        friend std::ostream& operator<<(std::ostream& s, const vessel_t& vessel);
    };



}

#endif //SP_EXAM_PROJECT_SIMULATION_H
