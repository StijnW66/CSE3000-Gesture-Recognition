from math import dist, perm
import numpy as np
from fastdtw import fastdtw
from pyparsing import dictOf
from scipy.spatial.distance import euclidean
import pickle
import matplotlib.pyplot as plt
from itertools import combinations, product


# Configuration
diode_configurations = ["triangle_555_606060"]
lux_level = -1

samples_per_hand = 2
gestures = ["swipe_left", "swipe_right"]#, "swipe_up", "swipe_down", "clockwise", "counterclockwise", "tap", "double_tap"]

def calculate_DTW_distance(diode_configuration_name, gesture0, gesture1):
    # Obtain data for first gesture
    data0 = []
    for hand in ["right_hand", "left_hand"]:
        path =  f"src/data_collection/data/{diode_configuration_name}/{lux_level}/{gesture0}/{hand}"
        with open(path + "/data.pickle", "rb") as file:
            for _ in range(samples_per_hand):
                data0.append(pickle.load(file))

    # Obtain data for second gesture
    data1 = []
    for hand in ["right_hand", "left_hand"]:
        path =  f"src/data_collection/data/{diode_configuration_name}/{lux_level}/{gesture1}/{hand}"
        with open(path + "/data.pickle", "rb") as file:
            for _ in range(samples_per_hand):
                data1.append(pickle.load(file))

    # Now calculate average DTW distance. TODO Match do more matches
    total_distance = 0
    prod = list(product(data0, data1))
    print(len(prod))
    for i in range(len(prod)):
        distance, _ = fastdtw(prod[i][0], prod[i][1], dist=euclidean)
        print(distance)
        total_distance += distance
    return total_distance / (len(prod))
    
    

options = list(combinations(gestures, 2))
options.append(("swipe_left", "swipe_left"))
options.append(("swipe_right", "swipe_right"))
for i in options:
    distance = calculate_DTW_distance(diode_configurations[0], i[0], i[1])
    print(f"Average distance of gesture: {i[0]} vs {i[1]} is {distance}")