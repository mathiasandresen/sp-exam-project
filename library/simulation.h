//
// Created by Mathias on 09-05-2021.
//

#ifndef SP_EXAM_PROJECT_SIMULATION_H
#define SP_EXAM_PROJECT_SIMULATION_H

#include <string>
#include <utility>
#include <vector>
#include <set>
#include <optional>
#include <map>
#include <numeric>
#include <random>
#include <algorithm>
#include <functional>
#include <sstream>
#include <fstream>
#include <chrono>
#include <thread>
#include <future>
#include <ranges>
#include "symbol_table.h"
#include "simulation_monitor.h"
#include "data.h"
#include "../include/thread-pool.hpp"

namespace StochasticSimulation {

    using map_type = std::map<double_t, std::shared_ptr<simulation_state>>;
    class simulation_trajectory: public map_type {
    private:
        double_t largest_time{-1};
        static double_t compute_interpolated_value(
                const std::string& key,
                simulation_state& s0,
                simulation_state& s1,
                double_t x);
    public:
        using map_type::map;

        simulation_trajectory(const simulation_trajectory& val): map_type(val) {
            std::cout << "copy constructor" << std::endl;

            largest_time = val.largest_time;
        }

        simulation_trajectory(simulation_trajectory&& rval): map_type(std::move(rval)) {
            std::cout << "move constructor" << std::endl;

            largest_time = std::move(rval.largest_time);
        };

        simulation_trajectory& operator=(const simulation_trajectory & val) {
            std::cout << "copy operator" << std::endl;
            map_type::operator=(val);
            largest_time = val.largest_time;
        };

        simulation_trajectory& operator=(simulation_trajectory&& rval) {
            std::cout << "move operator" << std::endl;
            map_type::operator=(std::move(rval));
            largest_time = std::move(rval.largest_time);
            return *this;
        };

        static simulation_trajectory compute_mean_trajectory(std::vector<std::shared_ptr<simulation_trajectory>>& trajectories);

        void insert(std::shared_ptr<simulation_state> state) {
            if (state->time > largest_time) {
                largest_time = state->time;
            }

            if (contains(state->time)) {
                std::cout << "already contains time" << std::endl;
            }

            insert_or_assign(state->time, std::move(state));
        }

        void write_csv(const std::string& path) {
            std::ofstream csv_file;
            csv_file.open(path);

            auto reactants = at(0)->reactants;

            for (auto& reactant : reactants) {
                csv_file << reactant.second.name << ",";
            }
            csv_file << "time" << std::endl;

            for (auto& state : *this) {
                for (auto& reactant: reactants) {
                    csv_file << state.second->reactants.get(reactant.second.name).amount << ",";
                }
                csv_file << state.second->time << std::endl;
            }

            csv_file.close();
        }

        double_t get_max_time() {
            return largest_time;
        }
    };

    class vessel_t {
    private:
        std::vector<Reaction> reactions{};
        symbol_table<reactant> reactants;
    public:
//        simulation_trajectory trajectory{};

        reactant operator()(std::string name, size_t initial_amount) {
            reactant newReactant{std::move(name), initial_amount};

            reactants.put(newReactant.name, newReactant);

            return newReactant;
        }

        Reaction operator()(Reaction&& reaction, double_t rate) {
            reaction.rate = rate;

            reactions.push_back(reaction);

            return reaction;
        }

        Reaction operator()(Reaction&& reaction, std::initializer_list<reactant> catalysts, double rate) {
            reaction.rate = rate;
            reaction.catalysts = catalysts;

            // Add to vessel reactions
            reactions.push_back(reaction);

            return reaction;
        }

        Reaction operator()(Reaction&& reaction, reactant catalyst, double_t rate) {
            reaction.rate = rate;
            reaction.catalysts = {catalyst};

            // Add to vessel reactions
            reactions.push_back(reaction);

            return reaction;
        }


        reactant environment() {
            if (reactants.contains("__env__")) {
               return reactants.get("__env__");
            }

            std::shared_ptr<reactant> newReactant(new reactant("__env__", 0, 0));

            reactants.put(newReactant->name, *newReactant);

            return *newReactant;
        }

        void visualize_reactions() {
            std::stringstream str;
            symbol_table<std::string> node_map{};

            str << "digraph {" << std::endl;

            auto i = 0;
            for (auto& reactant: reactants.getMap()) {
                if (reactant.second.name != "__env__") {
                    node_map.put(reactant.second.name, "s" + std::to_string(i));

                    str << node_map.get(reactant.second.name)
                        << "[label=\"" << reactant.second.name << "\",shape=\"box\",style=\"filled\",fillcolor=\"cyan\"];" << std::endl;
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
                for (auto& reactant: reaction.from) {
                    if (reactant.name != "__env__") {
                        str << node_map.get(reactant.name) << " -> " << reaction_node << ";" << std::endl;
                    }
                }
                for (auto& product: reaction.to) {
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

        symbol_table<reactant> get_reactants() {
            return reactants;
        }

        std::shared_ptr<simulation_trajectory> do_simulation2(double_t end_time, simulation_monitor& monitor = EMPTY_SIMULATION_MONITOR) {
            auto thread_id = std::this_thread::get_id();
            std::cout << "Beginning simulation2 (thread " << thread_id << ")" << std::endl;

            simulation_trajectory trajectory{};
            std::set<std::string> affected_reactants{};

            double_t t{0};
            std::default_random_engine engine{};
            auto epoch = std::chrono::system_clock::now().time_since_epoch().count();
            engine.seed(epoch * (std::hash<std::thread::id>{}(thread_id)));

            // Insert initial state
            try {
                trajectory.insert(std::make_shared<simulation_state>(simulation_state{reactants, t}));
            } catch (const std::exception& exception) {
                std::cout << exception.what() << std::endl;
            }

            while (t <= end_time) {
                for (Reaction& reaction: reactions) {
                    try {
                        // New: using new compute delay function
                        reaction.compute_delay2(*trajectory.at(t), engine);
                    } catch (const std::exception& exception) {
                        std::cout << exception.what() << std::endl;
                    }
                }

                auto r = reactions.front();

                // Select Reaction with min delay which is not -1
                for (auto& reaction: reactions) {
                    if (reaction.delay == -1) {
                        continue;
                    } else if (reaction.delay < r.delay) {
                        r = reaction;
                    } else if (r.delay == -1 && reaction.delay != -1) {
                        r = reaction;
                    }
                }

                // Stop if we have no reactions to do, thus r.delay == -1
                if (r.delay == -1) {
                    break;
                }

                auto& last_state = trajectory.at(t);

                if (last_state->reactants.getMap().size() == 0) {
                    std::cout << "what?" << std::endl;
                }

                t += r.delay;

                simulation_state state{last_state->reactants, t};

                affected_reactants.clear();

                if (
                        std::all_of(r.from.begin(), r.from.end(), [&state](reactant e){return state.reactants.get(e.name).amount >= e.required;}) &&
                        (
                                !r.catalysts.has_value() ||
                                std::all_of(r.catalysts.value().begin(), r.catalysts.value().end(), [&state](reactant e){return state.reactants.get(e.name).amount >= e.required;})
                        )
                        ) {
                    for (auto& reactant: r.from) {
                        state.reactants.get(reactant.name).amount -= reactant.required;
                        affected_reactants.insert(reactant.name);
                    }
                    for (auto& reactant: r.to) {
                        state.reactants.get(reactant.name).amount += reactant.required;
                        affected_reactants.insert(reactant.name);
                    }
                }

                try {
                    trajectory.insert(std::make_shared<simulation_state>(state));
                } catch (const std::exception& exception) {
                    std::cout << " tried to insert " << state << std::endl;
                    std::cout << exception.what() << std::endl;
                }

                monitor.monitor(*trajectory.at(t));
            }

            try {
                return std::make_shared<simulation_trajectory>(std::move(trajectory));
            } catch (const std::exception& exception) {
                std::cout << exception.what() << std::endl;
            }

        }

        std::shared_ptr<simulation_trajectory> do_simulation(double_t end_time, simulation_monitor& monitor = EMPTY_SIMULATION_MONITOR) {
            auto thread_id = std::this_thread::get_id();
            std::cout << "Beginning simulation (thread " << thread_id << ")" << std::endl;

            simulation_trajectory trajectory{};

            double_t t{0};
            std::default_random_engine engine{};
            auto epoch = std::chrono::system_clock::now().time_since_epoch().count();
            engine.seed(epoch * (std::hash<std::thread::id>{}(thread_id)));

            // Insert initial state
            try {
                 trajectory.insert(std::make_shared<simulation_state>(simulation_state{reactants, t}));
            } catch (const std::exception& exception) {
                std::cout << exception.what() << std::endl;
            }

            while (t <= end_time) {
                for (Reaction& reaction: reactions) {
                    try {
                        reaction.compute_delay(*trajectory.at(t), engine);
                    } catch (const std::exception& exception) {
                        std::cout << exception.what() << std::endl;
                    }
                }

                auto r = reactions.front();

                // Select Reaction with min delay which is not -1
                for (auto& reaction: reactions) {
                    if (reaction.delay == -1) {
                        continue;
                    } else if (reaction.delay < r.delay) {
                        r = reaction;
                    } else if (r.delay == -1 && reaction.delay != -1) {
                        r = reaction;
                    }
                }

                // Stop if we have no reactions to do, thus r.delay == -1
                if (r.delay == -1) {
                    break;
                }

                auto& last_state = trajectory.at(t);

                if (last_state->reactants.getMap().size() == 0) {
                    std::cout << "what?" << std::endl;
                }

                t += r.delay;

                simulation_state state{last_state->reactants, t};

                if (
                    std::all_of(r.from.begin(), r.from.end(), [&state](const reactant& e){return state.reactants.get(e.name).amount >= e.required;}) &&
                    (
                        !r.catalysts.has_value() ||
                        std::all_of(r.catalysts.value().begin(), r.catalysts.value().end(), [&state](const reactant& e){return state.reactants.get(e.name).amount >= e.required;})
                    )
                ) {
                    for (auto& reactant: r.from) {
                        state.reactants.get(reactant.name).amount -= reactant.required;
                    }
                    for (auto& reactant: r.to) {
                        state.reactants.get(reactant.name).amount += reactant.required;
                    }
                }

                try {
                    trajectory.insert(std::make_shared<simulation_state>(state));
                } catch (const std::exception& exception) {
                    std::cout << " tried to insert " << state << std::endl;
                    std::cout << exception.what() << std::endl;
                }

                monitor.monitor(*trajectory.at(t));
            }

            try {
                return std::make_shared<simulation_trajectory>(std::move(trajectory));
            } catch (const std::exception& exception) {
                std::cout << exception.what() << std::endl;
            }
        }

        std::vector<std::shared_ptr<simulation_trajectory>> do_multiple_simulations(double_t end_time, size_t simulations_to_run) {
            //TODO: exception when doing circadian
            auto futures = std::vector<std::future<simulation_trajectory>>{};

            auto pool = thread_pool();

            for (int i = 0; i < simulations_to_run; ++i) {
                futures.push_back(pool.async([&vessel = *this](double_t end_time){
                    auto result = vessel.do_simulation(end_time);
                    std::cout << "finished simulation" << std::endl;
                    return std::move(*result);
                }, end_time));

            }

            std::vector<std::shared_ptr<simulation_trajectory>> result{};
            result.reserve(futures.size());

                for (auto& fut: futures) {
                    try {
                        auto val = fut.get();
                        result.push_back(std::make_shared<simulation_trajectory>(std::move(val)));
                    } catch (const std::exception& exception) {
                        std::cout << exception.what() << std::endl;
                    }
                }

            return result;
        }

        friend std::ostream& operator<<(std::ostream& s, const vessel_t& vessel);
    };



}

#endif //SP_EXAM_PROJECT_SIMULATION_H
