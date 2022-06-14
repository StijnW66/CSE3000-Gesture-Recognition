#ifndef ML_ARDUINO_DEBUG_UTILS_H_
#define ML_ARDUINO_DEBUG_UTILS_H_

#include "dummy_data.hpp"
#include "main_arduino.hpp"

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

/**
 * Print the average time taken to run an inference on a fixed sample
 * 
 * @param resolution The number of inferences to execute and base the average on
 * 
 */
void printTimeInference(uint32_t resolution) {
  unsigned long startTime = micros();
  for (uint32_t iteration = 0; iteration < resolution; iteration++) {inferGesture2d(ROT_CLOCKWISE_DUMMY_0);}
  unsigned long endTime = micros();
  unsigned long singleInferenceTime = (endTime - startTime) / resolution;
  
  char inferenceTimeStr[96];
  sprintf(inferenceTimeStr, "Time for single inference (based on %u samples): %u",
    resolution,
    endTime - startTime);
  Serial.println(inferenceTimeStr);
}

#endif
