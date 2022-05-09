import serial
import numpy as np
import matplotlib.pyplot as plt

ser = serial.Serial('/dev/ttyACM0')

print(ser.name)

while True:

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
    
    # plot
    l = len(data1)
    xs = list(np.arange(0, l))
    ys = data1

    plt.plot(xs, data1, data2)
    plt.show()

ser.close()