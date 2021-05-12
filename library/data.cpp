//
// Created by Mathias on 11-05-2021.
//

#include <iostream>
#include <utility>
#include "simulation.h"
#include "data.h"


namespace StochasticSimulation {
    Reaction reactant::operator>>=(StochasticSimulation::reactant other) {
        return Reaction{{*this}, {std::move(other)}};
    }

    Reaction reactant::operator>>=(reactant_collection other) {
        return Reaction{{*this}, std::move(other)};
    }

    reactant_collection reactant::operator+(const reactant& other) {
        return reactant_collection{*this, other};
    }

    bool reactant::operator<(reactant other) const {
        return name < other.name;
    }

    Reaction reactant_collection::operator>>=(reactant other) {
        return Reaction{*this, {std::move(other)}};
    }

    Reaction reactant_collection::operator>>=(reactant_collection other) {
        return Reaction{*this, std::move(other)};
    }

    std::ostream &operator<<(std::ostream &s, const Reaction &reaction) {
        s << "{ ";
        for (const auto& reactant: reaction.from) {
            s << reactant.name << "+";
        }
        s << "\b" << " >>= ";
        if (reaction.catalysts.has_value()) {
            s << "(";
            for (const auto& catalyst: reaction.catalysts.value()) {
                s << catalyst.name << "+";
            }
            s << "\b" << ") ";
        }
        for (const auto& reactant: reaction.to) {
            s << reactant.name << "+";
        }
        s << "\b";

        return s << " - " << reaction.rate << " }";
    }

    void Reaction::compute_delay2(simulation_state& state, std::default_random_engine &engine) {
        size_t reactant_amount{1};
        size_t catalyst_amount{1};

        for (const reactant& reactant: from) {
            auto amount = reactant.name == "__env__" ? 1 : state.reactants.get(reactant.name).amount;
            reactant_amount *= amount;
        }
        // New: check if amount is 0 already
        if (reactant_amount == 0) {
            delay = -1;
            return;
        }

        if (catalysts.has_value()) {
            for (auto& catalyst: catalysts.value()) {
                catalyst_amount *= state.reactants.get(catalyst.name).amount;
            }
        }

        double_t rate_k = rate * reactant_amount * catalyst_amount;

        if (rate_k > 0) {
            delay = std::exponential_distribution<double_t>(rate_k)(engine);
        } else {
            delay = -1;
        }
    }

    void Reaction::compute_delay(simulation_state& state, std::default_random_engine &engine) {
        lastDelayState = std::make_shared<simulation_state>(state);

        size_t reactant_amount{1};
        size_t catalyst_amount{1};

        for (const reactant& reactant: from) {
            auto amount = reactant.name == "__env__" ? 1 : state.reactants.get(reactant.name).amount;
            reactant_amount *= amount;
        }
        if (catalysts.has_value()) {
            for (auto& catalyst: catalysts.value()) {
                catalyst_amount *= state.reactants.get(catalyst.name).amount;
            }
        }

        double_t rate_k = rate * reactant_amount * catalyst_amount;

        if (rate_k > 0) {
            delay = std::exponential_distribution<double_t>(rate_k)(engine);
        } else {
            delay = -1;
        }
    }

    std::ostream &operator<<(std::ostream &s, const simulation_state& state) {
        s << "{" << std::endl
            << "time: " << state.time << "," << std::endl
            << "reactants: {" << std::endl;
        for(auto& pair: state.reactants) {
            s << pair.first << ": " << pair.second.amount << "," << std::endl;
        }

        s << "}";
        return s;
    }
}