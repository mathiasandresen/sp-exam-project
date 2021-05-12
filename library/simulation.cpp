//
// Created by Mathias on 09-05-2021.
//

#include <iostream>
#include <utility>
#include "simulation.h"

namespace StochasticSimulation {


    std::ostream &operator<<(std::ostream &s, const vessel_t &vessel) {
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

    double_t simulation_trajectory::compute_interpolated_value(const std::string& key, simulation_state& s0, simulation_state& s1, double_t x) {
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
            simulation_state& s0 = *iterator->second;
            iterator++;
            simulation_state& s1 = *iterator->second;

            double_t t{0};

            while((t + average_delay) <= upper_bound) {
                if (t >= s0.time) {
                    if (t <= s1.time) {
                        if (!mean_trajectory.contains(t)) {
                            mean_trajectory.insert(std::make_shared<simulation_state>(simulation_state{{}, t}));
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

