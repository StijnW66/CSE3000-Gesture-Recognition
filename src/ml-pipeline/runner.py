import tensorflow as tf
import numpy as np
import models
import data_processing

EPOCHS = 500

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


def train_uwave():
    print("========== UWAVE DATA ==========")
    features, labels = data_processing.load_and_combine_uwave()
    features, labels = data_processing.preprocess_input(features, labels)
    train_dataset, test_dataset = data_processing.split_to_tf_datasets(features, labels)
    running_model = models.slam_cnn_padding(features[0].shape, data_processing.NUM_CLASSES_UWAVE)

    compile_model(running_model)
    train_and_evaluate(running_model, train_dataset, test_dataset)


def train_initial_raw_dataset():
    print("========== INITIAL RAW DATASET ==========")
    features, labels = data_processing.load_and_combine_initial_raw_data()
    train_dataset, test_dataset = data_processing.split_to_tf_datasets(features, labels)
    running_model = models.slam_cnn_padding(features[0].shape, data_processing.NUM_CLASSES_INITIAL_RAW_DATA)
    
    compile_model(running_model)
    train_and_evaluate(running_model, train_dataset, test_dataset)


if __name__ == "__main__":
    train_initial_raw_dataset()
