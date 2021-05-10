#!/usr/bin/python
import matplotlib.pyplot as plt
import pandas as pd
import sys


def covid_graph():
    CSV_FILE_PATH = "cmake-build-debug/covid_output.csv"

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
    CSV_FILE_PATH = "cmake-build-debug/intro_output.csv"

    data = pd.read_csv(CSV_FILE_PATH)

    plt.plot(data["time"].values, data["A"].values, label="A", color="red")
    plt.plot(data["time"].values, data["B"].values, label="B", color="green")
    plt.plot(data["time"].values, data["C"].values, label="C", color="blue")
    plt.title("Introduction example")
    plt.xlabel("time")
    plt.ylabel("count")
    plt.legend()
    plt.show()


if __name__ == "__main__":
    if len(sys.argv) > 1:
        if sys.argv[1] == "covid":
            covid_graph()
        elif sys.argv[1] == "intro":
            intro_graph()
        else:
            exit("wrong input")
    else:
        intro_graph()