import matplotlib.pyplot as plt
import pickle


data = []
with open("./src/data_collection/data/swipe_left/right_hand/candidate_29.pickle", "rb") as file:
    t = pickle.load(file)
    t = pickle.load(file)
    t = pickle.load(file)
    data = t
    print(t)

print("plotting")
print(data)
plt.plot(data)
plt.show()
print("done")