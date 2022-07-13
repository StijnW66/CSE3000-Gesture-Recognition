import numpy as np
import pandas as pd
from fastdtw import fastdtw
from scipy.spatial.distance import euclidean
import pickle
import matplotlib.pyplot as plt
from itertools import combinations, product


# Configuration
# diode_configurations = [("triangle_666_606060", [1500, 500, 250]), ("triangle_555_606060", [1500, 500, 250]), ("triangle_444_606060", [1500, 500, 250]), ("triangle_333_606060", [1500, 500, 250]), ("triangle_222_606060", [1500, 500, 250]), ("triangle_424_753075", [1500, 500]), ("triangle_626_802080", [1500, 500]), ("triangle_343_4010040", [1500, 500]), ("triangle_464_2513025", [1500, 500])]
diode_configurations = [("triangle_666_606060", [10000, 1500, 500, 250]), ("triangle_555_606060", [10000, 1500, 500, 250]), ("triangle_444_606060", [10000, 1500, 500, 250]), ("triangle_333_606060", [10000, 1500, 500, 250]), ("triangle_222_606060", [10000, 1500, 500, 250]), ("triangle_424_753075", [10000, 1500, 500, 250]), ("triangle_626_802080", [10000, 1500, 500, 250]), ("triangle_343_4010040", [10000, 1500, 500, 250]), ("triangle_464_2513025", [10000, 1500, 500, 250])]

samples_per_hand = 5
gestures = ["swipe_left", "swipe_right", "swipe_up", "swipe_down", "tap"]#, "clockwise", "counterclockwise", "tap", "double_tap"]

normal_scores = []
subtracted_scores = []

def calculate_DTW_distance(diode_configuration, gesture0, gesture1):
    # Obtain data for first gesture
    data0 = []
    for lux_level in diode_configuration[1]:
        for hand in ["right_hand", "left_hand"]:
            #print(f"Collecting {gesture0} for {hand}")
            path =  f"src/data_collection/data/final/{diode_configuration[0]}/{lux_level}/{gesture0}/{hand}"
            with open(path + "/data.pickle", "rb") as file:
                for _ in range(samples_per_hand):
                    try:
                        data0.append(pickle.load(file))
                    except:
                        print(f"unable to collect for {gesture0}: {hand} at {lux_level}. placement is {diode_configuration[0]}")

    # Obtain data for second gesture
    data1 = []
    for lux_level in diode_configuration[1]:
        for hand in ["right_hand", "left_hand"]:
            #print(f"Collecting {gesture1} for {hand}")
            path =  f"src/data_collection/data/final/{diode_configuration[0]}/{lux_level}/{gesture1}/{hand}"
            with open(path + "/data.pickle", "rb") as file:
                for _ in range(samples_per_hand):
                    try:
                        data1.append(pickle.load(file))
                    except:
                        print(f"unable to collect for {gesture1}: {hand} at {lux_level}. placement is {diode_configuration[0]}")

    # Now calculate average DTW distance.
    total_distance = 0
    prod = list(product(data0, data1))
    total_matches = len(prod)
    for i in range(len(prod)):
        distance, _ = fastdtw(prod[i][0], prod[i][1], dist=euclidean)
        total_distance += distance

        total_matches = total_matches - 1 if distance == 0 else total_matches # remove a match if the distance is 0 (comparing against self)
    return total_distance / total_matches
    
    

def calculate_distance_matrix(diode_configuration, gesture_set):
    options = list(combinations(gesture_set, 2))
    for i in gesture_set:
        options.append((i, i))


    similarity_matrix = np.matrix(np.zeros((len(gesture_set), len(gesture_set))))


    for i in options:
        distance = calculate_DTW_distance(diode_configuration, i[0], i[1])
        similarity_matrix[gesture_set.index(i[0]), gesture_set.index(i[1])] = distance
        similarity_matrix[gesture_set.index(i[1]), gesture_set.index(i[0])] = distance


    print(f"-------Performance statistics-------{diode_configuration}\n\n")

    print("Distances matrix")
    # Nicely format the similarity_matrix
    a = pd.DataFrame(similarity_matrix, index=gesture_set, columns=gesture_set)
    print(a)
    print("\n\n")

    np.savetxt(f"src/DTW/results/original/{diode_configuration[0]}.csv", similarity_matrix, delimiter=' & ', fmt='%2.2f', newline=' \\\\\n')

    normalized_similarity_matrix = similarity_matrix.copy()
    subtracted_similarity_matrix = similarity_matrix.copy()
    for i in range(len(gesture_set)):
        diagonal_elem = normalized_similarity_matrix[i, i]
        normalized_similarity_matrix[:, i] /= diagonal_elem

    print("Column normalized Distances matrix")
    a = pd.DataFrame(normalized_similarity_matrix, index=gesture_set, columns=gesture_set)
    print(a)

    normal_score = normalized_similarity_matrix.sum()
    print("Normalized Score: " + str(normal_score))

    np.savetxt(f"src/DTW/results/normal/{diode_configuration[0]}.csv", normalized_similarity_matrix, delimiter=' & ', fmt='%2.2f', newline=' \\\\\n')
    normal_scores.append((diode_configuration, normal_score))

    for i in range(len(gesture_set)):
        diagonal_elem = subtracted_similarity_matrix[i, i]
        subtracted_similarity_matrix[:, i] -= diagonal_elem

    print("Column subtracted Distances matrix")
    a = pd.DataFrame(subtracted_similarity_matrix, index=gesture_set, columns=gesture_set)
    print(a)

    subtracted_score = subtracted_similarity_matrix.sum()
    print("Subtracted Score: " + str(subtracted_score))

    np.savetxt(f"src/DTW/results/subtracted/{diode_configuration[0]}.csv", subtracted_similarity_matrix, delimiter=' & ', fmt='%2.2f', newline=' \\\\\n')
    subtracted_scores.append((diode_configuration, subtracted_score))


for i in diode_configurations:
    calculate_distance_matrix(diode_configuration=i, gesture_set=gestures)


print("Normal Scores:\n")
for i in normal_scores:
    print(f"{i[0][0]}: {i[1]}")



print("Subtracted Scores:\n")
for i in subtracted_scores:
    print(f"{i[0][0]}: {i[1]}")
