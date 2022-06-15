import serial
import numpy as np
import matplotlib.pyplot as plt

"""
    This Python script is used in combination with the receiver pipeline code running on an Arduino.
    It expects to be provided a set of signals representing gesture data and how it is processed 
    by the different pipeline stages from raw to final output.

    1. Connects to the serial port
    2. Awaits data
    3. Reads signal data between the messages "Start" and "Done".
    4. Reads a fixed amount of signals before plotting it and displaying it.
    5. Goes back to step 2.
"""

ser = serial.Serial('/dev/ttyACM0')

print(ser.name)

NUM_PLOTS   = 4
NUM_SIGNALS = 3

PLOT_NAMES = [
    "Raw",
    "FFT Filtered",
    "Normalised",
    "Stretched"
]

def readData():
    data = [[] for _ in range(NUM_SIGNALS)]
    while ser.readline() != b"Start\r\n":
        pass

    while True:
        line = ser.readline()
        print(line)
        if (line == b"Done\r\n"):
            break
        else:
            d = list(map(float, line.decode('ascii').strip().split(" ")))

            for i, x in enumerate(d):
                data[i].append(x)
    
    return data

def readData1():
    data1 = []
    while ser.readline() != b"Start\r\n":
        pass

    while True:
        line = ser.readline()
        print(line)
        if (line == b"Done\r\n"):
            break
        else:
            d = list(map(float, line.decode('ascii').strip().split(" ")))
            data1.append(d[0])
    
    return data1

def plotData(ax, data1, data2, data3, title, xlabel, ylabel):
    l = len(data1)
    xs = list(np.arange(0, l))

    ax.plot(xs, data1, c="r", label="right")
    ax.plot(xs, data2, c="b", label="left")
    ax.plot(xs, data3, c="g", label="up")
    ax.legend()
    ax.set_title(title)
    ax.set_xlabel(xlabel)
    ax.set_ylabel(ylabel)

while True:

    # Read the signals
    data = [[[] for _ in range(NUM_SIGNALS)] for _ in range(NUM_PLOTS)]

    for i in range(NUM_PLOTS):
        data[i] = readData()
    
    # plot
    fig, ax = plt.subplots(NUM_PLOTS // 2,2)
    fig.suptitle("PhotoDiode Data")

    for plotIndex in range(NUM_PLOTS):
        plotData(ax[plotIndex // 2][plotIndex % 2], data[plotIndex][0], data[plotIndex][1], data[plotIndex][2], PLOT_NAMES[plotIndex], "Time", "Photodiode Reading")

    plt.show()

ser.close()