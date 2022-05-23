from typing import List, Tuple
from scipy.io import arff
from os import listdir, path
from sklearn.model_selection import train_test_split
import tensorflow as tf
import numpy as np
import pandas as pd
import pickle
import constants

def _load_all_pickled_data(file_path) -> List:
    """
    Completely exhausts a file containing pickled data, returning all read items as elements of a list.

    Args:
        file_path: Path to a file containing pickled data

    Returns:
        A list containing all of the pickled data items that were read from the file
    """
    data = []
    with open(file_path, "rb") as open_file:
        while True:
            try:
                data.append(pickle.load(open_file))
            except EOFError:
                return data

def load_and_combine_initial_raw_data() -> Tuple[np.ndarray, np.ndarray]:
    """
    Load the initial raw dataset, combining the left and right hand results for each class.

    Returns:
        features: Numpy array where each entry is an (m x 3 x 1) 'image'
        labels: Numpy array where each entry is an integer corresponding to a class
    """
    class_count = 0
    features = []
    labels = []

    raw_data_path = path.join("data", "initial_raw_data")
    for gesture_name in listdir(raw_data_path):
        # Load and combine left and right hand data
        left_hand_data_path = path.join(raw_data_path, gesture_name, "left_hand", "data.pickle")
        left_hand_data = _load_all_pickled_data(left_hand_data_path)
        right_hand_data_path = path.join(raw_data_path, gesture_name, "right_hand", "data.pickle")
        right_hand_data = _load_all_pickled_data(right_hand_data_path)
        combined_data = np.append(left_hand_data, right_hand_data, axis=0)

        # Extend features list and create corresponding label list entries
        features.extend(combined_data)
        labels.extend([class_count for data_point in combined_data])
        class_count += 1

    # Convert to numpy arrays and add channel dimension to features
    features = np.array(features, dtype=np.uint16)
    features = np.expand_dims(features, -1)
    labels = np.array(labels, dtype=np.uint8)
    return features, labels


def load_and_combine_uwave() -> Tuple[np.ndarray, np.ndarray]:
    """
    Load the UWaveGestureLibrary data and combine the TRAIN and TEST data points into a single block.

    Returns:
        features: Numpy array where each entry is a flat array of a sample's readings
        labels: Numpy array where each entry is an integer corresponding to a class
    """
    # Load and combine train and test data
    train_data_path = path.join("data", "UWaveGestureLibraryAll_TRAIN.arff")
    data_train, meta_train = arff.loadarff(train_data_path)
    test_data_path = path.join("data", "UWaveGestureLibraryAll_TEST.arff")
    data_test, meta_test = arff.loadarff(test_data_path)
    combined_data = np.hstack((data_train, data_test))

    # Separate features and labels
    features = pd.DataFrame(combined_data)
    labels = features.pop("target")

    return features.to_numpy(np.float32), labels.to_numpy(np.uint8)


def preprocess_input(features: np.ndarray, labels: np.ndarray) -> Tuple[np.ndarray, np.ndarray]:
    """
    Preprocessing for input data.

    Args:
        features: Numpy array where each entry is a flat array of a sample's readings

    Returns:
        Numpy array where each entry is a (m x 3 x 1) 'image'
    """
    # Reshape each flat entry of reading to an (m X 3) 'image'
    num_samples = features.shape[1]
    processed_features = np.empty((features.shape[0], num_samples // 3, 3))
    for idx, row in enumerate(features):
        processed_features[idx] = np.reshape(row, (num_samples // 3, 3))

    # Add single channel dimension
    processed_features = np.expand_dims(processed_features, -1)

    # Class labels are in range (1, 8) so constrain to (0, 7) for TF
    decrement = lambda num : num - 1
    labels = decrement(labels)

    return processed_features, labels


def split_to_tf_datasets(features: np.ndarray, labels: np.ndarray) -> Tuple[tf.data.Dataset, tf.data.Dataset]:
    """
    Split a dataset into train and test sets and convert them to Tensorflow Datasets

    Args:
        features: Numpy array where each entry is an (m x 3 x 1) 'image'
        labels: Numpy array where each entry is an integer corresponding to a class

    Returns:
        train_dataset: Batched Tensorflow dataset for use with training
        test_dataset: Batched Tensorflow dataset for use with testing
    """
    x_train, x_test, y_train, y_test = train_test_split(features,
                                                        labels,
                                                        test_size=0.25,
                                                        random_state=constants.RANDOM_SEED)
    train_dataset = tf.data.Dataset.from_tensor_slices((x_train, y_train)).batch(constants.BATCH_SIZE)
    test_dataset = tf.data.Dataset.from_tensor_slices((x_test, y_test)).batch(constants.BATCH_SIZE)
    return train_dataset, test_dataset


if __name__ == "__main__":
    print("===== UWAVE DATA =====")
    features, labels = load_and_combine_uwave()
    features, labels = preprocess_input(features, labels)
    print("Feature data shape:", features.shape)
    print("Label data shape:", labels.shape)

    print("===== INITIAL RAW DATASET =====")
    features, labels = load_and_combine_initial_raw_data()
    print("Feature data shape: ", features.shape)
    print("Labels data shape: ", labels.shape)
