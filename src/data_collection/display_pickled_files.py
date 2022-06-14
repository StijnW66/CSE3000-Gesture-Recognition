import matplotlib.pyplot as plt
import pickle

raw = "src"
proc = "new_reformatted/new_post_process/src"
gest = "clockwise"
hand = "left_hand"
cand = 10

rawData = []
procData = []
controlData = []

with open("./{}/data_collection/data/control/candidate_{}.pickle".format(raw, cand), "rb") as file:
    t = pickle.load(file)
    controlData = t
    print(t)

with open("./{}/data_collection/data/{}/{}/candidate_{}.pickle".format(raw, gest, hand, cand), "rb") as file:
    t = pickle.load(file)
    t = pickle.load(file)
    t = pickle.load(file)
    rawData = t
    print(t)

with open("./{}/data_collection/data/{}/{}/candidate_{}.pickle".format(proc, gest, hand, cand), "rb") as file:
    t = pickle.load(file)
    t = pickle.load(file)
    t = pickle.load(file)
    procData = t
    print(t)

print("plotting")
fig, (ax1, ax2, ax3) = plt.subplots(1, 3)
ax1.plot(rawData)
ax2.plot(controlData)
ax3.plot(procData)
plt.show()
print("done")

with open("./input.txt", "w") as outInput:
    outInput.write("{}\n".format(len(rawData)))
    outInput.write("{} {} {}\n".format(int(controlData[:, 0].mean()), int(controlData[:, 1].mean()), int(controlData[:, 2].mean())))

    for i in range(len(rawData)):
        outInput.write("{} {} {}\n".format(rawData[i, 0], rawData[i, 1], rawData[i, 2]))