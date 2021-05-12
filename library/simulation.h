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
#include "SymbolTable.h"
#include "simulation_monitor.h"
#include "data.h"
#include "../include/thread-pool.hpp"

namespace StochasticSimulation {

    using map_type = std::map<double_t, std::shared_ptr<SimulationState>>;
    class simulation_trajectory: public map_type {
    private:
        double_t largest_time{-1};
        static double_t compute_interpolated_value(
                const std::string& key,
                SimulationState& s0,
                SimulationState& s1,
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

        void insert(std::shared_ptr<SimulationState> state) {
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

    class Vessel {
    private:
        std::vector<Reaction> reactions{};
        SymbolTable<Reactant> reactants;
    public:
        Reactant operator()(std::string name, size_t initial_amount) {
            Reactant newReactant{std::move(name), initial_amount};

            reactants.put(newReactant.name, newReactant);

            return newReactant;
        }

        Reaction operator()(Reaction&& reaction, double_t rate) {
            reaction.rate = rate;

            reactions.push_back(reaction);

            return reaction;
        }

        Reaction operator()(Reaction&& reaction, std::initializer_list<Reactant> catalysts, double rate) {
            reaction.rate = rate;
            reaction.catalysts = catalysts;

            // Add to vessel reactions
            reactions.push_back(reaction);

            return reaction;
        }

        Reaction operator()(Reaction&& reaction, Reactant catalyst, double_t rate) {
            reaction.rate = rate;
            reaction.catalysts = {catalyst};

            // Add to vessel reactions
            reactions.push_back(reaction);

            return reaction;
        }


        Reactant environment() {
            if (reactants.contains("__env__")) {
               return reactants.get("__env__");
            }

            std::shared_ptr<Reactant> newReactant(new Reactant("__env__", 0, 0));

            reactants.put(newReactant->name, *newReactant);

            return *newReactant;
        }

        void visualize_reactions();

        SymbolTable<Reactant> get_reactants() {
            return reactants;
        }

        std::shared_ptr<simulation_trajectory> do_simulation2(double_t end_time, simulation_monitor& monitor = EMPTY_SIMULATION_MONITOR);

        std::shared_ptr<simulation_trajectory> do_simulation(double_t end_time, simulation_monitor& monitor = EMPTY_SIMULATION_MONITOR);

        std::vector<std::shared_ptr<simulation_trajectory>> do_multiple_simulations(double_t end_time, size_t simulations_to_run);

        friend std::ostream& operator<<(std::ostream& s, const Vessel& vessel);
    };



}

#endif //SP_EXAM_PROJECT_SIMULATION_H
