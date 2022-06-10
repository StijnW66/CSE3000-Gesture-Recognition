import numpy as np
import pandas as pd
from fastdtw import fastdtw
from scipy.spatial.distance import euclidean
import pickle
import matplotlib.pyplot as plt
from itertools import combinations, product


# Configuration
diode_configurations = ["triangle_666_606060"]
lux_level = 1000

samples_per_hand = 5
gestures = ["swipe_left", "swipe_right", "swipe_up", "swipe_down", "clockwise", "counterclockwise", "tap", "double_tap"]

def calculate_DTW_distance(diode_configuration_name, gesture0, gesture1):
    # Obtain data for first gesture
    data0 = []
    for hand in ["right_hand", "left_hand"]:
        print(f"Collecting {gesture0} for {hand}")
        path =  f"src/data_collection/data/{diode_configuration_name}/{lux_level}/{gesture0}/{hand}"
        with open(path + "/data.pickle", "rb") as file:
            for _ in range(samples_per_hand):
                data0.append(pickle.load(file))

    # Obtain data for second gesture
    data1 = []
    for hand in ["right_hand", "left_hand"]:
        print(f"Collecting {gesture1} for {hand}")
        path =  f"src/data_collection/data/{diode_configuration_name}/{lux_level}/{gesture1}/{hand}"
        with open(path + "/data.pickle", "rb") as file:
            for _ in range(samples_per_hand):
                data1.append(pickle.load(file))

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
        distance = calculate_DTW_distance(diode_configurations[0], i[0], i[1])
        similarity_matrix[gesture_set.index(i[0]), gesture_set.index(i[1])] = distance
        similarity_matrix[gesture_set.index(i[1]), gesture_set.index(i[0])] = distance


    print("-------Performance statistics-------\n\n")

    print("Distances matrix")
    # Nicely format the similarity_matrix
    a = pd.DataFrame(similarity_matrix, index=gesture_set, columns=gesture_set)
    print(a)
    print("\n\n")

    avg = 0
    for i in range(len(gesture_set)):
        avg += similarity_matrix[i, i]
    avg /= len(gesture_set)
    print(f"Average on the diagonal {avg}")

    similarity_matrix /= avg
    print("Distances matrix (factor of diagonal average)")
    a = pd.DataFrame(similarity_matrix, index=gesture_set, columns=gesture_set)
    print(a)


calculate_distance_matrix(diode_configuration=diode_configurations[0], gesture_set=gestures)