from datetime import datetime
from typing import IO

import tensorflow as tf

from datasets import *
from models import *

if __name__ == '__main__':
    DATA_SET_NAME: str = 'extended'
    FRAME_LENGTH: int = 1

    MODEL_NAME: str = 'lstm'
    NUM_NEURONS: int = 32

    BATCH_SIZE: int = 32
    NUM_EPOCHS: int = 1000
    NUM_VALIDATION_FOLDS: int = 1

    data_set: DataSet = load_data_set(DATA_SET_NAME, FRAME_LENGTH)
    print(len(data_set[0].time_sequence))
    print(len(data_set[0].time_sequence[0]))
    print(len(data_set[0].time_sequence[0][0]))
    data_set = normalize_data_set(data_set)
    data_set = flatten_data_set(data_set)

    folded_data_set: List[Tuple[DataSet, DataSet]] = fold_data_set(data_set, NUM_VALIDATION_FOLDS)
    data_set_info = get_data_set_info(data_set)

    training_accuracy: List[float] = []
    validation_accuracy: List[float] = []

    for i in range(len(folded_data_set)):
        model: keras.models.Model = create_model(MODEL_NAME,
                                                 data_set_info.num_features,
                                                 data_set_info.num_classes,
                                                 data_set_info.sequence_length,
                                                 data_set_info.frame_length,
                                                 NUM_NEURONS)

        # Splits training & validation data into features & classes for the current fold.
        x_training = np.asarray(list(map(lambda data_instance: data_instance.time_sequence, folded_data_set[i][0])))
        y_training = np.asarray(list(map(lambda data_instance: data_instance.class_encoding, folded_data_set[i][0])))
        x_validation = np.asarray(list(map(lambda data_instance: data_instance.time_sequence, folded_data_set[i][1])))
        y_validation = np.asarray(list(map(lambda data_instance: data_instance.class_encoding, folded_data_set[i][1])))

        print()
        print('################################################################################################')
        print('Fold', i + 1, 'training commencing...')
        print('################################################################################################')
        print()

        # Trains the model on the current fold and saves its training classification accuracy.
        fold_training_accuracy = model.fit(
            x_training,
            y_training,
            epochs=NUM_EPOCHS,
            batch_size=BATCH_SIZE,
            validation_data=(x_validation, y_validation)
        ).history['categorical_accuracy'][-1]
        training_accuracy.append(fold_training_accuracy)

        print()
        print('################################################################################################')
        print('Fold', i + 1, 'validation commencing...')
        print('################################################################################################')
        print()

        # Validates the model on the current fold and saves its actual classification accuracy.
        fold_validation_accuracy: float = model.evaluate(x_validation, y_validation)[-1]
        validation_accuracy.append(fold_validation_accuracy)

    print()
    print('################################################################################################')
    print('Validation complete. Final training commencing...')
    print('################################################################################################')
    print()

    model: keras.models.Model = create_model(MODEL_NAME,
                                             data_set_info.num_features,
                                             data_set_info.num_classes,
                                             data_set_info.sequence_length,
                                             data_set_info.frame_length,
                                             NUM_NEURONS)

    # Splits training data into features & classes for the full dataset.
    x = np.asarray(list(map(lambda data_instance: data_instance.time_sequence, data_set)))
    y = np.asarray(list(map(lambda data_instance: data_instance.class_encoding, data_set)))

    # Trains the model on the full dataset.
    history = model.fit(
        x,
        y,
        epochs=NUM_EPOCHS,
        batch_size=BATCH_SIZE,
    )

    print()
    print('################################################################################################')
    print('Training complete. Saving model to file...')
    print('################################################################################################')
    print()

    # Converts the model to TensorFlow Lite
    converter: tf.lite.TFLiteConverter = tf.lite.TFLiteConverter.from_keras_model(model)
    converter.target_spec.supported_ops = [
        tf.lite.OpsSet.TFLITE_BUILTINS,  # enable TensorFlow Lite ops.
        tf.lite.OpsSet.SELECT_TF_OPS  # enable TensorFlow ops.
    ]

    # Default space & latency optimizations e.g. quantization.
    converter.optimizations = [tf.lite.Optimize.DEFAULT]

    # Generator for data instances that are representative of the dataset, needed to optimize the converted model.
    def representative_dataset_generator() -> List[np.ndarray]:
        # In this case the data instances are actually just taken directly from the dataset.
        for time_sequence in x:
            # Funky list wrapping to correctly format the data for the TFLite converter.
            yield [np.array([time_sequence], dtype=np.float32)]
    converter.representative_dataset = representative_dataset_generator

    tf_lite_model = converter.convert()

    # Saves model to file
    file_name: str = \
        MODEL_NAME + '_' + \
        DATA_SET_NAME + '_' + \
        str(round(sum(validation_accuracy) / len(validation_accuracy), 2)) + '_' + \
        datetime.now().strftime('%m-%d-%Y_%H-%M-%S') + '.tflite'
    model_file: IO = open('./models/' + file_name, 'wb')
    model_file.write(tf_lite_model)
    model_file.close()

    print()
    print('################################################################################################')
    print('Model Performance:')
    print('Mean training accuracy:', round(sum(training_accuracy) / len(training_accuracy), 2))
    print('Mean validation accuracy:', round(sum(validation_accuracy) / len(validation_accuracy), 2))
    print('################################################################################################')
    print()
