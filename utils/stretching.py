import matplotlib.pyplot as plt
import numpy as np

signal = np.array([0, 1, 5, 3, 2, 2, 1, 3, 2, 3, 4, 7, 8])
stretch = np.zeros(40)

i = 0
coeff = 12 / 39

sLength = 13
dLength = 40

while i < 40:
    index = i if i == 0 else (sLength - 1) if (i == dLength - 1) else (i * coeff)
    low = int(np.floor(index))
    high = int(np.ceil(index))

    stretch[i] = (index - low) * signal[high] + (low + 1 - index) * signal[low]

    i = i + 1

t1 = np.arange(0, 13)
t2 = np.arange(0, 40)

fig, (ax1, ax2) = plt.subplots(1,2)
ax1.tick_params(axis='both', labelsize=16)
ax2.tick_params(axis='both', labelsize=16)

ax1.set_title("Data before stretching - 12 samples", fontsize=16)
ax2.set_title("Data after stretching - 40 samples", fontsize=16)
ax1.plot(t1, signal)
ax2.plot(t2, stretch)

plt.show()