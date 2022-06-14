# 2D ML Arduino Code
This directory contains the code needed to run the 2D CNN on an Arduino Nano 33 BLE. It makes use of Tensorflow Lite for Microcontrollers, specifically the [Arduino_TensorFlowLite library](https://www.arduino.cc/reference/en/libraries/arduino_tensorflowlite/).

## Code Structure
- `constants.hpp`: Contains constants used for operation. Generally should not be modified as the values reflect the structure of the project's dataset
- `debug_utils.hpp`: Miscellaneous utilities for debugging TFLite operation. Should not be included except during testing
- `dummy_data.hpp`: Hard-coded data from the dataset. Used for verifying model functionality and obtaining metrics (e.g. inference latency)
- `main_arduino.hpp`: The actual meat of the code. Contains a setup function `tensorflowSetup()` which should be called in the `setup()` function prior to inference. The function `inferGesture2d()` performs the actual inference.
- `model_quantization.cpp`: The TFLite model as a hex dumped C array. Produced using the `xxd` CLI tool and the models's corresponding `.tflite` file
- `model_quantization.h`: The header file corresponding to `model_quantization.cpp`
- `prediction_enums.hpp`: Defines an `enum` corresponding to each of the gestures in the dataset

## Usage
Include `main_arduino.hpp` (and optionally `debug_utils.hpp`) in the `main.cpp` to be flashed to an Arduino Nano 33 BLE. Call `tensorflowSetup()` in the `setup()` function and call `inferGesture2d()` with processed photodiode data to run the model and produce an inference.
