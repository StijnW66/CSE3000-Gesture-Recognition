from random import randrange
from typing import List, Tuple, Union
from typing import NamedTuple

import matplotlib.pyplot as plt
import numpy as np

# Single 1D time step with a value for each feature.
TimeStep = List[float]
# N 1D time steps concatenated into a 2D frame.
TimeFrame = List[TimeStep]
# All 1D time steps concatenated into a 2D sequence.
TimeSequence2D = List[TimeStep]
# All 2D frames concatenated into a 3D sequence.
TimeSequence3D = List[TimeFrame]
# One-hot encoding of the data classes.
ClassEncoding = List[float]


# Representation of a single instance in the dataset.
class DataInstance(NamedTuple):
    # Model input data.
    time_sequence: Union[TimeSequence3D, TimeSequence2D]
    # Expected model output data.
    class_encoding: ClassEncoding


# Entire dataset, made up of all data instances.
DataSet = List[DataInstance]


# Stores important parameters for a given dataset.
class DataSetInfo(NamedTuple):
    num_features: int
    num_classes: int
    num_instances: int
    sequence_length: int
    frame_length: Union[int, None]


def get_data_set_info(data_set: DataSet) -> DataSetInfo:
    """
    Returns important parameters for a given dataset.

    :param data_set: The dataset to check parameters of.

    :return: A DataSetInfo object containing the dataset parameters.
    """
    num_features: int = len(data_set[0].time_sequence[0])
    num_classes: int = len(data_set[0].class_encoding)
    num_instances: int = len(data_set)
    sequence_length: int = len(data_set[0].time_sequence)
    # None indicates that the dataset was flattened.
    frame_length: Union[int, None] = None

    # Checks if data instances have 2D or 3D time sequences.
    if type(data_set[0].time_sequence[0][0]) == list:
        num_features = len(data_set[0].time_sequence[0][0])
        frame_length = len(data_set[0].time_sequence[0])

    return DataSetInfo(num_features, num_classes, num_instances, sequence_length, frame_length)


def normalize_data_set(data_set: DataSet) -> DataSet:
    """
    Normalizes all values in a dataset to be between 0 and 1.

    :param data_set: The dataset to normalize.

    :return: The same dataset, now with normalized values.
    """
    x: np.ndarray[Union[TimeSequence3D, TimeSequence2D]] = np.array(list(
        map(lambda data_instance: data_instance.time_sequence, data_set)
    ))
    y: np.ndarray[ClassEncoding] = np.array(list(
        map(lambda data_instance: data_instance.class_encoding, data_set)
    ))

    min_value: float = np.amin(x)
    max_value: float = np.amax(x)

    x = (x - min_value) / (max_value - min_value)

    normalized_data_set: DataSet = []

    for i in range(len(data_set)):
        normalized_data_set.append(DataInstance(x[i], y[i]))

    return normalized_data_set


def flatten_data_set(data_set: DataSet) -> DataSet:
    """
    If the data in a dataset is 3D-formatted, i.e. split into frames, this function concatenates the time steps in each
    frame, making the frames 1-dimensional instead of 2-dimensional. This is done to make the data suitable for
    non-convolutional machine learning models. It is worth noting that converting 3D-formatted data to 2D-formatting
    using this function will increase the number of input features, as they are now concatenated across several time
    steps.

    :param data_set: The dataset to flatten.

    :return: The flattened dataset if the input dataset is 3D-formatted, or the original dataset otherwise.
    """
    if type(data_set[0].time_sequence) == TimeSequence2D:
        return data_set

    flattened_data_set: DataSet = []

    for data_instance in data_set:
        flattened_time_sequence: TimeSequence2D = []

        for time_frame in data_instance.time_sequence:
            # 1D representation of 2D time frame - effectively a concatenation of time steps.
            frame_vector: TimeStep = []

            for time_step in time_frame:
                frame_vector.extend(time_step)

            flattened_time_sequence.append(frame_vector)

        flattened_data_set.append(DataInstance(flattened_time_sequence, data_instance.class_encoding))

    return flattened_data_set


def fold_data_set(data_set: DataSet, num_folds: int) -> List[Tuple[DataSet, DataSet]]:
    """
    Copies a dataset into a number of 'folds' which contain a training and validation set each. The size of the training
    and validation sets for each fold depends on the number of total folds. For example, with 8 total folds, 1/8th of
    each fold is reserved for validation while the remaining data is used for training. Although the size of each set
    remains the same across all folds, each fold reserves a different part of the dataset for validation compared
    to the others. If the number of folds is 1, the fold returned has a 90/10 training/validation split.

    :param data_set: The dataset to fold.
    :param num_folds: The number of folds to partition the dataset into.

    :return: Several copies, or 'folds' of the original dataset, each containing a different training/validation split.
    """
    folds: List[Tuple[DataSet, DataSet]] = []

    # Special case for when the number of folds is 1.
    if num_folds == 1:
        # If the number of data instances in the dataset is not divisible by the number of folds, the remaining
        # instances are discarded.
        fold_size: int = len(data_set) // 10

        training_set: DataSet = []
        validation_set: DataSet = []

        for i in range(0, 9 * fold_size):
            training_set.append(data_set[i])

        for i in range(9 * fold_size, 10 * fold_size):
            validation_set.append(data_set[i])

        folds.append((training_set, validation_set))

    else:
        for fold_num in range(num_folds):
            # If the number of data instances in the dataset is not divisible by the number of folds, the remaining
            # instances are discarded.
            fold_size: int = len(data_set) // num_folds

            training_set: DataSet = []
            validation_set: DataSet = []

            for i in range(0, fold_num * fold_size):
                training_set.append(data_set[i])

            for i in range(fold_num * fold_size, (fold_num + 1) * fold_size):
                validation_set.append(data_set[i])

            for i in range((fold_num + 1) * fold_size, num_folds * fold_size):
                training_set.append(data_set[i])

            folds.append((training_set, validation_set))

    return folds


def plot_example_data(data_set: DataSet) -> None:
    """
    Plots a random data instance from a chosen dataset to better visualize it.

    :param data_set: The dataset to plot an instance from.
    """
    random_index: int = randrange(len(data_set))
    data_instance = data_set[random_index]

    x_dim = []
    y_dim = []
    z_dim = []

    for time_frame in data_instance.time_sequence:
        for time_step in time_frame:
            x_dim.append(time_step[0])
            y_dim.append(time_step[1])
            z_dim.append(time_step[2])

    class_num: str = str(np.argmax(data_instance.class_encoding))

    plt.title('Example data for class ' + class_num)
    plt.plot(x_dim, 'r')
    plt.plot(y_dim, 'g')
    plt.plot(z_dim, 'b')
    plt.show()
