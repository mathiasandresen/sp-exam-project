//
// Created by Mathias on 11-05-2021.
//

#ifndef SP_EXAM_PROJECT_DATA_H
#define SP_EXAM_PROJECT_DATA_H

namespace StochasticSimulation {
    class simulation_state;
    struct Reaction;
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

        Reaction operator>>=(reactant other);

        Reaction operator>>=(reactant_collection other);

        reactant_collection operator+(const reactant& other);

        bool operator<(reactant other) const;

        reactant operator*(size_t req) {
            required = req;
            return *this;
        }

    };

    class reactant_collection: public std::set<reactant> {
    public:
        using std::set<reactant>::set;
        Reaction operator>>=(reactant other);
        Reaction operator>>=(reactant_collection other);
    };

    class Reaction {
    public:
        std::set<reactant> from;
        std::set<reactant> to;
        std::optional<std::vector<reactant>> catalysts;
        double_t rate{};
        double_t delay{-1};
        std::shared_ptr<simulation_state> lastDelayState;

        Reaction(std::set<reactant> from, std::set<reactant> to):
                from(from),
                to(to)
        {}

        Reaction(std::set<reactant> from, std::set<reactant> to, std::initializer_list<reactant> catalysts, double rate):
                from(from),
                to(to),
                catalysts(catalysts),
                rate(rate)
        {}

        Reaction(std::set<reactant> from, std::set<reactant> to, double rate):
                from(from),
                to(to),
                catalysts{},
                rate(rate)
        {}

        void compute_delay(simulation_state& state, std::default_random_engine& engine);
        void compute_delay2(simulation_state& state, std::default_random_engine& engine);

        friend std::ostream &operator<<(std::ostream &s, const Reaction &reaction);
    };

    struct simulation_state {
    public:
        symbol_table<reactant> reactants;
        double_t time;

        simulation_state(symbol_table<reactant> reactants, double_t time):
            reactants{reactants},
            time{time}
        {};

        simulation_state(const simulation_state&) = default;
        simulation_state(simulation_state&&) = default;

        simulation_state& operator=(const simulation_state &) = default;
        simulation_state& operator=(simulation_state&&) = default;

//        simulation_state(const simulation_state&) = default;
//        simulation_state& operator=(const simulation_state&) = default;
//
        ~simulation_state() = default;

        friend std::ostream &operator<<(std::ostream &, const simulation_state &);
    };

}

#endif //SP_EXAM_PROJECT_DATA_H
