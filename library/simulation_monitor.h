//
// Created by Mathias on 11-05-2021.
//

#ifndef SP_EXAM_PROJECT_SIMULATION_MONITOR_H
#define SP_EXAM_PROJECT_SIMULATION_MONITOR_H

#include <functional>
#include "data.h"

namespace StochasticSimulation {
    class simulation_monitor {
    public:
        virtual void monitor(simulation_state& state) = 0;
    };


    class empty_simulation_monitor: public simulation_monitor {
        void monitor(simulation_state &state) override {
          return;
        };
    };

    static auto EMPTY_SIMULATION_MONITOR = empty_simulation_monitor{};

    class basic_simulation_monitor: public simulation_monitor {
    private:
        const std::function<void(simulation_state&)> monitor_function;
    public:
        basic_simulation_monitor(const std::function<void(simulation_state&)>& monitor_func):
            simulation_monitor{},
            monitor_function{monitor_func}
        {}

        void monitor(simulation_state& state) override {
            monitor_function(state);
        }
    };
}

#endif //SP_EXAM_PROJECT_SIMULATION_MONITOR_H
