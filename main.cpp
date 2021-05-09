#include <iostream>
#include "library/simulation.h"

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
    const auto P_H = 0.9e-3; // probability of hospitalization
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

int main() {
    vessel_t covid_vessel = seihr(1e4);

    std::cout << "done" << std::endl;

}


