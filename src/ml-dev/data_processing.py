from typing import Iterable, List, Tuple
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


def _load_and_combine_all_candidates(data_path: str, num_to_process: int = 100) -> np.ndarray:
    """
    Load and combine the data of a particular gesture from all candidates.

    Args:
        data_path: Path to the folder containing the .pickle files of a particular gesture
        as performed with a particular hand
        num_to_process: Number of candidates to (maximally) process

    Returns:
        Numpy array where each entry is an (m x 3 x 1) 'image'
    """
    to_process = [f"candidate_{i}.pickle" for i in range (1, num_to_process + 1)]
    combined_data = np.empty((0, 100, 3), dtype=np.float32) # TODO: Make this more programmatic instead of hardcording data sizes
    for candidate in listdir(data_path):
        if candidate in to_process:
            file_name = path.join(data_path, candidate)
            candidate_data = _load_all_pickled_data(file_name)
            combined_data = np.append(combined_data, candidate_data, axis=0)
    return combined_data


def load_and_combine_data(raw_data_path: str = path.join("data", "processed_data"),
                          num_to_process: int = 100) -> Tuple[np.ndarray, np.ndarray]:
    """
    Load the processed dataset, combining the left and right hand results across all
    candidates for each class.

    Args:
        raw_data_path: Root directory where data for each class is stored
        num_to_process: Number of candidates to (maximally) process

    Returns:
        features: Numpy array where each entry is an (m x 3 x 1) 'image'
        labels: Numpy array where each entry is an integer corresponding to a class
    """
    class_count = 0
    features = []
    labels = []

    for gesture_name in listdir(raw_data_path):
        # Load and combine left and right hand data
        left_hand_data_path = path.join(raw_data_path, gesture_name, "left_hand")
        left_hand_data = _load_and_combine_all_candidates(left_hand_data_path, num_to_process)
        right_hand_data_path = path.join(raw_data_path, gesture_name, "right_hand")
        right_hand_data = _load_and_combine_all_candidates(right_hand_data_path, num_to_process)
        combined_data = np.append(left_hand_data, right_hand_data, axis=0)

        # Extend features list and create corresponding label list entries
        features.extend(combined_data)
        labels.extend([class_count for data_point in combined_data])
        class_count += 1

    # Convert to numpy arrays and add channel dimension to features
    features = np.array(features, dtype=np.float32)
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
    train_data_path = path.join("uwave", "data", "UWaveGestureLibraryAll_TRAIN.arff")
    data_train, meta_train = arff.loadarff(train_data_path)
    test_data_path = path.join("uwave", "data", "UWaveGestureLibraryAll_TEST.arff")
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


def reshape_to_sensor_output(features: np.ndarray) -> List:
    """
    Reshape the array of features to emulate the tranposed output
    of the signal processing pipeline

    Args:
        features: Numpy array where each entry is an (m x 3 x 1) 'image'

    Returns:
        List where each entry is a (3 x m x 1) 'image'
    """
    reshaped_output = []
    for image in features:
        image = np.squeeze(image)
        reshaped_output.append(np.array([image[:, 0], image[:, 1], image[:, 2]]))
    return reshaped_output


def print_feature_label_pair(features: Iterable, labels: np.ndarray, idx: int):
    print(f"=== FEATURE LIST PAIR AT INDEX {idx} ===")
    print("Feature:")
    print(repr(np.array(features[idx], dtype=np.float32)))
    print(f"Label: {labels[idx]}")


if __name__ == "__main__":
    print("===== PROCESSED DATASET =====")
    features, labels = load_and_combine_data(num_to_process=29)
    print("Feature data shape: ", features.shape)
    print("Labels data shape: ", labels.shape)

    print("===== RAW DATASET =====")
    features, labels = load_and_combine_data(path.join("data", "raw_data"), num_to_process=29)
    print("Feature data shape: ", features.shape)
    print("Labels data shape: ", labels.shape)
