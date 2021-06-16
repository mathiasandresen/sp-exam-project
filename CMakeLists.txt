cmake_minimum_required(VERSION 3.17)
project(sp_exam_project)

set(CMAKE_CXX_STANDARD 20)

add_library(
    stochastic-simulation
    library/simulation.cpp
    library/simulation.cpp
    library/SymbolTable.h
    library/simulation_monitor.h
    library/data.h
    library/data.cpp
)

add_executable(sp_exam_project main.cpp vessels.h)

target_link_libraries(sp_exam_project PRIVATE stochastic-simulation)

