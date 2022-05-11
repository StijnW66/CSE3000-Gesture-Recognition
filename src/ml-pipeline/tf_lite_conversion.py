from runner import compile_model, train_and_evaluate
import data_processing
import models
import numpy as np
import pandas as pd
import tensorflow as tf

representative_dataset = None # TODO: Devise less hacky solution

def _convert_no_optimisations(model: tf.keras.Model):
    """
    Convert a Keras model to a TensorflowLite model without applying any additional
    optimisations and save it to a file.

    Args:
        model: The model to be converted
    """
    converter = tf.lite.TFLiteConverter.from_keras_model(model)
    tflite_model = converter.convert()
    open("model_no_optimisations.tflite", "wb").write(tflite_model)


def _convert_with_quantization(model: tf.keras.Model, dataset: tf.data.Dataset):
    """
    Convert a Keras model to a TensorflowLite model, applying default optimisations
    and quantization, and save it to a file.

    Args:
        model: The model to be converted
        dataset: The dataset used to test and/or train the model. Used to infer
        representative input values for quantization
    """
    converter = tf.lite.TFLiteConverter.from_keras_model(model)
    converter.optimizations = [tf.lite.Optimize.DEFAULT]

    # Create representative values for quantization
    representative_dataset = dataset
    def representative_dataset_generator():
        for value in representative_dataset:
            yield [np.array(value, dtype=np.float32, ndmin=4)]
    converter.representative_dataset = representative_dataset_generator

    tflite_model = converter.convert()
    open("model_quantization.tflite", "wb").write(tflite_model)


def evaluate_tflite_model(file_path: str,
                          model: tf.keras.Model,
                          test_dataset: tf.data.Dataset):
    tflite_interpreter = tf.lite.Interpreter(model_path=file_path)
    input_details = tflite_interpreter.get_input_details()
    output_details = tflite_interpreter.get_output_details()

    # Resize I/O tensors to be able to process BATCH_SIZE batches of data
    tflite_interpreter.resize_tensor_input(
        input_details[0]['index'],
        (data_processing.BATCH_SIZE, 315, 3, 1))
    tflite_interpreter.resize_tensor_input(
        output_details[0]['index'],
        (data_processing.BATCH_SIZE, data_processing.NUM_CLASSES_UWAVE))
    tflite_interpreter.allocate_tensors()

    # Feed first test batch to input tensor
    first_batch_x = np.array(list(test_dataset.as_numpy_iterator())[0][0], dtype=np.float32, ndmin=4)
    input_details = tflite_interpreter.get_input_details()
    tflite_interpreter.set_tensor(input_details[0]['index'], first_batch_x)

    tflite_interpreter.invoke()

    # Extract prediction values
    output_details = tflite_interpreter.get_output_details()
    tflite_model_predictions = tflite_interpreter.get_tensor(output_details[0]['index'])
    
    # Visual inspection because I'm lazy (TODO: Not this)
    print(f"===== PREDICTIONS FOR MODEL {file_path} =====")
    pred_dataframe = pd.DataFrame(tflite_model_predictions)
    print(pred_dataframe.to_string())
    print("==============================================")


if __name__ == "__main__":
    features, labels = data_processing.load_and_combine_uwave()
    features, labels = data_processing.preprocess_input(features, labels)
    train_dataset, test_dataset = data_processing.split_to_tf_datasets(features, labels)
    running_model = models.slam_cnn_padding(features[0].shape, data_processing.NUM_CLASSES_UWAVE)

    compile_model(running_model)
    train_and_evaluate(running_model, train_dataset, test_dataset)

    _convert_no_optimisations(running_model)
    _convert_with_quantization(running_model, features)

    # Visual inspection of results of inferring the first batch of test data
    evaluate_tflite_model("model_no_optimisations.tflite", running_model, test_dataset)
    evaluate_tflite_model("model_quantization.tflite", running_model, test_dataset)
    print("===== TRUE LABELS =====")
    labels_dataframe = pd.DataFrame(list(test_dataset.as_numpy_iterator())[0][1])
    print(labels_dataframe.to_string())
