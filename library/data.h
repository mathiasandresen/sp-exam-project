//
// Created by Mathias on 11-05-2021.
//

#ifndef SP_EXAM_PROJECT_DATA_H
#define SP_EXAM_PROJECT_DATA_H

namespace StochasticSimulation {
    class simulation_state;
    struct basic_reaction;
    class reactant_collection;

    struct reactant {
        std::string name;
        double_t amount; //TODO: should properly be fixed
        size_t required{1}; //TODO: This should properly be changed

        reactant(std::string name, size_t initial_amount):
                name(std::move(name)),
                amount(initial_amount)
        {}

        reactant(std::string name, double_t initial_amount):
                name(std::move(name)),
                amount(initial_amount)
        {}

        reactant(std::string name, size_t initial_amount, size_t required):
                name(std::move(name)),
                amount(initial_amount),
                required(required)
        {}

        ~reactant() = default;

        basic_reaction operator>>=(reactant other);

        basic_reaction operator>>=(reactant_collection other);

        reactant_collection operator+(reactant other);

        reactant operator*(size_t req) {
            required = req;
            return *this;
        }

    };

    class reactant_collection: public std::vector<reactant> {
    public:
        using std::vector<reactant>::vector;
        basic_reaction operator>>=(reactant other);
        basic_reaction operator>>=(reactant_collection other);
    };

    struct basic_reaction {
        reactant_collection from;
        reactant_collection to;
    };

    class reaction {
    public:
        basic_reaction basicReaction;
        std::optional<std::vector<reactant>> catalysts;
        double_t rate{};
        double_t delay{-1};

        reaction(basic_reaction basicReaction, std::initializer_list<reactant> catalysts, double rate):
                basicReaction(std::move(basicReaction)),
                catalysts(catalysts),
                rate(rate)
        {}

        reaction(basic_reaction basicReaction, double rate):
                basicReaction(std::move(basicReaction)),
                catalysts{},
                rate(rate)
        {}

        void compute_delay(simulation_state& state, std::default_random_engine& engine);

        friend std::ostream &operator<<(std::ostream &s, const reaction &reaction);
    };

    struct simulation_state {
    public:
        symbol_table<reactant> reactants;
        double_t time;

        simulation_state(symbol_table<reactant> reactants, double_t time):
            reactants{reactants},
            time{time}
        {};

//        simulation_state(const simulation_state&) = default;
////        simulation_state& operator=(const simulation_state&) = default;
//
//        ~simulation_state() = default;
    };

}

#endif //SP_EXAM_PROJECT_DATA_H
