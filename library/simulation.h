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
#include <thread>
#include <future>
#include "symbol_table.h"
#include "simulation_monitor.h"
#include "data.h"

namespace StochasticSimulation {

    class simulation_trajectory: public std::map<double_t, simulation_state> {
    private:
        double_t largest_time{-1};
        static double_t compute_interpolated_value(const std::string& key, std::shared_ptr<simulation_state> s0, std::shared_ptr<simulation_state> s1, double_t x);
    public:
        using std::map<double_t, simulation_state>::map;

        simulation_trajectory(const simulation_trajectory&) = default;
        simulation_trajectory(simulation_trajectory&&) = default;

        static simulation_trajectory compute_mean_trajectory(std::vector<simulation_trajectory> trajectories);

        void insert(simulation_state state) {
            if (state.time > largest_time) {
                largest_time = state.time;
            }
            insert_or_assign(state.time, std::move(state));
        }

        void write_csv(const std::string& path) {
            std::ofstream csv_file;
            csv_file.open(path);

            auto reactants = begin()->second.reactants;

            for (auto& reactant : reactants) {
                csv_file << reactant.second.name << ",";
            }
            csv_file << "time" << std::endl;

            for (auto& state : *this) {
                for (auto& reactant: reactants) {
                    csv_file << state.second.reactants.get(reactant.second.name).amount << ",";
                }
                csv_file << state.second.time << std::endl;
            }

            csv_file.close();
        }

        double_t get_max_time() {
            return largest_time;
        }
    };

    class vessel_t {
    private:
        std::vector<reaction> reactions{};
        symbol_table<reactant> reactants;
    public:
//        simulation_trajectory trajectory{};

        reactant operator()(std::string name, size_t initial_amount) {
            reactant newReactant{std::move(name), initial_amount};

            reactants.put(newReactant.name, newReactant);

            return newReactant;
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

        symbol_table<reactant> get_reactants() {
            return reactants;
        }


        simulation_trajectory do_simulation(double_t end_time, simulation_monitor& monitor) {
            simulation_trajectory trajectory{};

            double_t t{0};
            std::default_random_engine engine{};
            auto epoch = std::chrono::system_clock::now().time_since_epoch().count();
            engine.seed(epoch);

            simulation_state initial_state{reactants, t};
            trajectory.insert(std::move(initial_state));

            while (t <= end_time) {
                for (reaction& reaction: reactions) {
                    reaction.compute_delay(trajectory.at(t), engine);
                }

                auto r = reactions.front();

                // Select reaction with min delay which is not -1
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

                auto last_state = trajectory.at(t);

                t += r.delay;

                simulation_state state{last_state.reactants, t};

                if (
                    std::all_of(r.basicReaction.from.begin(), r.basicReaction.from.end(), [&state](reactant& e){return state.reactants.get(e.name).amount >= e.required;}) &&
                    (
                        !r.catalysts.has_value() ||
                        std::all_of(r.catalysts.value().begin(), r.catalysts.value().end(), [&state](reactant& e){return state.reactants.get(e.name).amount >= e.required;})
                    )
                ) {
                    for (auto& reactant: r.basicReaction.from) {
                        state.reactants.get(reactant.name).amount -= reactant.required;
                    }
                    for (auto& reactant: r.basicReaction.to) {
                        state.reactants.get(reactant.name).amount += reactant.required;
                    }
                }

                trajectory.insert(std::move(state));

                monitor.monitor(trajectory.at(t));
            }

            return trajectory;
        }

        simulation_trajectory do_simulation(double_t end_time) {
            return do_simulation(end_time, EMPTY_SIMULATION_MONITOR);
        }

        std::vector<simulation_trajectory> do_multiple_simulations(double_t end_time, size_t simulations_to_run) {
            //TODO: only doing 48 when set to 50 - this is because of the simulations_per_thread calculation
            //TODO: exception when doing circadian
            auto result = std::vector<simulation_trajectory>{};
            auto futures = std::vector<std::future<std::vector<simulation_trajectory>>>{};

            auto cores = std::thread::hardware_concurrency();
            auto amount_of_threads = std::min(cores, simulations_to_run);
            auto simulations_per_thread = simulations_to_run / amount_of_threads;

            for (int i = 1; i <= amount_of_threads; i++ ) {
                std::future<std::vector<simulation_trajectory>> future = std::async(std::launch::async, [&vessel = *this, &end_time, &simulations_per_thread](){
                    std::vector<simulation_trajectory> trajectories{};
                    for (int j = 0; j < simulations_per_thread; ++j) {
                        auto trajectory = vessel.do_simulation(end_time);
                        trajectories.push_back(std::move(trajectory));
                    }
                    return trajectories;
                });

                futures.push_back(std::move(future));
            }

//            for (int i = 0; i < simulations_to_run; ++i) {
//                auto future = std::async(std::launch::async, &vessel_t::do_simulation, this, end_time, EMPTY_SIMULATION_MONITOR);
//                futures.push_back(std::move(future));
////                std::packaged_task<simulation_trajectory(double_t, const std::function<void(simulation_state&)>&)> task;
////                std::future<simulation_trajectory> future = task.get_future();
////                std::thread thread{std::move()};
//            }

            for (auto& future: futures) {
                auto trajectories = future.get();
                for (auto& t: trajectories) {
                    result.push_back(std::move(t));
                }
            }


            return result;
        }

        friend std::ostream& operator<<(std::ostream& s, const vessel_t& vessel);
    };



}

#endif //SP_EXAM_PROJECT_SIMULATION_H
