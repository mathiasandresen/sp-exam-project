#include <iostream>
#include "library/simulation.h"
#include "library/SymbolTable.h"
#include <fstream>
#include <chrono>
#include "vessels.h"

//#include "include/matplotlibcpp.h"

//namespace plt = matplotlibcpp;

using namespace StochasticSimulation;

class hospitalized_monitor: public simulation_monitor {
private:
    double_t hospitalized_acc{0.0};
    double_t last_time{0.0};
public:
    size_t max_hospitalized{0};

    void monitor(SimulationState &state) override {
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
    Vessel covid_vessel = seihr(10000);

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
    Vessel introduction_vessel = introduction(25, 25, 1, 0.001);

    std::cout << introduction_vessel << std::endl;

    introduction_vessel.visualize_reactions();

    auto trajectories = introduction_vessel.do_multiple_simulations(80, 50);

//    auto trajectory = introduction_vessel.do_simulation(80);

//    auto trajectory = simulation_trajectory::compute_mean_trajectory(trajectories);

//    trajectory.write_csv("intro_output.csv");
}


void simulate_circadian() {
    std::cout << "Simulating circadian rhythm example..." << std::endl;
    Vessel oscillator = circadian_oscillator2();

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


