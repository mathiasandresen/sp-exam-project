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
#include <sstream>
#include <fstream>
#include <chrono>
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

        reactant(std::string name, size_t initial_amount, size_t required):
                name(std::move(name)),
                amount(initial_amount),
                required(required)
        {}

        basic_reaction operator>>=(reactant other);

        basic_reaction operator>>=(reactant_collection other);

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

        void compute_delay(symbol_table<std::shared_ptr<reactant>>& reactants, std::default_random_engine& engine) {
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
                delay = std::exponential_distribution<double_t>(rate_k)(engine);
            } else {
                delay = -1;
            }
        }

        friend std::ostream &operator<<(std::ostream &s, const reaction &reaction);
    };

//    struct simulation_state {
//    public:
//        simulation_state(std::shared_ptr<symbol_table<std::shared_ptr<reactant>>> reactants, std::shared_ptr<std::vector<reaction>> reactions, std::shared_ptr<double_t> time):
//            reactants{reactants},
//            reactions{reactions},
//            time{time}
//        {};
//        const std::shared_ptr<symbol_table<std::shared_ptr<reactant>>> reactants;
//        const std::shared_ptr<std::vector<reaction>> reactions;
//        const std::shared_ptr<double_t> time;
//    };

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

        reactant environment() {
            if (reactants.contains("__env__")) {
               return *reactants.get("__env__");
            }

            std::shared_ptr<reactant> newReactant(new reactant("__env__", 0, 0));

            reactants.put(newReactant->name, newReactant);

            return *newReactant;
        }

        void visualize_reactions() {
            std::stringstream str;
            symbol_table<std::string> node_map{};

            str << "digraph {" << std::endl;

            auto i = 0;
            for (auto& reactant: reactants.getMap()) {
                if (reactant.second->name != "__env__") {
                    node_map.put(reactant.second->name, "s" + std::to_string(i));

                    str << node_map.get(reactant.second->name)
                        << "[label=\"" << reactant.second->name << "\",shape=\"box\",style=\"filled\",fillcolor=\"cyan\"];" << std::endl;
                    i++;
                }
            }

            i = 0;
            for (auto& reaction: reactions) {
                std::string reaction_node{"r" + std::to_string(i)};

                str << reaction_node << "[label=\"" << reaction.rate << "\",shape=\"oval\",style=\"filled\",fillcolor=\"yellow\"];" << std::endl;
                if (reaction.catalysts.has_value()) {
                    for (auto& catalyst: reaction.catalysts.value()) {
                        str << node_map.get(catalyst.name) << " -> " << reaction_node << " [arrowhead=\"tee\"];" << std::endl;
                    }
                }
                for (auto& reactant: reaction.basicReaction.from) {
                    if (reactant.name != "__env__") {
                        str << node_map.get(reactant.name) << " -> " << reaction_node << ";" << std::endl;
                    }
                }
                for (auto& product: reaction.basicReaction.to) {
                    if (product.name != "__env__") {
                        str << reaction_node << " -> " << node_map.get(product.name) << ";" << std::endl;
                    }
                }

                i++;
            }

            str << "}";

            std::ofstream dotfile;
            dotfile.open("graph.dot");
            dotfile << str.str();
            dotfile.close();

            system("dot -Tpng -o graph.png graph.dot");
        }

        symbol_table<std::shared_ptr<reactant>> getState() {
            return reactants;
        }

//        void do_simulation(double_t end_time, const std::function<void(double_t, symbol_table<std::shared_ptr<reactant>>, std::vector<reaction>)>& monitor) {
//            do_simulation(end_time, monitor)
//        }

        void do_simulation(double_t end_time, const std::function<void(double_t, symbol_table<std::shared_ptr<reactant>>, std::vector<reaction>)>& monitor) {
            double_t t{0};


//            std::random_device rd;
            std::default_random_engine engine{};

            auto epoch = std::chrono::system_clock::now().time_since_epoch().count();
//
//
            engine.seed(epoch);

            while (t <= end_time) {
                for (reaction& reaction: reactions) {
                    reaction.compute_delay(reactants, engine);
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
                        !r.catalysts.has_value() ||
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

                monitor(t, reactants, reactions);
            }
        }

        friend std::ostream& operator<<(std::ostream& s, const vessel_t& vessel);
    };



}

#endif //SP_EXAM_PROJECT_SIMULATION_H
