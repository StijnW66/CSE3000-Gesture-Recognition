import matplotlib.pyplot as plt


data = []
with open("./post_process/src/data_collection/data/swipe_left/right_hand/candidate_00/iteration_0.txt", "r") as file:
    for line in file:
        temp = list(map(float, line.strip().split(" ")))
        # print(temp)
        data.append(temp)

print("plotting")
print(data)
plt.plot(data)
plt.show()
print("done")