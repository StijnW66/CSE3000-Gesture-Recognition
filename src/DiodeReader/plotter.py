import serial
import numpy as np
import matplotlib.pyplot as plt

ser = serial.Serial('/dev/ttyACM0')

print(ser.name)

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

def plotData(ax, data1, data2, data3, title):
    l = len(data1)
    xs = list(np.arange(0, l))

    ax.plot(xs, data1, c="r")
    ax.plot(xs, data2, c="b")
    ax.plot(xs, data3, c="g")
    ax.set_title(title)

while True:
    
    initData1, initData2, initData3 = readData()
    smothData1, smothData2, smothData3 = readData()
    normData1, normData2, normData3 = readData()
    streData1, streData2, streData3 = readData()
    
    # plot
    fig, ((ax1, ax2), (ax3, ax4)) = plt.subplots(2,2)
    fig.suptitle("PhotoDiode Data")

    plotData(ax1, initData1, initData2, initData3, "Raw Data")
    plotData(ax2, smothData1, smothData2, smothData3, "Filtered Data")
    plotData(ax3, normData1, normData2, normData3, "Normalised Data")
    plotData(ax4, streData1, streData2, streData3, "Time-Stretched Data")

    plt.show()

ser.close()