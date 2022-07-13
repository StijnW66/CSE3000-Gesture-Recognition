import matplotlib.pyplot as plt
import scipy.ndimage
resistors_sun = [990, 660, 430, 330, 330, 200, 147, 147, 100, 100, 69, 69, 47, 40, 40, 30, 30, 30, 30, 20, 20, 20, 20, 20, 20, 20, 20, 10, 10, 10, 10]
lux_sun = [200, 300, 400, 500, 600, 750, 1000, 1250, 1500, 1750, 2000, 2500, 3000, 3500, 4000, 4500, 5000, 5500, 6000, 6500, 7000, 7500, 8000, 8500, 9000, 9500, 10000, 10500, 11000, 11500, 12000]

resistors_art = [1132, 1032, 680]
lux_art = [250, 300, 400]

for i in range(len(resistors_sun)):
  resistors_sun[i] *= 1000

resistors_sun = scipy.ndimage.gaussian_filter1d(resistors_sun, 0.8)

plt.plot(lux_sun, resistors_sun)
plt.xlabel("Light Intensity (Lux)")
plt.ylabel(r"Resistor ($\Omega$)")
plt.title("Optimal resistor for light intensity")
plt.tight_layout()
plt.show()