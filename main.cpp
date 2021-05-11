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

/** direct encoding */
vessel_t circadian_oscillator()
{
    auto alphaA = 50.0;
    auto alpha_A = 500.0;
    auto alphaR = 0.01;
    auto alpha_R = 50.0;
    auto betaA = 50.0;
    auto betaR = 5.0;
    auto gammaA = 1.0;
    auto gammaR = 1.0;
    auto gammaC = 2.0;
    auto deltaA = 1.0;
    auto deltaR = 0.2;
    auto deltaMA = 10.0;
    auto deltaMR = 0.5;
    auto thetaA = 50.0;
    auto thetaR = 100.0;
    auto v = vessel_t{};
    auto env = v.environment();
    auto DA = v("DA", 1);
    auto D_A = v("D_A", 0);
    auto DR = v("DR", 1);
    auto D_R = v("D_R", 0);
    auto MA = v("MA", 0);
    auto MR = v("MR", 0);
    auto A = v("A", 0);
    auto R = v("R", 0);
    auto C = v("C", 0);
    v(A + DA >>= D_A, gammaA);
    v(D_A >>= DA + A, thetaA);
    v(A + DR >>= D_R, gammaR);
    v(D_R >>= DR + A, thetaR);
    v(D_A >>= MA + D_A, alpha_A);
    v(DA >>= MA + DA, alphaA);
    v(D_R >>= MR + D_R, alpha_R);
    v(DR >>= MR + DR, alphaR);
    v(MA >>= MA + A, betaA);
    v(MR >>= MR + R, betaR);
    v(A + R >>= C, gammaC);
    v(C >>= R, deltaA);
    v(A >>= env, deltaA);
    v(R >>= env, deltaR);
    v(MA >>= env, deltaMA);
    v(MR >>= env, deltaMR);
    return v;
}

/** alternative encoding using catalysts */
vessel_t circadian_oscillator2()
{
    auto alphaA = 50.0;
    auto alpha_A = 500.0;
    auto alphaR = 0.01;
    auto alpha_R = 50.0;
    auto betaA = 50.0;
    auto betaR = 5.0;
    auto gammaA = 1.0;
    auto gammaR = 1.0;
    auto gammaC = 2.0;
    auto deltaA = 1.0;
    auto deltaR = 0.2;
    auto deltaMA = 10.0;
    auto deltaMR = 0.5;
    auto thetaA = 50.0;
    auto thetaR = 100.0;
    auto v = vessel_t{};
    auto env = v.environment();
    auto DA = v("DA", 1);
    auto D_A = v("D_A", 0);
    auto DR = v("DR", 1);
    auto D_R = v("D_R", 0);
    auto MA = v("MA", 0);
    auto MR = v("MR", 0);
    auto A = v("A", 0);
    auto R = v("R", 0);
    auto C = v("C", 0);
    v(A + DA >>= D_A, gammaA);
    v(D_A >>= DA + A, thetaA);
    v(DR + A >>= D_R, gammaR);
    v(D_R >>= DR + A, thetaR);
    v(env >>= MA, D_A, alpha_A);
    v(env >>= MA, DA, alphaA);
    v(env >>= MR, D_R, alpha_R);
    v(env >>= MR, DR, alphaR);
    v(env >>= A, MA, betaA);
    v(env >>= R, MR, betaR);
    v(A + R >>= C, gammaC);
    v(C >>= R, deltaA);
    v(A >>= env, deltaA);
    v(R >>= env, deltaR);
    v(MA >>= env, deltaMA);
    v(MR >>= env, deltaMR);
    return v;
}

void simulate_covid() {
    std::cout << "Simulating covid19 example" << std::endl;
    vessel_t covid_vessel = seihr(10000);

    std::cout << covid_vessel << std::endl;

    covid_vessel.visualize_reactions();

    std::ofstream csv_file;
    csv_file.open("covid_output.csv");

    auto state = covid_vessel.getState();
    for (auto& reactant : state.getMap()) {
        csv_file << reactant.second->name << ",";
    }
    csv_file << "time" << std::endl;

    covid_vessel.do_simulation(400, [&csv_file](double_t time, symbol_table<std::shared_ptr<reactant>> reactants, const std::vector<reaction>& reactions){
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

    std::cout << introduction_vessel << std::endl;

    introduction_vessel.visualize_reactions();

    std::ofstream csv_file;
    csv_file.open("intro_output.csv");

    auto state = introduction_vessel.getState();
    for (auto& reactant : state.getMap()) {
        csv_file << reactant.second->name << ",";
    }
    csv_file << "time" << std::endl;

    introduction_vessel.do_simulation(80, [&csv_file](double_t time, symbol_table<std::shared_ptr<reactant>> reactants, auto reactions){
        for (auto& reactant : reactants.getMap()) {
            csv_file << reactant.second->amount << ",";
        }
        csv_file << time << std::endl;

    });

    csv_file.close();
}


void simulate_circadian() {
    std::cout << "Simulating circadian rhythm example" << std::endl;
    vessel_t oscillator = circadian_oscillator();

    std::cout << oscillator << std::endl;

    oscillator.visualize_reactions();

    std::ofstream csv_file;
    csv_file.open("circadian_output.csv");

    auto state = oscillator.getState();
    for (auto& reactant : state.getMap()) {
        csv_file << reactant.second->name << ",";
    }
    csv_file << "time" << std::endl;

    double_t last_time = -1;

    oscillator.do_simulation(200, [&csv_file, &last_time](double_t time, symbol_table<std::shared_ptr<reactant>> reactants, auto reactions){
        if (time - last_time > 0.25) {
            last_time = time;
            for (auto& reactant : reactants.getMap()) {
                csv_file << reactant.second->amount << ",";
            }
            csv_file << time << std::endl;
        }
    });

    csv_file.close();
}

int main() {
    simulate_covid();

//    simulate_introduction();

//    simulate_circadian();

//    std::exponential_distribution<double_t> rng{1};
//    std::default_random_engine engine;
//
//    auto acc = 0.0;
//
//    for (int i = 0; i < 10000; ++i) {
//        auto num = rng(engine);
//        acc += num;
////        std::cout << num << std::endl;
//    }
//
//    std::cout << "Average: " << (acc / 10000) << std::endl;

}


