#ifndef ML_ARDUINO_MAIN_H_
#define ML_ARDUINO_MAIN_H_

#include <TensorFlowLite.h>

#include "constants.hpp"
#include "model_quantization.h"
#include "prediction_enums.hpp"
#include "../DiodeReader/parameters.h"

#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"

// Globals, used for compatibility with Arduino-style sketches.
namespace {
tflite::ErrorReporter* error_reporter = nullptr;
const tflite::Model* model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;
TfLiteTensor* input = nullptr;
TfLiteTensor* output = nullptr;

constexpr size_t kTensorArenaSize = 14U * 1024U;
uint8_t tensor_arena[kTensorArenaSize];
}

void tensorflowSetup() {
  // Set up logging. Google style is to avoid globals or statics because of
  // lifetime uncertainty, but since this has a trivial destructor it's okay.
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

  // Pull in the operations needed for the 2D CNN
  // TODO: Remove extra operations (e.g. PadV2 and AveragePool2D) depending on final model structure
  static tflite::MicroMutableOpResolver<11> resolver;
  resolver.AddAveragePool2D();
  resolver.AddConv2D();
  resolver.AddDequantize();
  resolver.AddFullyConnected();
  resolver.AddMaxPool2D();
  resolver.AddPad();
  resolver.AddPadV2();
  resolver.AddQuantize();
  resolver.AddRelu();
  resolver.AddReshape();
  resolver.AddSoftmax();

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
  
  // Note that if a fully quantized model is used, the output needs to be quantized as well
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
 * @return An enum indicating which gesture was recognised (see prediction_enums.hpp)
 */
Gesture inferGesture2d(float signal[NUM_PDs][ML_DATA_LENGTH]) {
  // Re-order data to fit expected input shape (in effect, it's transposed)
  // Note that if a fully quantized model is used, the input needs to be quantized as well
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

#endif
