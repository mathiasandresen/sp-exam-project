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

    double_t simulation_trajectory::compute_interpolated_value(const std::string& key, std::shared_ptr<simulation_state> s0, std::shared_ptr<simulation_state> s1, double_t x) {
        return
            s0->reactants.get(key).amount
            + ((
                   ( (double_t) s1->reactants.get(key).amount - (double_t) s0->reactants.get(key).amount) /
                   ( s1->time - s0->time )
               ) * (x - s0->time));
    }

    simulation_trajectory simulation_trajectory::compute_mean_trajectory(std::vector<simulation_trajectory> trajectories) {
        auto first = *trajectories.begin();
        auto average_delay = first.get_max_time() / first.size();
        double_t upper_bound{-1.0};

        std::vector<std::string> reactant_keys{};
        for (auto& reactant: first.begin()->second.reactants) {
            reactant_keys.push_back(reactant.second.name);
        }

        std::vector<simulation_trajectory> interpolated_trajectories{};

        for (auto& trajectory: trajectories) {
            if (upper_bound == -1.0 || trajectory.get_max_time() < upper_bound) {
                upper_bound = trajectory.get_max_time();
            }

            simulation_trajectory interpolated{};

            auto iterator = trajectory.begin();
            auto s0 = std::make_shared<simulation_state>(iterator->second);
            auto s1 = std::make_shared<simulation_state>((++iterator)->second);

            double_t t{0};

            while(t < trajectory.get_max_time()) {
                if (t >= s0->time) {
                    if (t <= s1->time) {
                        symbol_table<reactant> table{};
                        for (auto& key: reactant_keys) {
                            auto interpolated_value = simulation_trajectory::compute_interpolated_value(key, s0, s1, t);
                            table.put(key, reactant{key, interpolated_value});
                        }
                        interpolated.insert(simulation_state{table, t});
                        t += average_delay;
                    } else {
                        s0.swap(s1);
                        s1 = std::make_shared<simulation_state>((++iterator)->second);
//                        s1 = std::shared_ptr<simulation_state>{};
                    }
                } else {
                    std::cout << "woops";
                }
            }

            interpolated.write_csv("covid_output.csv");

            interpolated_trajectories.push_back(std::move(interpolated));
        }

        simulation_trajectory mean_trajectory{};

        auto first_interpolated = *interpolated_trajectories.begin();

        for (double_t i = 0; (i + average_delay) <= upper_bound; i += average_delay) {
            symbol_table<reactant> table{};

            for (auto& trajectory: interpolated_trajectories) {
                for (auto& key: reactant_keys) {
                    if (!table.contains(key)) {
                        reactant reactant{key, 0.0};
                        table.put(key, reactant);
                    }

                    table.get(key).amount += trajectory.at(i).reactants.get(key).amount;
                }
            }

            for (auto& key: reactant_keys) {
                table.get(key).amount /= interpolated_trajectories.size();
            }

            mean_trajectory.insert(simulation_state{table, i});
        }

        return mean_trajectory;
    }
}

