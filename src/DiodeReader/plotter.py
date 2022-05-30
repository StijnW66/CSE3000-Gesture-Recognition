import serial
import numpy as np
import matplotlib.pyplot as plt

ser = serial.Serial('/dev/ttyACM0')

print(ser.name)

NUM_PLOTS   = 5
NUM_SIGNALS = 3

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
    fig, axes = plt.subplots(3,2)
    fig.suptitle("PhotoDiode Data")

    plotData(ax[0][0], data[0][0], data[0][1], data[0][2], "Raw"            ,"Time", "Photodiode Reading")
    plotData(ax[0][1], data[1][0], data[1][1], data[1][2], "FFT Filtered"   ,"Time", "Photodiode Reading")
    plotData(ax[1][0], data[2][0], data[2][1], data[2][2], "LPF Filtered"   ,"Time", "Photodiode Reading")
    plotData(ax[2][1], data[3][0], data[3][1], data[3][2], "Normalised"     ,"Time", "Photodiode Reading")
    plotData(ax[2][0], data[4][0], data[4][1], data[4][2], "Stretched"      ,"Time", "Photodiode Reading")

    plt.show()

ser.close()