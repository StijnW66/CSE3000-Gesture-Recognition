from sklearn.model_selection import KFold, train_test_split
from rich.console import Console
import tensorflow as tf
import tensorflow_model_optimization as tfmot
import numpy as np
import data_processing
import models
import results_analysis

EPOCHS = 500
CONSOLE = Console()

def compile_model(model: tf.keras.Model):
    model.summary()
    model.compile(
        optimizer=tf.keras.optimizers.Adam(learning_rate=0.0001),
        loss=tf.keras.losses.SparseCategoricalCrossentropy(),
        metrics=[tf.keras.metrics.SparseCategoricalAccuracy()])


def train_and_evaluate_np(model: tf.keras.Model,
                       x_train: np.ndarray, x_test: np.ndarray,
                       y_train: np.ndarray, y_test: np.ndarray,
                       plot_metrics: bool = False):
    validation_split = 0.1 if plot_metrics else 0
    history = model.fit(
        x_train, y_train,
        batch_size=data_processing.BATCH_SIZE, epochs=EPOCHS, validation_split=validation_split)
    score = model.evaluate(x_test, y_test, verbose=1)

    print("Test loss:", score[0])
    print("Test accuracy:", score[1])
    if plot_metrics:
        results_analysis.plot_losses(history)
        results_analysis.plot_accuracies(history)


def train_and_evaluate_tf(model: tf.keras.Model,
                          train_dataset: tf.data.Dataset,
                          test_dataset: tf.data.Dataset):
    history = model.fit(train_dataset, epochs=EPOCHS)
    score = model.evaluate(test_dataset, verbose=1)
    print("Test loss:", score[0])
    print("Test accuracy:", score[1])


def kfold_cross_validation(model: tf.keras.Model, features: np.ndarray, labels: np.ndarray,
                           num_folds: int = 5, quantize: bool = False):
    """
    Perform k-fold cross validation and print summary of the results.

    Args:
        model: Uncompiled Keras model to perform validation on
        features: Numpy array where each entry is an (m x 3 x 1) 'image'
        labels: Numpy array where each entry is an integer corresponding to a class
        num_folds: Number of folds to cross-validate with
        quantize: Convert model to quantization-aware version
    """
    fold_num = 1
    acc_per_fold = []
    loss_per_fold = []
    kfold = KFold(num_folds, shuffle=True)
    if quantize:
        model = tfmot.quantization.keras.quantize_model(model)

    for train, test in kfold.split(features, labels):
        CONSOLE.rule(f"[bold red]Fold No.{fold_num}")

        # Compile and fit to training data
        compile_model(model)
        history = model.fit(features[train], labels[train],
                            batch_size=data_processing.BATCH_SIZE, epochs=EPOCHS,
                            verbose=2)

        # Generate generalization metrics
        scores = model.evaluate(features[test], labels[test], verbose=2)
        print(f'Score: {model.metrics_names[0]} of {scores[0]:.3f}; {model.metrics_names[1]} of {scores[1]*100:.3f}%')
        acc_per_fold.append(scores[1] * 100)
        loss_per_fold.append(scores[0])

        fold_num += 1

    # Print summary of results
    CONSOLE.rule(f"[bold cyan]{num_folds}-fold Validation Summary")
    print('== Score per fold:')
    for i in range(0, num_folds):
        print(f'> Fold {i+1} - Loss: {loss_per_fold[i]:.3f} - Accuracy: {acc_per_fold[i]:.3f}%')
    print()
    print('== Average scores for all folds:')
    print(f'> Accuracy: {np.mean(acc_per_fold):.3f}% (+- {np.std(acc_per_fold):.3f}%)')
    print(f'> Loss: {np.mean(loss_per_fold):.3f}')


# ===== CONVENIENCE METHODS =====


def train_uwave():
    print("========== UWAVE DATA ==========")
    features, labels = data_processing.load_and_combine_uwave()
    features, labels = data_processing.preprocess_input(features, labels)
    train_dataset, test_dataset = data_processing.split_to_tf_datasets(features, labels)
    running_model = models.slam_cnn_padding(features[0].shape, data_processing.NUM_CLASSES_UWAVE)

    compile_model(running_model)
    train_and_evaluate_tf(running_model, train_dataset, test_dataset, plot_metrics=True)


def train_initial_raw_dataset():
    print("========== INITIAL RAW DATASET ==========")
    features, labels = data_processing.load_and_combine_initial_raw_data()
    x_train, x_test, y_train, y_test = train_test_split(features,
                                                        labels,
                                                        test_size=0.25,
                                                        random_state=data_processing.RANDOM_SEED)

    running_model = models.slam_cnn_padding_pyramid(features[0].shape, data_processing.NUM_CLASSES_INITIAL_RAW_DATA)
    compile_model(running_model)
    train_and_evaluate_np(running_model, x_train, x_test, y_train, y_test, plot_metrics=True)


if __name__ == "__main__":
    features, labels = data_processing.load_and_combine_initial_raw_data()
    kfold_cross_validation(
        models.slam_cnn_padding_pyramid(features[0].shape, data_processing.NUM_CLASSES_INITIAL_RAW_DATA),
        features, labels)
