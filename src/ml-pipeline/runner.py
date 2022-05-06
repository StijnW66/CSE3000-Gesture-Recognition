import tensorflow as tf
import numpy as np
import models
from data_processing import load_and_combine_data, preprocess_input, split_to_tf_datasets

EPOCHS = 25

def compile_model(model: tf.keras.Model):
    model.summary()
    model.compile(
        optimizer=tf.keras.optimizers.Adam(),
        loss=tf.keras.losses.SparseCategoricalCrossentropy(),
        metrics=[tf.keras.metrics.SparseCategoricalAccuracy()]
    )


def train_and_evaluate(model: tf.keras.Model,
                       x_train: np.ndarray, x_test: np.ndarray,
                       y_train: np.ndarray, y_test: np.ndarray):
    model.fit(
        x_train, y_train,
        epochs=EPOCHS)
    score = model.evaluate(x_test, y_test, verbose=1)
    print("Test loss:", score[0])
    print("Test accuracy:", score[1])


def train_and_evaluate(model: tf.keras.Model,
                       train_dataset: tf.data.Dataset,
                       test_dataset: tf.data.Dataset):
    model.fit(
        train_dataset,
        epochs=EPOCHS)
    score = model.evaluate(test_dataset, verbose=1)
    print("Test loss:", score[0])
    print("Test accuracy:", score[1])


if __name__ == "__main__":
    features, labels = load_and_combine_data()
    features, labels = preprocess_input(features, labels)
    train_dataset, test_dataset = split_to_tf_datasets(features, labels)
    running_model = models.slam_cnn(features[0].shape)

    compile_model(running_model)
    train_and_evaluate(running_model, train_dataset, test_dataset)
