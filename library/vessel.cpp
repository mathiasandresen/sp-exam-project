//
// Created by Mathias on 12-05-2021.
//

#include <sstream>
#include <fstream>
#include "vessel.h"

namespace Simulation {

    void Vessel::visualize_reactions() {
        std::stringstream str;
        StochasticSimulation::symbol_table<std::string> node_map{};

        str << "digraph {" << std::endl;

        auto i = 0;
        for (auto& reactant: element_amounts) {
            if (reactant.first != "__env__") {
                node_map.put(reactant.first, "s" + std::to_string(i));

                str << node_map.get(reactant.first)
                    << "[label=\"" << reactant.first << "\",shape=\"box\",style=\"filled\",fillcolor=\"cyan\"];" << std::endl;
                i++;
            }
        }

        i = 0;
        for (auto& reaction: reactions) {
            std::string reaction_node{"r" + std::to_string(i)};

            str << reaction_node << "[label=\"" << reaction.rate << "\",shape=\"oval\",style=\"filled\",fillcolor=\"yellow\"];" << std::endl;
            for (auto& catalyst: reaction.catalyst) {
                str << node_map.get(catalyst.name) << " -> " << reaction_node << " [arrowhead=\"tee\"];" << std::endl;
            }
            for (auto& reactant: reaction.from) {
                if (reactant.name != "__env__") {
                    str << node_map.get(reactant.name) << " -> " << reaction_node << ";" << std::endl;
                }
            }
            for (auto& product: reaction.to) {
                if (product.name != "__env__") {
                    str << reaction_node << " -> " << node_map.get(product.name) << ";" << std::endl;
                }
            }

            i++;
        }

        str << "}";

        std::ofstream dotfile;
        dotfile.open("graph.dot");
        dotfile << str.str();
        dotfile.close();

        system("dot -Tpng -o graph.png graph.dot");
    }
}
