//
// Created by Mathias on 11-05-2021.
//

#include <iostream>
#include <utility>
#include "simulation.h"

namespace StochasticSimulation {
    basic_reaction reactant::operator>>=(StochasticSimulation::reactant other) {
        return basic_reaction{{*this}, {std::move(other)}};
    }

    basic_reaction reactant::operator>>=(reactant_collection other) {
        return basic_reaction{{*this}, std::move(other)};
    }

    reactant_collection reactant::operator+(reactant other) {
        return reactant_collection{*this, std::move(other)};
    }

    basic_reaction reactant_collection::operator>>=(reactant other) {
        return basic_reaction{*this, {std::move(other)}};
    }

    basic_reaction reactant_collection::operator>>=(reactant_collection other) {
        return basic_reaction{*this, std::move(other)};
    }

    std::ostream &operator<<(std::ostream &s, const reaction &reaction) {
        s << "{ ";
        for (const auto& reactant: reaction.basicReaction.from) {
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
        for (const auto& reactant: reaction.basicReaction.to) {
            s << reactant.name << "+";
        }
        s << "\b";

        return s << " - " << reaction.rate << " }";
    }

    void reaction::compute_delay(simulation_state& state, std::default_random_engine &engine) {
        size_t reactant_amount{1};
        size_t catalyst_amount{1};

        for (reactant& reactant: basicReaction.from) {
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
}