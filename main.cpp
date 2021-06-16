#include <iostream>
#include "library/simulation.h"
#include <chrono>
#include "vessels.h"

using namespace StochasticSimulation;

// Requirement 7 use of monitor
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
    std::cout << "Simulating covid19 example with hospitalized monitor" << std::endl;
    Vessel covid_vessel = seihr(10000);

    std::cout << covid_vessel << std::endl;
    covid_vessel.visualize_reactions("covid_graph.png");

    std::cout << "reaction graph can be seen at: covid_graph.png" << std::endl;

    hospitalized_monitor monitor{};

    auto trajectory = covid_vessel.do_simulation(120, monitor);

    std::cout << "Simulation done" << std::endl;
    std::cout << "Max hospitalized: " << monitor.max_hospitalized << std::endl;
    std::cout << "Mean hospitalized: " << monitor.get_mean_hospitalized() << std::endl;

    std::cout << "Writing trajectory to csv file at covid_output.csv" << std::endl;
    trajectory->write_csv("covid_output.csv");
    std::cout << "Turn it into a graph using python ./draw_graph.py covid release" << std::endl;
}

void simulate_covid_multiple() {
    std::cout << "Simulating covid19 example 30 times and calculating mean" << std::endl;
    Vessel covid_vessel = seihr(10000);

    auto trajectories = covid_vessel.do_multiple_simulations(110, 100);

    std::cout << "Simulations done" << std::endl << "Computing mean trajectory" << std::endl;

    auto mean = SimulationTrajectory::compute_mean_trajectory(trajectories);

    std::cout << "Writing mean trajectory to csv file at covid_output_multiple.csv" << std::endl;
    mean.write_csv("covid_output_multiple.csv");
    std::cout << "Turn it into a graph using python ./draw_graph.py covid covid_output_multiple.csv" << std::endl;
}

void simulate_introduction() {
    std::cout << "Simulating introduction example" << std::endl;
    Vessel introduction_vessel = introduction(25, 50, 1, 0.001);
    std::cout << introduction_vessel << std::endl;

    introduction_vessel.visualize_reactions("intro_graph.png");

    auto trajectory = introduction_vessel.do_simulation(400);

    trajectory->write_csv("intro_output.csv");
}


void simulate_circadian() {
    std::cout << "Simulating circadian rhythm example..." << std::endl;
    Vessel oscillator = circadian_oscillator();

    std::cout << oscillator << std::endl;
    oscillator.visualize_reactions("cir_graph.png");

    auto trajectory = oscillator.do_simulation(110);

    std::cout << "Writing csv file..." << std::endl;
    trajectory->write_csv("circadian_output.csv");
}

void simulate_circadian2() {
    std::cout << "Simulating circadian rhythm alternative example..." << std::endl;
    Vessel oscillator = circadian_oscillator2();

    std::cout << oscillator << std::endl;
    oscillator.visualize_reactions("cir2_graph.png");

    auto trajectory = oscillator.do_simulation(110);

    std::cout << "Writing csv file..." << std::endl;
    trajectory->write_csv("circadian2_output.csv");
}

void benchmark() {
    std::cout << "Benchmarking with circadian rhythm example (max_time=100)" << std::endl;

    auto runs{30};

    Vessel oscillator = circadian_oscillator();

    unsigned long time_acc1{0};
    for (int i = 0; i < runs; ++i) {
        auto t0 = std::chrono::high_resolution_clock::now();
        oscillator.do_simulation(100);
        auto t1 = std::chrono::high_resolution_clock::now();

        time_acc1 += std::chrono::duration_cast<std::chrono::nanoseconds>(t1-t0).count();
    }
    auto mean_time1 = time_acc1 / runs;
    std::cout << "Simulation 1 mean time (nanoseconds): " << mean_time1 << std::endl;

    unsigned long time_acc2{0};
    for (int i = 0; i < runs; ++i) {
        auto t0 = std::chrono::high_resolution_clock::now();
        oscillator.do_simulation2(100);
        auto t1 = std::chrono::high_resolution_clock::now();

        time_acc2 += std::chrono::duration_cast<std::chrono::nanoseconds>(t1-t0).count();
    }
    auto mean_time2 = time_acc2 / runs;
    std::cout << "Simulation 2 mean time (nanoseconds): " << mean_time2 << std::endl;
}

int main() {
//    simulate_covid();
//    simulate_covid_multiple();

//    simulate_introduction();
    simulate_circadian();
//    simulate_circadian2();

//    benchmark();
}


