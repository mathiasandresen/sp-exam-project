#!/usr/bin/python
import matplotlib.pyplot as plt
import pandas as pd
import sys

MODE = "debug"
CUSTOM_FILE_NAME = None

def covid_graph():
    CSV_FILE_PATH = f"cmake-build-{MODE}/{CUSTOM_FILE_NAME if CUSTOM_FILE_NAME else 'covid_output.csv'}"

    data = pd.read_csv(CSV_FILE_PATH)

    plt.plot(data["time"].values, data["S"].values, label="S")
    plt.plot(data["time"].values, data["E"].values, label="E")
    plt.plot(data["time"].values, data["I"].values, label="I")
    plt.plot(data["time"].values, data["H"].values*1000, label="H*1000")
    plt.plot(data["time"].values, data["R"].values, label="R")
    plt.title("Covid19 graph with N=10000")
    plt.xlabel("time, days")
    plt.ylabel("population count")
    plt.legend()
    plt.show()


def intro_graph():
    CSV_FILE_PATH = f"cmake-build-{MODE}/{CUSTOM_FILE_NAME if CUSTOM_FILE_NAME else 'intro_output.csv'}"

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
    CSV_FILE_PATH = f"cmake-build-{MODE}/{CUSTOM_FILE_NAME if CUSTOM_FILE_NAME else 'circadian_output.csv'}"

    data = pd.read_csv(CSV_FILE_PATH)

    plt.plot(data["time"].values, data["C"].values, label="C", color="red")
    plt.plot(data["time"].values, data["A"].values, label="A", color="green")
    plt.plot(data["time"].values, data["R"].values, label="R", color="blue")
    plt.title("Circadian rhythm example")
    plt.xlabel("time, hours")
    plt.ylabel("count")
    plt.legend()
    plt.show()


if __name__ == "__main__":
    if len(sys.argv) > 1:
        if len(sys.argv) > 2:
            MODE = sys.argv[2]

            if (len(sys.argv) > 3):
                CUSTOM_FILE_NAME = sys.argv[3]

            
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