#!/usr/bin/python
import matplotlib.pyplot as plt
import pandas as pd
import sys

MODE = "debug"

def covid_graph():
    CSV_FILE_PATH = f"cmake-build-{MODE}/covid_output.csv"

    data = pd.read_csv(CSV_FILE_PATH)

    plt.plot(data["time"].values, data["S"].values, label="S")
    plt.plot(data["time"].values, data["E"].values, label="E")
    plt.plot(data["time"].values, data["I"].values, label="I")
    plt.plot(data["time"].values, data["H"].values*1000, label="H")
    plt.plot(data["time"].values, data["R"].values, label="R")
    plt.title("test")
    plt.xlabel("time")
    plt.legend()
    plt.show()


def intro_graph():
    CSV_FILE_PATH = f"cmake-build-{MODE}/intro_output.csv"

    data = pd.read_csv(CSV_FILE_PATH)

    plt.plot(data["time"].values, data["A"].values, label="A", color="red")
    plt.plot(data["time"].values, data["B"].values, label="B", color="green")
    plt.plot(data["time"].values, data["C"].values, label="C", color="blue")
    plt.title("Introduction example")
    plt.xlabel("time")
    plt.ylabel("count")
    plt.legend()
    plt.show()


def cir_graph():
    CSV_FILE_PATH = f"cmake-build-{MODE}/circadian_output.csv"

    data = pd.read_csv(CSV_FILE_PATH)

    plt.plot(data["time"].values, data["C"].values, label="C", color="red")
    plt.plot(data["time"].values, data["A"].values, label="A", color="green")
    plt.plot(data["time"].values, data["R"].values, label="R", color="blue")
    plt.title("Circadian rhythm example")
    plt.xlabel("time, hours")
    plt.ylabel("count")
    plt.legend()
    plt.show()


def covid_delay_graph():
    CSV_FILE_PATH = f"cmake-build-{MODE}/covid_delay_output.csv"

    data = pd.read_csv(CSV_FILE_PATH)
    
    plt.plot(data["time"].values, data["r0"].values, label="r0", color="red")
    plt.plot(data["time"].values, data["r1"].values, label="r1", color="green")
    plt.plot(data["time"].values, data["r2"].values, label="r2", color="blue")
    plt.plot(data["time"].values, data["r3"].values, label="r3", color="purple")
    plt.plot(data["time"].values, data["r4"].values, label="r4", color="orange")
    plt.title("Covid delay")
    # plt.xlabel("time, hours")
    # plt.ylabel("count")
    plt.legend()
    plt.show()


if __name__ == "__main__":
    if len(sys.argv) > 1:
        if len(sys.argv) > 2 and sys.argv[2] == "release":
            MODE = "release"
            
        arg = sys.argv[1]

        if arg == "covid":
            covid_graph()
        elif arg == "covid-delay":
            covid_delay_graph()
        elif arg == "intro":
            intro_graph()
        elif arg == "cir":
            cir_graph()
        else:
            exit("wrong input")
    else:
        intro_graph()