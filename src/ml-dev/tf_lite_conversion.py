from typing import Tuple
from sklearn.metrics import accuracy_score
import constants
import data_processing
import models
import numpy as np
import tensorflow as tf
import tensorflow_model_optimization as tfmot

representative_dataset = None # TODO: Devise less hacky solution

def _convert_no_optimisations(model: tf.keras.Model, write_to_file: bool = True):
    """
    Convert a Keras model to a TensorflowLite model without applying any additional
    optimisations and (optionally) save it to a file.

    Args:
        model: The (trained) model to be converted
        write_to_file: Whether to save the converted model to a file

    Returns:
        The converted TFLite model in serialised format
    """
    converter = tf.lite.TFLiteConverter.from_keras_model(model)
    tflite_model = converter.convert()
    if write_to_file:
        open("model_no_optimisations.tflite", "wb").write(tflite_model)
    return tflite_model


def _convert_with_quantization(model: tf.keras.Model, dataset: np.ndarray,
                               write_to_file: bool = True):
    """
    Convert a Keras model to a TensorflowLite model, applying default optimisations
    and quantization (incl. I/O quantization), and (optionally) save it to a file.

    Args:
        model: The (trained) model to be converted
        dataset: The dataset used to test and/or train the model. Used to infer
        representative input values for quantization
        write_to_file: Whether to save the converted model to a file

    Returns:
        The converted TFLite model in serialised format
    """
    converter = tf.lite.TFLiteConverter.from_keras_model(model)
    converter.optimizations = [tf.lite.Optimize.DEFAULT]
    converter.target_spec.supported_ops = [tf.lite.OpsSet.TFLITE_BUILTINS_INT8]
    converter.inference_input_type = tf.uint8
    converter.inference_output_type = tf.uint8

    # Create representative values for quantization
    representative_dataset = dataset
    def representative_dataset_generator():
        for value in representative_dataset:
            yield [np.array(value, dtype=np.float32, ndmin=4)]
    converter.representative_dataset = representative_dataset_generator

    tflite_model = converter.convert()
    if write_to_file:
        open("model_quantization.tflite", "wb").write(tflite_model)
    return tflite_model


def _convert_with_quantization_aware_training(model: tf.keras.Model,
                                              train_dataset: tf.data.Dataset,
                                              test_dataset: tf.data.Dataset,
                                              write_to_file: bool = True):
    """
    Convert a Keras model to a quantization-aware model, train and evaluate it
    (applying default optimisation), and (optionally) save it to a file.

    Args:
        model: The model to be converted. Can be trained or untrained
        train_dataset: The dataset to use for training the model
        test_dataset: The dataset to use for evaluating the model
        write_to_file: Whether to save the converted model to a file

    Returns:
        The converted TFLite model in serialised format
    """
    from runner import compile_model, train_and_evaluate_tf

    # Compile, train and evaluate
    quantization_aware_model = tfmot.quantization.keras.quantize_model(model)
    compile_model(quantization_aware_model)
    train_and_evaluate_tf(quantization_aware_model, train_dataset, test_dataset)

    # Convert and (optionally) write to file
    converter = tf.lite.TFLiteConverter.from_keras_model(quantization_aware_model)
    converter.optimizations = [tf.lite.Optimize.DEFAULT]
    tflite_model = converter.convert()
    if write_to_file:
        open("model_quantization_aware_training.tflite", "wb").write(tflite_model)
    return tflite_model


def evaluate_tflite_model(file_path: str, model: tf.keras.Model,
                          test_dataset: tf.data.Dataset,
                          num_classes: int, input_shape: Tuple):
    """
    Print the accuracy of predictions made by a base model and its converted TFLite
    equivalent to assess any losses incurred by optimisations.

    Args:
        file_path: Path to the .tflite file containing the converted TFLite model 
        model: Base model from which the converted model was constructed
        test_dataset: Dataset to use for comparing classification performance
        num_classes: Number of classification classes
        input_shape: Shape of the input that the model expects
    """
    # Re-obtain test samples as np array and labels as list for easier processing
    test_dataset = test_dataset.unbatch()
    test_dataset_arr = list(test_dataset)
    test_dataset_samples = list(map(lambda sample_label_pair: sample_label_pair[0], test_dataset_arr))
    test_dataset_samples = np.array(test_dataset_samples, dtype=np.float32)
    test_dataset_labels = list(map(lambda sample_label_pair: sample_label_pair[1], test_dataset_arr))

    # Initialise TFLite interpreter and acquire I/O tensor handles
    tflite_interpreter = tf.lite.Interpreter(model_path=file_path)
    input_details = tflite_interpreter.get_input_details()
    output_details = tflite_interpreter.get_output_details()

    # Resize I/O tensors to be able to process test dataset
    tflite_interpreter.resize_tensor_input(input_details[0]['index'], (len(test_dataset_arr),) + input_shape)
    tflite_interpreter.resize_tensor_input(output_details[0]['index'],(len(test_dataset_arr), num_classes))
    tflite_interpreter.allocate_tensors()

    # Create input appropriate for TFLite model
    test_dataset_samples_tflite = test_dataset_samples.copy()
    if input_details[0]['dtype'] == np.uint8:
      input_scale, input_zero_point = input_details[0]["quantization"]
      test_dataset_samples_tflite = test_dataset_samples / input_scale + input_zero_point
    test_dataset_samples_tflite = test_dataset_samples_tflite.astype(input_details[0]["dtype"])

    # Feed test data to input tensor
    input_details = tflite_interpreter.get_input_details()
    tflite_interpreter.set_tensor(input_details[0]['index'], test_dataset_samples_tflite)

    # Run predictions on TFLite and base model
    tflite_interpreter.invoke()
    base_model_class_probabilities = model.predict(test_dataset_samples)

    # Extract class probabilities and convert to predictions
    output_details = tflite_interpreter.get_output_details()
    tflite_class_probabilities = tflite_interpreter.get_tensor(output_details[0]['index'])
    tflite_predictions = np.argmax(tflite_class_probabilities, axis=1)
    base_model_predictions = np.argmax(base_model_class_probabilities, axis=1)

    print(f"Base model accuracy: {accuracy_score(test_dataset_labels, base_model_predictions)}")
    print(f"TFLite model accuracy: {accuracy_score(test_dataset_labels, tflite_predictions)}")


if __name__ == "__main__":
    features, labels = data_processing.load_and_combine_data()
    train_dataset, test_dataset = data_processing.split_to_tf_datasets(features, labels)
    running_model = models.slam_cnn_padding_pyramid_lite(
        features[0].shape,
        constants.NUM_CLASSES_RAW_DATA)

    from runner import compile_model, train_and_evaluate_tf
    compile_model(running_model)
    train_and_evaluate_tf(running_model, train_dataset, test_dataset)

    _convert_no_optimisations(running_model)
    _convert_with_quantization(running_model, features)
    # _convert_with_quantization_aware_training(running_model, train_dataset, test_dataset)

    evaluate_tflite_model(
        "model_no_optimisations.tflite",
        running_model,
        test_dataset,
        constants.NUM_CLASSES_RAW_DATA,
        features[0].shape)
    evaluate_tflite_model(
        "model_quantization.tflite",
        running_model,
        test_dataset,
        constants.NUM_CLASSES_RAW_DATA,
        features[0].shape)
    # evaluate_tflite_model(
    #     "model_quantization_aware_training.tflite",
    #     running_model,
    #     test_dataset,
    #     constants.NUM_CLASSES_INITIAL_RAW_DATA,
    #     features[0].shape)
