//
// Created by Mathias on 12-05-2021.
//

#include <memory>
#include <utility>
#include "reaction.h"

namespace Simulation {
    Reaction Reactant::operator>>=(const Reactant& other) const {
        return Reaction({name, 1}, {other.name, 1});
    }

    Reactant Reactant::operator*(size_t amount) {
        this->required = amount;

        return *this;
    }

    ReactantCollection Reactant::operator+(Reactant other) {
        return {*this, std::move(other)};
    }

    Reaction Reactant::operator>>=(ReactantCollection other) const {
        return {{*this}, std::move(other)};
    }

    Reaction ReactantCollection::operator>>=(Reactant other) {
        return {*this, {std::move(other)}};
    }

    Reaction ReactantCollection::operator>>=(ReactantCollection other) {
        return {*this, std::move(other)};
    }
}