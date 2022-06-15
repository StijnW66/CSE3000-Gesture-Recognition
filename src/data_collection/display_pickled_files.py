import matplotlib.pyplot as plt
import pickle

raw = "raw_data"
proc = "reformatted/post_process./raw_data"
gest = "swipe_left"
hand = "left_hand"
cand = 34

rawData = []
procData = []
controlData = []

# Plot control data
# with open("./{}/data_collection/data/control/candidate_{}.pickle".format(raw, cand), "rb") as file:
#     t = pickle.load(file)
#     controlData = t
#     print(t)

with open("./{}/{}/{}/candidate_{}.pickle".format(raw, gest, hand, cand), "rb") as file:
    t = pickle.load(file)
    t = pickle.load(file)
    t = pickle.load(file)
    rawData = t
    print(t)

with open("./{}/{}/{}/candidate_{}.pickle".format(proc, gest, hand, cand), "rb") as file:
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