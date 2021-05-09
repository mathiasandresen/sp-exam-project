//
// Created by Mathias on 09-05-2021.
//

#ifndef SP_EXAM_PROJECT_SIMULATION_H
#define SP_EXAM_PROJECT_SIMULATION_H

#include <string>
#include <utility>
#include <vector>
#include <optional>

namespace StochasticSimulation {

    struct basic_reaction;

    struct reactant {
        std::string name;
        size_t amount;

        reactant(std::string name, size_t initial_amount):
            name(std::move(name)),
            amount(initial_amount)
        {}

        basic_reaction operator>>=(reactant other);
    };

    struct basic_reaction {
        std::vector<reactant> from;
        std::vector<reactant> to;
    };

    struct reaction {
        basic_reaction basicReaction;
        std::optional<reactant> catalyst;
        double rate{};

        reaction(basic_reaction basicReaction, reactant catalyst, double rate):
            basicReaction(std::move(basicReaction)),
            catalyst(catalyst),
            rate(rate)
        {}

        reaction(basic_reaction basicReaction, double rate):
                basicReaction(std::move(basicReaction)),
                catalyst{},
                rate(rate)
        {}

    };

    class vessel_t {
    private:
        std::vector<reaction> reactions{};
    public:
        reactant operator()(std::string name, size_t initial_amount) {
            //TODO: Add reactant to list?
            return reactant{std::move(name), initial_amount};
        }

        reaction operator()(basic_reaction basicReaction, reactant catalyst, double rate) {
            reaction newReaction{std::move(basicReaction), std::move(catalyst), rate};

            // Add to vessel reactions
            reactions.push_back(newReaction);

            return newReaction;
        }

        reaction operator()(basic_reaction basicReaction, double rate) {
            reaction newReaction{std::move(basicReaction), rate};

            // Add to vessel reactions
            reactions.push_back(newReaction);

            return newReaction;
        }

    };



}

#endif //SP_EXAM_PROJECT_SIMULATION_H
