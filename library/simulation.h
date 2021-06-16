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

namespace StochasticSimulation {

    using map_type = std::map<double_t, SimulationState>;
    class SimulationTrajectory: public map_type {
    private:
        double_t largest_time{-1};
        static double_t compute_interpolated_value(
                const std::string& key,
                SimulationState& s0,
                SimulationState& s1,
                double_t x);
    public:
        using map_type::map;

        SimulationTrajectory(const SimulationTrajectory& val): map_type(val) {
            largest_time = val.largest_time;
        }

        SimulationTrajectory(SimulationTrajectory&& rval): map_type(std::move(rval)) {
            largest_time = std::move(rval.largest_time);
        };

        SimulationTrajectory& operator=(const SimulationTrajectory & val) {
            map_type::operator=(val);
            largest_time = val.largest_time;
        };

        SimulationTrajectory& operator=(SimulationTrajectory&& rval) {
            map_type::operator=(std::move(rval));
            largest_time = std::move(rval.largest_time);
        };

        // Requirement 9 compute mean
        static SimulationTrajectory compute_mean_trajectory(std::vector<std::shared_ptr<SimulationTrajectory>>& trajectories);

        void insert(SimulationState state) {
            if (state.time > largest_time) {
                largest_time = state.time;
            }

            map_type::insert({state.time, std::move(state)});
        }

        // Requirement 6 output trajectory
        void write_csv(const std::string& path);

        double_t get_max_time() {
            return largest_time;
        }
    };

    // Requirement 1 operators for DSEL
    class Vessel {
    private:
        std::vector<Reaction> reactions{};
        SymbolTable<Reactant> reactants;
    public:

        Vessel() = default;

        Vessel(const Vessel &val) {
            reactions = val.reactions;
            reactants = val.reactants;
        }

        Vessel (Vessel&& rval) {
            reactions = std::move(rval.reactions);
            reactants = std::move(rval.reactants);
        };

        Reactant& operator()(std::string name, size_t initial_amount) {
            Reactant newReactant{std::move(name), initial_amount};

            reactants.put(newReactant.name, newReactant);

            return reactants.get(newReactant.name);
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


        Reactant& environment() {
            if (reactants.contains("__env__")) {
               return reactants.get("__env__");
            }

            auto newReactant = Reactant("__env__", 0, 0);

            reactants.put(newReactant.name, newReactant);

            return reactants.get(newReactant.name);
        }

        // Requirement 2 network graph
        void visualize_reactions(const std::string& filename);

        // Requirement 10 optimized algorithm
        std::shared_ptr<SimulationTrajectory> do_simulation2(double_t end_time, simulation_monitor& monitor = EMPTY_SIMULATION_MONITOR);

        // Requirement 4 simulation
        std::shared_ptr<SimulationTrajectory> do_simulation(double_t end_time, simulation_monitor& monitor = EMPTY_SIMULATION_MONITOR);

        // Requirement 8 parallelization
        std::vector<std::shared_ptr<SimulationTrajectory>> do_multiple_simulations(double_t end_time, size_t simulations_to_run);

        // Requirement 2 pretty print
        friend std::ostream& operator<<(std::ostream& s, const Vessel& vessel);
    };



}

#endif //SP_EXAM_PROJECT_SIMULATION_H
