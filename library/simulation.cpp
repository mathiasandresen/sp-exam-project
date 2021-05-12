//
// Created by Mathias on 09-05-2021.
//

#include <iostream>
#include <utility>
#include "simulation.h"

namespace StochasticSimulation {


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

    void Vessel::visualize_reactions() {
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
        dotfile.open("graph.dot");
        dotfile << str.str();
        dotfile.close();

        system("dot -Tpng -o graph.png graph.dot");
    }

    std::shared_ptr<simulation_trajectory> Vessel::do_simulation2(double_t end_time, simulation_monitor &monitor) {
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
            trajectory.insert(std::make_shared<SimulationState>(SimulationState{reactants, t}));
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

            SimulationState state{last_state->reactants, t};

            affected_reactants.clear();

            if (
                    std::all_of(r.from.begin(), r.from.end(), [&state](Reactant e){return state.reactants.get(e.name).amount >= e.required;}) &&
                    (
                            !r.catalysts.has_value() ||
                            std::all_of(r.catalysts.value().begin(), r.catalysts.value().end(), [&state](Reactant e){return state.reactants.get(e.name).amount >= e.required;})
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
                trajectory.insert(std::make_shared<SimulationState>(state));
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

    std::shared_ptr<simulation_trajectory> Vessel::do_simulation(double_t end_time, simulation_monitor &monitor) {
        auto thread_id = std::this_thread::get_id();
        std::cout << "Beginning simulation (thread " << thread_id << ")" << std::endl;

        simulation_trajectory trajectory{};

        double_t t{0};
        auto epoch = std::chrono::system_clock::now().time_since_epoch().count();
        auto seed = epoch * (std::hash<std::thread::id>{}(thread_id));
        std::default_random_engine engine(seed);

        // Insert initial state
        try {
            trajectory.insert(std::make_shared<SimulationState>(SimulationState{reactants, t}));
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

            SimulationState state{last_state->reactants, t};

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

            try {
                trajectory.insert(std::make_shared<SimulationState>(state));
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

    std::vector<std::shared_ptr<simulation_trajectory>>
    Vessel::do_multiple_simulations(double_t end_time, size_t simulations_to_run) {
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

    double_t simulation_trajectory::compute_interpolated_value(const std::string& key, SimulationState& s0, SimulationState& s1, double_t x) {
        return
            s0.reactants.get(key).amount
            + ((
                   ( (double_t) s1.reactants.get(key).amount - (double_t) s0.reactants.get(key).amount) /
                   ( s1.time - s0.time )
               ) * (x - s0.time));
    }

    simulation_trajectory simulation_trajectory::compute_mean_trajectory(std::vector<std::shared_ptr<simulation_trajectory>>& trajectories) {
        auto average_delay = trajectories.front()->get_max_time() / trajectories.front()->size();

        // Get a list of all keys
        std::vector<std::string> reactant_keys{};
        for (auto& reactant: trajectories.front()->begin()->second->reactants) {
            reactant_keys.push_back(reactant.second.name);
        }

        // Find upper bound for mean trajectory
        double_t upper_bound{-1.0};
        for (auto& trajectory: trajectories) {
            if (upper_bound == -1.0 || trajectory->get_max_time() < upper_bound) {
                upper_bound = trajectory->get_max_time();
            }
        }

        simulation_trajectory mean_trajectory{};

        for (auto& trajectory: trajectories) {
            auto iterator = trajectory->begin();
            SimulationState& s0 = *iterator->second;
            iterator++;
            SimulationState& s1 = *iterator->second;

            double_t t{0};

            while((t + average_delay) <= upper_bound) {
                if (t >= s0.time) {
                    if (t <= s1.time) {
                        if (!mean_trajectory.contains(t)) {
                            mean_trajectory.insert(std::make_shared<SimulationState>(SimulationState{{}, t}));
                        }

                        for (auto& key: reactant_keys) {
                            auto interpolated_value = simulation_trajectory::compute_interpolated_value(key, s0, s1, t);

                            auto& table = mean_trajectory.at(t)->reactants;

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
                            s1 = *iterator->second;
                        } else {
                            break;
                        }
                    }
                } else {
                   std::cout << "woops";
                }
            }
        }

        for (double_t i = 0; (i + average_delay) <= upper_bound; i += average_delay) {
            for (auto& key: reactant_keys) {
                mean_trajectory.at(i)->reactants.get(key).amount /= trajectories.size();
            }
        }

        return std::move(mean_trajectory);
    }
}

