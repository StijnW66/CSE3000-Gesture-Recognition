from os import path
from typing import List, Tuple
from skimage.measure import block_reduce
from sklearn.metrics import confusion_matrix
from sklearn.model_selection import KFold, train_test_split
from sys import getsizeof
from tf_lite_conversion import _convert_with_quantization
import tensorflow as tf
import tensorflow_model_optimization as tfmot
import numpy as np
import constants
import data_processing
import models
import results_analysis


def compile_model(model: tf.keras.Model):
    model.summary()
    model.compile(
        optimizer=tf.keras.optimizers.Adam(learning_rate=0.001),
        loss=tf.keras.losses.SparseCategoricalCrossentropy(),
        metrics=[tf.keras.metrics.SparseCategoricalAccuracy()])


def train_and_evaluate_np(model: tf.keras.Model,
                          x_train: np.ndarray, x_test: np.ndarray,
                          y_train: np.ndarray, y_test: np.ndarray,
                          plot_metrics: bool = False):
    validation_split = 0.1 if plot_metrics else 0
    history = model.fit(
        x_train, y_train,
        batch_size=constants.BATCH_SIZE, epochs=constants.EPOCHS, validation_split=validation_split)
    score = model.evaluate(x_test, y_test, verbose=1)

    print("Test loss:", score[0])
    print("Test accuracy:", score[1])
    if plot_metrics:
        results_analysis.plot_losses(history)
        results_analysis.plot_accuracies(history)


def train_and_evaluate_tf(model: tf.keras.Model,
                          train_dataset: tf.data.Dataset,
                          test_dataset: tf.data.Dataset):
    history = model.fit(train_dataset, epochs=constants.EPOCHS)
    score = model.evaluate(test_dataset, verbose=1)
    print("Test loss:", score[0])
    print("Test accuracy:", score[1])


def kfold_cross_validation(model: tf.keras.Model, features: np.ndarray, labels: np.ndarray,
                           num_folds: int = 5, quantize: bool = False) -> Tuple[List[float], List[float], List[np.ndarray]]:
    """
    Perform k-fold cross validation.

    Args:
        model: Uncompiled Keras model to perform validation on
        features: Numpy array where each entry is an (m x 3 x 1) 'image'
        labels: Numpy array where each entry is an integer corresponding to a class
        num_folds: Number of folds to cross-validate with
        quantize: Convert model to quantization-aware version

    Returns:
        acc_per_fold: A list with the testing accuracy computed for each fold
        loss_per_fold: A list with the testing loss computed for each fold
        confusion_per_fold: A list with the confusion matrix computed for each fold
    """
    fold_num = 1
    acc_per_fold = []
    loss_per_fold = []
    confusion_per_fold = []
    kfold = KFold(num_folds, shuffle=True, random_state=constants.RANDOM_SEED) # TODO: CHANGE THIS FOR ACTUAL VALIDATION THIS CAN CAUSE HOUSEFIRES
    if quantize:
        model = tfmot.quantization.keras.quantize_model(model)

    for train, test in kfold.split(features, labels):
        constants.CONSOLE.rule(f"[bold red]Fold No.{fold_num}")

        # Compile and fit to training data
        compile_model(model)
        history = model.fit(features[train], labels[train],
                            batch_size=constants.BATCH_SIZE, epochs=constants.EPOCHS,
                            verbose=2)

        # Generate evaluation metrics
        scores = model.evaluate(features[test], labels[test], verbose=2)
        predictions = np.argmax(model.predict(features[test]), axis=1)
        acc_per_fold.append(scores[1] * 100)
        loss_per_fold.append(scores[0])
        confusion_per_fold.append(confusion_matrix(labels[test], predictions))

        fold_num += 1

    return acc_per_fold, loss_per_fold, confusion_per_fold


def compare_models(models: List[tf.keras.Model], model_names: List[str],
                   features: np.ndarray, labels: np.ndarray, num_folds: int = 5,
                   quantize: bool = False):
    """
    Perform k-fold cross validation on a number of models and print evaluation results

    Args:
        models: Unfitted Keras models        
        model_names: Name of each of the given models
        features: Features to use for k-fold cross validation
        labels: Labels for corresponding features
        num_folds: Number of folds to use for k-fold cross validation (default: 5)
        quantize: Quantize models and utilise quantization-aware training (default: False)
    """
    results = [kfold_cross_validation(model, features, labels, num_folds, quantize) for model in models]
    accuracies = [acc_loss_pair[0] for acc_loss_pair in results]
    losses = [acc_loss_pair[1] for acc_loss_pair in results]
    quantized_sizes = [getsizeof(_convert_with_quantization(model, features, False)) for model in models]

    results_analysis.print_multiple_kfold_results(model_names, accuracies, losses, quantized_sizes)

# ===== CONVENIENCE METHODS =====

def train_uwave():
    print("========== UWAVE DATA ==========")
    features, labels = data_processing.load_and_combine_uwave()
    features, labels = data_processing.preprocess_input(features, labels)
    train_dataset, test_dataset = data_processing.split_to_tf_datasets(features, labels)
    running_model = models.slam_cnn_padding(features[0].shape, constants.NUM_CLASSES_UWAVE)

    compile_model(running_model)
    train_and_evaluate_tf(running_model, train_dataset, test_dataset, plot_metrics=True)


def train_initial_raw_dataset():
    print("========== RAW DATASET ==========")
    features_20_hz, labels_20_hz = data_processing.load_and_combine_data(path.join("data", "ideal_data"), num_to_process=29)
    features_100_hz, labels_100_hz = data_processing.load_and_combine_data(path.join("data", "ideal_data"), start_candidate=30, sample_length=500)
    features_100_hz = block_reduce(features_100_hz, block_size=(1, 5, 1, 1), func=np.mean)
    features = np.append(features_20_hz, features_100_hz, axis=0)
    labels = np.append(labels_20_hz, labels_100_hz, axis=0)
    x_train, x_test, y_train, y_test = train_test_split(features,
                                                        labels,
                                                        test_size=0.20,
                                                        random_state=constants.RANDOM_SEED)

    running_model = models.slam_cnn_padding_pyramid_lite(features[0].shape, constants.NUM_CLASSES_RAW_DATA)
    compile_model(running_model)
    train_and_evaluate_np(running_model, x_train, x_test, y_train, y_test, plot_metrics=True)

# ===== END CONVENIENCE METHODS =====

if __name__ == "__main__":
    # train_initial_raw_dataset()

    features_20_hz, labels_20_hz = data_processing.load_and_combine_data(path.join("data", "ideal_data"), num_to_process=29)
    features_100_hz, labels_100_hz = data_processing.load_and_combine_data(path.join("data", "ideal_data"), start_candidate=30, sample_length=500)
    features_100_hz = block_reduce(features_100_hz, block_size=(1, 5, 1, 1), func=np.mean)
    features = np.append(features_20_hz, features_100_hz, axis=0)
    labels = np.append(labels_20_hz, labels_100_hz, axis=0)

    acc_per_fold, loss_per_fold, confusion_per_fold = kfold_cross_validation(
        models.narrow_slam_cnn_padding_pyramid(features[0].shape, constants.NUM_CLASSES_RAW_DATA),
        features, labels, 5, False)
    results_analysis.print_kfold_results(acc_per_fold, loss_per_fold, confusion_per_fold)
    
    # model_list = [
    #     models.slam_cnn_padding(features[0].shape, constants.NUM_CLASSES_RAW_DATA),
    #     models.slam_cnn_padding_pyramid(features[0].shape, constants.NUM_CLASSES_RAW_DATA),
    #     models.slam_cnn_padding_pyramid_lite(features[0].shape, constants.NUM_CLASSES_RAW_DATA),
    #     models.narrow_slam_cnn_padding_pyramid(features[0].shape, constants.NUM_CLASSES_RAW_DATA)]
    # model_names = [
    #     "SLAM CNN Padding", "SLAM CNN Padding Pyramid",
    #     "SLAM CNN Padding Pyramid Lite", "Narrow SLAM CNN Padding Pyramid"]
    # compare_models(model_list, model_names, features, labels, num_folds=5)
