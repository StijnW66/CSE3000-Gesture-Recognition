# Real-time Photodiode (PD) - based Gesture Receiver

## Directory Structure
- [receiver.hpp](receiver.hpp) : receiver main file, containing its FSM architecture

- [receiver-parameters.h](receiver-parameters.h) : various macros and photodiode pins setups for use by the rest of the files   

- [receiver-util.h](receiver-util.h) : auxiliary utility macros and functions

- [GRDiodeReader.h](GRDiodeReader.h) : abstraction over PD reading. Used in [receiver.hpp](receiver.hpp).

- [GREdgeDetector.h](GREdgeDetector.h) : abstraction over checking for gesture start and end using an adjustable threshold. Used in [receiver.hpp](receiver.hpp).

- [GRPreprocessingPipeline.h](GRPreprocessingPipeline.h) + [pipeline-stages/](pipeline-stages/) : pipeline for noise reduction and normalisation of gesture data. Used in [receiver.hpp](receiver.hpp).

- [plotting/](plotting/) : auxiliary files for plotting the output of the preprocessing pipeline stages.


## Plotting the receiver pipeline stages' outputs:

* Go to `receiver-parameters.h` and uncomment the `PLOT_RECEIVER` macro.
* After starting the system on the Arduino connected via a serial port, run the Python script [plotter.py](plotting/plotter.py).

## Debugging the receiver main FSM:

* Go to `receiver-parameters.h` and uncomment the `DEBUG_RECEIVER` macro.
* Start the system on the Arduino connected via a serial port and monitor the port input.

## Turning off hardware adjustment and only using software threshold adjustment

* Go to `receiver-parameters.h` and uncomment the `NO_HARDWARE_ADJUSTMENT` macro.

