import serial
import numpy as np
import matplotlib.pyplot as plt

ser = serial.Serial('/dev/ttyACM0')

print(ser.name)

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

def readData():
    data1 = []
    data2 = []
    data3 = []
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
            data2.append(d[1])
            data3.append(d[2])
    
    return (data1, data2, data3)

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

def plotData1(ax, data1, title, xlabel, ylabel):
    l = len(data1)
    xs = list(np.arange(0, l))

    ax.plot(xs, data1, c="r")
    ax.legend()
    ax.set_title(title)
    ax.set_xlabel(xlabel)
    ax.set_ylabel(ylabel)


while True:
    
    initData1 = readData1()
    streData1 = readData1()
    
    # plot
    fig, ((ax1, ax2)) = plt.subplots(1,2)
    fig.suptitle("PhotoDiode Data")

    plotData1(ax1, initData1, "Raw Data","Time", "Photodiode Reading")
    # plotData(ax2, smothData1, smothData2, smothData3, "Filtered Data")
    # plotData(ax3, normData1, normData2, normData3, "Normalised Data")
    plotData1(ax2, streData1, "Receiver Output Data", "Time", "Normalised Photodiode Reading")

    plt.show()

ser.close()