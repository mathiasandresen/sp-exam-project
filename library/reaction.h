//
// Created by Mathias on 12-05-2021.
//

#ifndef SP_EXAM_PROJECT_REACTION_H
#define SP_EXAM_PROJECT_REACTION_H

#include <string>
#include <vector>

namespace Simulation {

    struct Reaction;
    class ReactantCollection;

    struct Element {
        std::string name;
        Element(std::string name): name{name} {}

    };

    struct Reactant: public Element {
        size_t required;
        Reactant(std::string name, size_t required):
                Element(name),
                required{required}
        {}

        Reaction operator>>=(const Reactant&) const;
        Reaction operator>>=(ReactantCollection) const;
        Reactant operator*(size_t amount);
        ReactantCollection operator+(Reactant other);
    };

    class ReactantCollection: public std::vector<Reactant> {
    public:
        using std::vector<Reactant>::vector;
        Reaction operator>>=(Reactant other);
        Reaction operator>>=(ReactantCollection other);
    };

    struct Reaction {
        std::vector<Reactant> from;
        std::vector<Reactant> to;
        std::vector<Reactant> catalyst;
        double_t rate{-1};

        Reaction(std::vector<Reactant> from, std::vector<Reactant> to): from{from}, to{to} {}
        Reaction(Reactant from, Reactant to) {
            this->from = {from};
            this->to = {to};
        }
    };


}


#endif //SP_EXAM_PROJECT_REACTION_H
