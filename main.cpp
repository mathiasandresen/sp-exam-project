#include <iostream>
#include "library/simulation.h"
#include "library/symbol_table.h"
#include <fstream>
//#include "include/matplotlibcpp.h"

//namespace plt = matplotlibcpp;

using namespace StochasticSimulation;

vessel_t seihr(uint32_t N)
{
    auto v = vessel_t{};
    const auto eps = 0.0009; // initial fraction of infectious
    const auto I0 = size_t(std::round(eps*N)); // initial infectious
    const auto E0 = size_t(std::round(eps*N*15)); // initial exposed
    const auto S0 = N-I0-E0; // initial susceptible
    const auto R0 = 2.4; // basic reproductive number (initial, without lockdown etc)
    const auto alpha = 1.0 / 5.1; // incubation rate (E -> I) ~5.1 days
    const auto gamma = 1.0 / 3.1; // recovery rate (I -> R) ~3.1 days
    const auto beta = R0 * gamma; // infection/generation rate (S+I -> E+I)
    const auto P_H = 0.8; // probability of hospitalization
    const auto kappa = gamma * P_H*(1.0-P_H); // hospitalization rate (I -> H)
    const auto tau = 1.0/10.12; // recovery/death rate in hospital (H -> R) ~10.12 days

    // Reactants
    auto S = v("S", S0); // susceptible
    auto E = v("E", E0); // exposed
    auto I = v("I", I0); // infectious
    auto H = v("H", 0); // hospitalized
    auto R = v("R", 0); // removed/immune (recovered + dead)

    // Reactions
    v(S >>= E, I, beta/N);
    v(E >>= I, alpha);
    v(I >>= R, gamma);
    v(I >>= H, kappa);
    v(H >>= R, tau);

    return v;
}

vessel_t introduction(uint32_t A_start, uint32_t B_Start, uint32_t D_amount, double_t lambda) {
    auto v = vessel_t{};
    // Reactants
    auto A = v("A", A_start);
    auto B = v("B", B_Start);
    auto C = v("C", 0);
    auto D = v("D", D_amount);
    // Reactions
    v(A + B * 2 >>= C, D, lambda);

    return v;
}

void simulate_covid() {
    std::cout << "Simulating covid19 example" << std::endl;
    vessel_t covid_vessel = seihr(10000);

    std::ofstream csv_file;
    csv_file.open("covid_output.csv");

    auto state = covid_vessel.getState();
    for (auto& reactant : state.getMap()) {
        csv_file << reactant.second->name << ",";
    }
    csv_file << "time" << std::endl;

    covid_vessel.do_simulation(400, [&csv_file](symbol_table<std::shared_ptr<reactant>> reactants, double_t time){
        for (auto& reactant : reactants.getMap()) {
            csv_file << reactant.second->amount << ",";
        }
        csv_file << time << std::endl;
    });

    csv_file.close();
}

void simulate_introduction() {
    std::cout << "Simulating introduction example" << std::endl;
    vessel_t introduction_vessel = introduction(25, 25, 1, 0.001);

    std::ofstream csv_file;
    csv_file.open("intro_output.csv");

    auto state = introduction_vessel.getState();
    for (auto& reactant : state.getMap()) {
        csv_file << reactant.second->name << ",";
    }
    csv_file << "time" << std::endl;

    introduction_vessel.do_simulation(400, [&csv_file](symbol_table<std::shared_ptr<reactant>> reactants, double_t time){
        for (auto& reactant : reactants.getMap()) {
            csv_file << reactant.second->amount << ",";
        }
        csv_file << time << std::endl;
    });

    csv_file.close();
}

int main() {
    simulate_covid();
//    simulate_introduction();
}


