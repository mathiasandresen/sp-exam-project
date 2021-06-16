//
// Created by Mathias on 09-05-2021.
//

#include <iostream>
#include <utility>
#include "simulation.h"

namespace StochasticSimulation {


    // Requirement 2
    std::ostream &operator<<(std::ostream &s, const Vessel &vessel) {
        s << "{" << std::endl;
        for (const auto& reaction: vessel.reactions) {
            s << "\t" << reaction;
            if (&reaction != &vessel.reactions.back()) {
                s << ",";
            }
            s << std::endl;
        }
        return s << "}";
    }

    // Requirement 2
    void Vessel::visualize_reactions(const std::string& filename) {
        std::stringstream str;
        SymbolTable<std::string> node_map{};

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
        dotfile.open(filename + ".dot");
        dotfile << str.str();
        dotfile.close();

        std::stringstream command_builder;
        command_builder << "dot -Tpng -o " << filename << " " << filename << ".dot";
        system(command_builder.str().c_str());
    }

    // Requirement 10 alternative simulation
    std::shared_ptr<SimulationTrajectory> Vessel::do_simulation2(double_t end_time, simulation_monitor &monitor) {
        SimulationTrajectory trajectory{};
        double_t t{0};

        auto thread_id = std::this_thread::get_id();
        auto epoch = std::chrono::system_clock::now().time_since_epoch().count();
        std::default_random_engine engine(epoch * (std::hash<std::thread::id>{}(thread_id)));

        // Insert initial state
        trajectory.insert(SimulationState{reactants, t});

        while (t <= end_time) {
            for (Reaction& reaction: reactions) {
                // New: using new compute delay function
                reaction.compute_delay2(trajectory.at(t), engine);
            }

            auto r = reactions.front();

            // Select Reaction with min delay which is not -1
            for (auto& reaction: reactions) {
                if (reaction.delay == -1) {
                    continue;
                } else if (reaction.delay < r.delay) {
                    r = reaction;
                } else if (r.delay == -1) {
                    r = reaction;
                }
            }

            // Stop if we have no reactions to do, thus r.delay == -1
            if (r.delay == -1) {
                break;
            }

            auto& last_state = trajectory.at(t);

            t += r.delay;

            SimulationState state{last_state.reactants, t};

            if (
                    std::all_of(r.from.begin(), r.from.end(), [&state](const Reactant& e){return state.reactants.get(e.name).amount >= e.required;}) &&
                    (
                            !r.catalysts.has_value() ||
                            std::all_of(r.catalysts.value().begin(), r.catalysts.value().end(), [&state](const Reactant& e){return state.reactants.get(e.name).amount >= e.required;})
                    )
                    ) {
                for (auto& reactant: r.from) {
                    state.reactants.get(reactant.name).amount -= reactant.required;
                }
                for (auto& reactant: r.to) {
                    state.reactants.get(reactant.name).amount += reactant.required;
                }
            }

            trajectory.insert(std::move(state));

            monitor.monitor(trajectory.at(t));
        }

        return std::make_shared<SimulationTrajectory>(std::move(trajectory));
    }

    // Requirement 4 simulation
    std::shared_ptr<SimulationTrajectory> Vessel::do_simulation(double_t end_time, simulation_monitor &monitor) {
        SimulationTrajectory trajectory{};
        double_t t{0};

        auto thread_id = std::this_thread::get_id();
        auto epoch = std::chrono::system_clock::now().time_since_epoch().count();
        std::default_random_engine engine(epoch * (std::hash<std::thread::id>{}(thread_id)));

        // Insert initial state
        trajectory.insert(SimulationState{reactants, t});

        while (t <= end_time) {
            for (Reaction& reaction: reactions) {
                reaction.compute_delay(trajectory.at(t), engine);
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

            t += r.delay;

            SimulationState state{last_state.reactants, t};

            if (
                    std::all_of(r.from.begin(), r.from.end(), [&state](const Reactant& e){return state.reactants.get(e.name).amount >= e.required;}) &&
                    (
                            !r.catalysts.has_value() ||
                            std::all_of(r.catalysts.value().begin(), r.catalysts.value().end(), [&state](const Reactant& e){return state.reactants.get(e.name).amount >= e.required;})
                    )
                    ) {
                for (auto& reactant: r.from) {
                    state.reactants.get(reactant.name).amount -= reactant.required;
                }
                for (auto& reactant: r.to) {
                    state.reactants.get(reactant.name).amount += reactant.required;
                }
            }

            trajectory.insert(std::move(state));

            monitor.monitor(trajectory.at(t));
        }

        return std::make_shared<SimulationTrajectory>(std::move(trajectory));
    }

    // Requirement 8 multiple at same time
    std::vector<std::shared_ptr<SimulationTrajectory>>
    Vessel::do_multiple_simulations(double_t end_time, size_t simulations_to_run) {
        std::vector<std::shared_ptr<SimulationTrajectory>> result{};
        result.reserve(simulations_to_run);

        auto cores = std::thread::hardware_concurrency();
        int jobs = std::min(simulations_to_run, (cores - 1));
        auto simulations_per_job = simulations_to_run / jobs;

        auto futures = std::vector<std::future<std::vector<std::shared_ptr<SimulationTrajectory>>>>{};

        auto lambda = [&vessel = *this, &end_time](size_t to_run){
            auto simulations = std::vector<std::shared_ptr<SimulationTrajectory>>{};
            simulations.reserve(to_run);

            auto new_vessel = Vessel(vessel);

            for (int i = 0; i < to_run; ++i) {
                simulations.push_back(new_vessel.do_simulation(end_time));
            }

            return simulations;
        };

        for (int i = 0; i < jobs; ++i) {
            futures.push_back(std::async(std::launch::async, lambda, simulations_per_job));
        }
        auto missing_simulations = simulations_to_run - (simulations_per_job * jobs);
        if (missing_simulations != 0) {
            futures.push_back(std::async(std::launch::async, lambda, missing_simulations));
        }

        for (auto& future: futures) {
            auto future_result = future.get();
            for (auto& res: future_result) {
                result.push_back(std::move(res));
            }
        }

        return result;
    }

    // Private function for calculation interpolated value
    double_t SimulationTrajectory::compute_interpolated_value(const std::string& key, SimulationState& s0, SimulationState& s1, double_t x) {
        return
            s0.reactants.get(key).amount
            + ((
                   ( (double_t) s1.reactants.get(key).amount - (double_t) s0.reactants.get(key).amount) /
                   ( s1.time - s0.time )
               ) * (x - s0.time));
    }

    // Requirement 9 compute mean trajectory
    SimulationTrajectory SimulationTrajectory::compute_mean_trajectory(std::vector<std::shared_ptr<SimulationTrajectory>>& trajectories) {
        auto average_delay = trajectories.front()->get_max_time() / trajectories.front()->size();

        // Get a list of all keys
        std::vector<std::string> reactant_keys{};
        for (auto& reactant: trajectories.front()->begin()->second.reactants) {
            reactant_keys.push_back(reactant.second.name);
        }

        // Find upper bound for mean trajectory
        double_t upper_bound{-1.0};
        for (auto& trajectory: trajectories) {
            if (upper_bound == -1.0 || trajectory->get_max_time() < upper_bound) {
                upper_bound = trajectory->get_max_time();
            }
        }

        SimulationTrajectory mean_trajectory{};

        for (auto& trajectory: trajectories) {
            auto iterator = trajectory->begin();
            SimulationState& s0 = iterator->second;
            iterator++;
            SimulationState& s1 = iterator->second;

            double_t t{0};

            while((t + average_delay) <= upper_bound) {
                if (t >= s0.time) {
                    if (t <= s1.time) {
                        if (!mean_trajectory.contains(t)) {
                            mean_trajectory.insert((SimulationState{{}, t}));
                        }

                        for (auto& key: reactant_keys) {
                            auto interpolated_value = SimulationTrajectory::compute_interpolated_value(key, s0, s1, t);

                            auto& table = mean_trajectory.at(t).reactants;

                            if (!table.contains(key)) {
                                Reactant reactant{key, 0.0};
                                table.put(key, reactant);
                            }

                            table.get(key).amount += interpolated_value;
                        }

                        t += average_delay;
                    } else {
                        s0 = s1;
                        iterator++;

                        if (iterator != trajectory->end()) {
                            s1 = iterator->second;
                        } else {
                            break;
                        }
                    }
                }
            }
        }

        for (double_t i = 0; (i + average_delay) <= upper_bound; i += average_delay) {
            for (auto& key: reactant_keys) {
                mean_trajectory.at(i).reactants.get(key).amount /= trajectories.size();
            }
        }

        return std::move(mean_trajectory);
    }

    // Requirement 6 output to csv which can then be turned into a graph via python script
    void SimulationTrajectory::write_csv(const std::string &path) {
        std::ofstream csv_file;
        csv_file.open(path);

        auto reactants = at(0).reactants;

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
}

