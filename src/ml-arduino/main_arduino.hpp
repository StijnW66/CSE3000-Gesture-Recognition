#ifndef ML_ARDUINO_MAIN_H_
#define ML_ARDUINO_MAIN_H_

#include <TensorFlowLite.h>

#include "constants.hpp"
#include "dummy_data.hpp"
#include "model_quantization.h"
#include "prediction_enums.hpp"
#include "../DiodeReader/parameters.h"

#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"

// Globals, used for compatibility with Arduino-style sketches.
namespace {
tflite::ErrorReporter* error_reporter = nullptr;
const tflite::Model* model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;
TfLiteTensor* input = nullptr;
TfLiteTensor* output = nullptr;

constexpr size_t kTensorArenaSize = 15U * 1024U;
uint8_t tensor_arena[kTensorArenaSize];
}  // namespace

void tensorflowSetup() {
  // Set up logging. Google style is to avoid globals or statics because of
  // lifetime uncertainty, but since this has a trivial destructor it's okay.
  // NOLINTNEXTLINE(runtime-global-variables)
  static tflite::MicroErrorReporter micro_error_reporter;
  error_reporter = &micro_error_reporter;

  // Map the model into a usable data structure. This doesn't involve any
  // copying or parsing, it's a very lightweight operation.
  model = tflite::GetModel(model_quantization_tflite);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    TF_LITE_REPORT_ERROR(error_reporter,
                          "Model provided is schema version %d not equal "
                          "to supported version %d.",
                          model->version(), TFLITE_SCHEMA_VERSION);
    return;
  }

  // This pulls in all the operation implementations we need.
  static tflite::AllOpsResolver resolver;

  // Build an interpreter to run the model with.
  static tflite::MicroInterpreter static_interpreter(
      model, resolver, tensor_arena, kTensorArenaSize, error_reporter);
  interpreter = &static_interpreter;

  // Allocate memory from the tensor_arena for the model's tensors.
  TfLiteStatus allocate_status = interpreter->AllocateTensors();
  if (allocate_status != kTfLiteOk) {
    TF_LITE_REPORT_ERROR(error_reporter, "AllocateTensors() failed");
    return;
  }

  // Obtain pointers to the model's input and output tensors.
  input = interpreter->input(0);
  output = interpreter->output(0);  
}

/**
 * Computes which gesture has the highest inferred probability.
 * Should only be called after an interpreter->Invoke() call.
 * 
 * @return The enum value of the gesture with the highest class probability
 */
static Gesture gestureHighestProbability() {
  Gesture majority_gesture = UNIDENTIFIED;
  float majority_probability = -1.0f;
  Gesture const all_gestures[] = {SWIPE_UP, SWIPE_DOWN, SWIPE_LEFT, SWIPE_RIGHT,
                                  TAP_SINGLE, TAP_DOUBLE,
                                  ROTATE_CLOCKWISE, ROTATE_COUNTERCLOCKWISE};
  
  for (Gesture current_gesture : all_gestures) {
    float current_probability = output->data.f[current_gesture];
    if (current_probability > majority_probability) {
      majority_probability = current_probability;
      majority_gesture = current_gesture;
    }
  }
  return majority_gesture;
}

/**
 * Attempt to classify given photodiode readings as one of the supported gestures.
 * 
 * @param signal A 2D array where each row represents the readings of a single photodiode
 * over time. Expected dimensions are (NUM_PDs x ML_DATA_LENGTH), see DiodeReader/parameters.h
 * 
 */
Gesture inferGesture2d(float signal[NUM_PDs][ML_DATA_LENGTH]) {
  // Re-order data to fit expected input shape (in effect, it's transposed)
  // Note that if a fully quantized model is used, input and output need to be quantized as well
  size_t current_byte = 0;
  for (size_t row = 0; row < kNumRows; row++) {
    for (size_t col = 0; col < kNumCols; col++) {
      input->data.f[current_byte++] = signal[col][row];
    }
  }

  // Run inference and report any error
  TfLiteStatus invoke_status = interpreter->Invoke();
  if (invoke_status != kTfLiteOk) {
    TF_LITE_REPORT_ERROR(error_reporter, "Invocation failed");
    return UNIDENTIFIED;
  }
  
  return gestureHighestProbability();
}

/**
 * Print the dimensions of the input and output tensors to serial.
 */
void printTensorDimensionsToSerial() {
  char input_prop_str[96];
  sprintf(input_prop_str, "Input dimensions: %d - (%d, %d, %d, %d)",
    input->dims->size,
    input->dims->data[0],
    input->dims->data[1],
    input->dims->data[2],
    input->dims->data[3]);
  Serial.println(input_prop_str);
  char output_prop_str[96];
  sprintf(output_prop_str, "Output dimensions: %d - (%d, %d)",
    output->dims->size,
    output->dims->data[0],
    output->dims->data[1]);
  Serial.println(output_prop_str);
}

/**
 * Print the results of running the CNN on dummy data (see dummy_data.hpp)
 */
void printDummyDataInferenceResults() {
  Gesture rot_clockwise_inference_0 = inferGesture2d(ROT_CLOCKWISE_DUMMY_0);
  Gesture rot_clockwise_inference_1 = inferGesture2d(ROT_CLOCKWISE_DUMMY_1);
  Gesture swipe_up_inference_0 = inferGesture2d(SWIPE_UP_DUMMY_0);

  char inference_results_str[32];
  sprintf(inference_results_str, "(%d, %d, %d)",
    rot_clockwise_inference_0,
    rot_clockwise_inference_1,
    swipe_up_inference_0);
  Serial.println(inference_results_str);
}

#endif
