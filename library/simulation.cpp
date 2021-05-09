//
// Created by Mathias on 09-05-2021.
//

#include "simulation.h"

namespace StochasticSimulation {
    basic_reaction reactant::operator>>=(StochasticSimulation::reactant other) {
        std::vector<reactant> from{*this};
        std::vector<reactant> to{std::move(other)};

        return basic_reaction{from, to};
    }

}

