import matplotlib.pyplot as plt
import pickle
import scipy.ndimage

raw = "src"
proc = "new_reformatted/new_post_process/src"
gest = "clockwise"
hand = "left_hand"
cand = 10

path = "src/data_collection/data/final/Q1_Tap_result_Resitor_1112k/25/tap/right_hand/data.pickle"

data = []
with open(path, "rb") as file:
    t = pickle.load(file)

    #t = pickle.load(file)
    data = t
    print(t)

fst = []

for i in data:
    fst.append(i[0])
#fst = scipy.ndimage.gaussian_filter1d(fst, 1.5) # apply filtering for 50Hz noise
plt.plot(fst)

plt.ylim([0, 800]) if ("original" in path) else print()#plt.ylim([0, 1])
plt.ylabel("Photodiode reading") if ("original" in path) else plt.ylabel("Normalized photodiode reading")

plt.xlabel("Time")

plt.tight_layout()
plt.show()
