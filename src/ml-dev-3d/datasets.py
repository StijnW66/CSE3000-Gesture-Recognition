import pickle
import random

import pandas as pd
from scipy.io import arff

from data_processing import *


def load_data_set(data_set_name: str, frame_length: int = 1) -> DataSet:
    """
    Creates a DataSet object by reading & processing files of a chosen dataset.

    :param data_set_name: The name of the dataset to load. Possible options can be found in data_set_dict.
    :param frame_length: The number of time steps to include in a frame. Each 2D data sequence is split into frames in
        order to convert it into a 3D format.

    :return: A DataSet object representing the chosen dataset.
    """
    print()
    print('################################################################################################')
    print('Loading dataset: ' + data_set_name + '...')
    print('################################################################################################')
    print()

    data_set: DataSet = []

    def project():
        data_set_path: str = './datasets/' + data_set_name + '/'

        class_indices = {
            'initial': {
                0: 'swipe_up/',
                1: 'swipe_down/',
                2: 'swipe_left/',
                3: 'swipe_right/'
            },
            'extended': {
                0: 'swipe_up/',
                1: 'swipe_down/',
                2: 'swipe_left/',
                3: 'swipe_right/',
                4: 'clockwise/',
                5: 'counterclockwise/',
                6: 'tap/',
                7: 'double_tap/'
            }
        }

        def load_pickle(file_path: str) -> List[List[List[int]]]:
            data = []
            with open(file_path, "rb") as open_file:
                while True:
                    try:
                        data.append(pickle.load(open_file))
                    except EOFError:
                        return data

        def add_data(data: List[List[List[int]]], num_classes: int, class_index: int):
            for instance in data:
                time_sequence: TimeSequence3D = []
                time_frame: TimeFrame = []

                for step_num in range(100):
                    time_step: TimeStep = [
                        float(instance[step_num][0]),
                        float(instance[step_num][1]),
                        float(instance[step_num][2])
                    ]

                    time_frame.append(time_step)

                    if (step_num + 1) % frame_length == 0:
                        time_sequence.append(time_frame)
                        time_frame = []

                class_encoding: ClassEncoding = [0.0] * num_classes
                class_encoding[class_index] = 1.0

                data_instance: DataInstance = DataInstance(time_sequence, class_encoding)
                data_set.append(data_instance)

        for i in range(len(class_indices[data_set_name])):
            left_hand_class_data = load_pickle(
                data_set_path +
                class_indices[data_set_name][i] +
                'left_hand/data.pickle'
            )
            add_data(left_hand_class_data, len(class_indices[data_set_name]), i)

            right_hand_class_data = load_pickle(
                data_set_path +
                class_indices[data_set_name][i] +
                'right_hand/data.pickle'
            )
            add_data(right_hand_class_data, len(class_indices[data_set_name]), i)

    data_set_dict = {
        'initial': project,
        'extended': project
    }

    data_set_dict[data_set_name]()

    random.shuffle(data_set)

    return data_set
