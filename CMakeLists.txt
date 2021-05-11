cmake_minimum_required(VERSION 3.17)
project(sp_exam_project)

set(CMAKE_CXX_STANDARD 20)

#link_libraries("C:\\Program Files\\Graphviz\\include\\graphviz")

#target_link_libraries(graphviz-lib gvc.h PATHS C:\\Program Files\\Graphviz\\include\\graphviz)

#INCLUDE_DIRECTORIES(SYSTEM "C:/Program Files/Graphviz/lib/")

add_library(
    stochastic-simulation
    library/simulation.cpp
    library/simulation.cpp
    library/symbol_table.h
    library/simulation_monitor.h
        library/data.h library/data.cpp)

#add_library(
#    matplotlib_cpp
#    include/matplotlibcpp.h
#)

find_package(PythonLibs 3.8.7 COMPONENTS NumPy)
#find_package(Python38 COMPONENTS Interpreter Development REQUIRED)
#find_package(Python38 COMPONENTS NumPy)

find_package(PythonLibs 3.8.7 COMPONENTS NumPy)

add_executable(sp_exam_project main.cpp ${PYTHON_INCLUDE_DIRS} include/matplotlibcpp.h)

target_include_directories(sp_exam_project PRIVATE ${PYTHON_INCLUDE_DIRS})

target_link_libraries(sp_exam_project INTERFACE ${PYTHON_LIBRARIES} PRIVATE stochastic-simulation )

#target_link_libraries(sp_exam_project ${PYTHON_LIBRARIES})

