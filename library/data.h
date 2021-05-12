//
// Created by Mathias on 11-05-2021.
//

#ifndef SP_EXAM_PROJECT_DATA_H
#define SP_EXAM_PROJECT_DATA_H

namespace StochasticSimulation {
    class SimulationState;
    struct Reaction;
    class ReactantCollection;

    struct Reactant {
        std::string name;
        double_t amount; //TODO: should properly be fixed
        size_t required{1}; //TODO: This should properly be changed

        Reactant(std::string name, size_t initial_amount):
                name(std::move(name)),
                amount(initial_amount)
        {}

        Reactant(std::string name, double_t initial_amount):
                name(std::move(name)),
                amount(initial_amount)
        {}

        Reactant(std::string name, size_t initial_amount, size_t required):
                name(std::move(name)),
                amount(initial_amount),
                required(required)
        {}

        ~Reactant() = default;

        Reaction operator>>=(Reactant other);

        Reaction operator>>=(ReactantCollection other);

        ReactantCollection operator+(const Reactant& other);

        bool operator<(Reactant other) const;

        Reactant operator*(size_t req) {
            required = req;
            return *this;
        }

    };

    class ReactantCollection: public std::set<Reactant> {
    public:
        using std::set<Reactant>::set;
        Reaction operator>>=(Reactant other);
        Reaction operator>>=(ReactantCollection other);
    };

    class Reaction {
    public:
        std::set<Reactant> from;
        std::set<Reactant> to;
        std::optional<std::vector<Reactant>> catalysts;
        double_t rate{};
        double_t delay{-1};
        std::shared_ptr<SimulationState> lastDelayState;

        Reaction(std::set<Reactant> from, std::set<Reactant> to):
                from(from),
                to(to)
        {}

        Reaction(std::set<Reactant> from, std::set<Reactant> to, std::initializer_list<Reactant> catalysts, double rate):
                from(from),
                to(to),
                catalysts(catalysts),
                rate(rate)
        {}

        Reaction(std::set<Reactant> from, std::set<Reactant> to, double rate):
                from(from),
                to(to),
                catalysts{},
                rate(rate)
        {}

        void compute_delay(SimulationState& state, std::default_random_engine& engine);
        void compute_delay2(SimulationState& state, std::default_random_engine& engine);

        friend std::ostream &operator<<(std::ostream &s, const Reaction &reaction);
    };

    struct SimulationState {
    public:
        SymbolTable<Reactant> reactants;
        double_t time;

        SimulationState(SymbolTable<Reactant> reactants, double_t time):
            reactants{reactants},
            time{time}
        {};

        SimulationState(const SimulationState&) = default;
        SimulationState(SimulationState&&) = default;

        SimulationState& operator=(const SimulationState &) = default;
        SimulationState& operator=(SimulationState&&) = default;

//        SimulationState(const SimulationState&) = default;
//        SimulationState& operator=(const SimulationState&) = default;
//
        ~SimulationState() = default;

        friend std::ostream &operator<<(std::ostream &, const SimulationState &);
    };

}

#endif //SP_EXAM_PROJECT_DATA_H
