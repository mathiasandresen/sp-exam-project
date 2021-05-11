//
// Created by Mathias on 09-05-2021.
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


}

