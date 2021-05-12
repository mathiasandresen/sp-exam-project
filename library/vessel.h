//
// Created by Mathias on 12-05-2021.
//

#ifndef SP_EXAM_PROJECT_VESSEL_H
#define SP_EXAM_PROJECT_VESSEL_H

#include <vector>
#include "reaction.h"
#include "SymbolTable.h"

namespace Simulation {

    class Vessel {
    private:
        std::vector<Reaction> reactions;
        StochasticSimulation::SymbolTable<size_t> element_amounts;
    public:
        Reactant operator()(std::string name, size_t initial_amount) {
            element_amounts.put(name, initial_amount);

            return {std::move(name), 1};
        }

        Reaction operator()(Reaction&& reaction, double_t rate) {
            reaction.rate = rate;

            return reaction;
        }

        Reaction operator()(Reaction&& reaction, Reactant catalyst, double_t rate) {
            reaction.rate = rate;
            reaction.catalyst = {{catalyst.name, 1}};

            return reaction;
        }

        void visualize_reactions();
//        void do_simulation();
    };
}

#endif //SP_EXAM_PROJECT_VESSEL_H
