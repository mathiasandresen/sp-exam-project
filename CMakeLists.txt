cmake_minimum_required(VERSION 3.17)
project(sp_exam_project)

set(CMAKE_CXX_STANDARD 20)

add_executable(sp_exam_project main.cpp library/simulation.cpp library/simulation.h)