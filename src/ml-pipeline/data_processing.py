from concurrent.futures import process
from typing import Tuple
from scipy.io import arff
from os import path
from sklearn.model_selection import train_test_split
import tensorflow as tf
import numpy as np
import pandas as pd

BATCH_SIZE = 128
RANDOM_SEED = 69 # Allows for train-test split to be deterministic
NUM_CLASSES = 8


def load_and_combine_data() -> Tuple[np.ndarray, np.ndarray]:
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

    return features.to_numpy(np.float32), labels.to_numpy(np.int8)


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
        train_dataset: Tensorflow dataset for use with training
        test_dataset: Tensorflow dataset for use with testing
    """
    x_train, x_test, y_train, y_test = train_test_split(features,
                                                        labels,
                                                        test_size=0.25,
                                                        random_state=RANDOM_SEED)
    train_dataset = tf.data.Dataset.from_tensor_slices((x_train, y_train)).batch(BATCH_SIZE)
    test_dataset = tf.data.Dataset.from_tensor_slices((x_test, y_test)).batch(BATCH_SIZE)
    return train_dataset, test_dataset


if __name__ == "__main__":
    features, labels = load_and_combine_data()
    features, labels = preprocess_input(features, labels)
    print("Feature data shape:", features.shape)
    print("Label data shape:", labels.shape)
