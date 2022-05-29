#include "Arduino.h"
#include "diode_calibration/diode_calibration.h"
#include "mbed.h"


rtos::Thread calibration_thread;

void setup() {
  // Initialise the serial port
  Serial.begin(19200);
  Serial1.begin(19200);
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  while(!Serial1);
  calibration_thread.start(calibrate_diode_setup); // Should not be done multithreaded but is good for keeping the visualization.
}

void loop() {
  Serial.println((String)analogRead(A0) + "  " + analogRead(A1) + "  " + analogRead(A2));
  Serial1.println((String)analogRead(A0) + "  " + analogRead(A1) + "  " + analogRead(A2));
  delay(50);
}