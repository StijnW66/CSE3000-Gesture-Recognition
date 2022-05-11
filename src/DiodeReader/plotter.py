import serial
import numpy as np
import matplotlib.pyplot as plt

ser = serial.Serial('/dev/ttyACM0')

print(ser.name)

def readData():
    data1 = []
    data2 = []
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
    
    return (data1, data2)

def plotData(ax, data1, data2, title):
    l = len(data1)
    xs = list(np.arange(0, l))

    ax.plot(xs, data1, data2)
    ax.set_title(title)

while True:
    
    initData1, initData2 = readData()
    smothData1, smothData2 = readData()
    normData1, normData2 = readData()
    streData1, streData2 = readData()
    
    # plot
    fig, ((ax1, ax2), (ax3, ax4)) = plt.subplots(2,2)
    fig.suptitle("PhotoDiode Data")

    plotData(ax1, initData1, initData2, "Raw Data")
    plotData(ax2, smothData1, smothData2, "Filtered Data")
    plotData(ax3, normData1, normData2, "Normalised Data")
    plotData(ax4, streData1, streData2, "Time-Stretched Data")

    plt.show()

ser.close()