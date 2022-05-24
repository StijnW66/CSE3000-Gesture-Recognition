#include "DiodeReader/main_receiver.hpp"
#include "ml-arduino/main_arduino.hpp"

// The name of this function is important for Arduino compatibility.
void setup() {
  tensorflowSetup();
  receiverSetup();
}

// The name of this function is important for Arduino compatibility.
void loop() {
  receiverLoop();

  // TODO: Remove
  // printTensorDimensionsToSerial();
  // printDummyDataInferenceResults();
}
