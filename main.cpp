#include <iostream>
#include "library/simulation.h"
#include "library/symbol_table.h"
#include <fstream>
#include "library/reaction.h"
#include "library/vessel.h"
#include <chrono>

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

class hospitalized_monitor: public simulation_monitor {
private:
    double_t hospitalized_acc{0.0};
    double_t last_time{0.0};
public:
    size_t max_hospitalized{0};

    void monitor(simulation_state &state) override {
        auto currently_hospitalized = state.reactants.get("H").amount;

        if (currently_hospitalized > max_hospitalized) {
            max_hospitalized = currently_hospitalized;
        }

        hospitalized_acc += (currently_hospitalized * (state.time - last_time));

        last_time = state.time;
    }

    double_t get_mean_hospitalized() const {
        return (hospitalized_acc / last_time);
    }
};

void simulate_covid() {
    std::cout << "Simulating covid19 example" << std::endl;
    vessel_t covid_vessel = seihr(10000);

    std::cout << covid_vessel << std::endl;

    covid_vessel.visualize_reactions();

    hospitalized_monitor monitor{};

//    auto trajectory = covid_vessel.do_simulation(400, monitor);
//    trajectory->write_csv("covid_output.csv");

//    std::cout << "Max hospitalized: " << monitor.max_hospitalized << std::endl;
//    std::cout << "Mean hospitalized: " << monitor.get_mean_hospitalized() << std::endl;
//


    auto trajectories = covid_vessel.do_multiple_simulations(400, 3);

//    for (auto& t: trajectories) {
//        t.write_csv("covid_output.csv");
//    }

    std::cout << "simulations done" << std::endl << "computing mean" << std::endl;

//    auto mean = simulation_trajectory::compute_mean_trajectory(trajectories);
//    mean.write_csv("covid_output.csv");
}

void simulate_introduction() {
    std::cout << "Simulating introduction example" << std::endl;
    vessel_t introduction_vessel = introduction(25, 25, 1, 0.001);

    std::cout << introduction_vessel << std::endl;

    introduction_vessel.visualize_reactions();

    auto trajectories = introduction_vessel.do_multiple_simulations(80, 50);

//    auto trajectory = introduction_vessel.do_simulation(80);

//    auto trajectory = simulation_trajectory::compute_mean_trajectory(trajectories);

//    trajectory.write_csv("intro_output.csv");
}


void simulate_circadian() {
    std::cout << "Simulating circadian rhythm example..." << std::endl;
    vessel_t oscillator = circadian_oscillator2();

    std::cout << oscillator << std::endl;

    oscillator.visualize_reactions();

//    auto trajectories = oscillator.do_multiple_simulations(110, 5);
//    std::cout << "Done simulating" << std::endl;
//    std::cout << "Calculating mean trajectory of " << trajectories.size() << " trajectories" << std::endl;
//    auto trajectory = simulation_trajectory::compute_mean_trajectory(trajectories);


    auto t0 = std::chrono::high_resolution_clock::now();
    auto trajectory = oscillator.do_simulation(110);
    auto t1 = std::chrono::high_resolution_clock::now();
    auto trajectory2 = oscillator.do_simulation2(110);
    auto t2 = std::chrono::high_resolution_clock::now();

    std::cout << "Simulation 1 took: " << std::chrono::duration_cast<std::chrono::nanoseconds>(t1-t0).count() << std::endl;
    std::cout << "Simulation 2 took: " << std::chrono::duration_cast<std::chrono::nanoseconds>(t2-t1).count() << std::endl;


    std::cout << "Writing csv file..." << std::endl;
    trajectory->write_csv("circadian_output.csv");
    trajectory2->write_csv("circadian_output2.csv");

    std::cout << "Done!" << std::endl;

}

int main() {
//    simulate_covid();

//    simulate_introduction();

    simulate_circadian();
}


