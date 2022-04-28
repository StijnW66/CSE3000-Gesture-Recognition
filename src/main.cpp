#include "Arduino.h"
#include "diode_calibration/diode_calibration.h"
#include "mbed.h"

using namespace mbed;
using namespace rtos;

Thread calibration_thread;

void setup() {
  Serial.begin(9600);
  while(!Serial);
  calibration_thread.start(calibrate_diode_setup); // Should not be done multithreaded but is good for keeping the visualization.
}

void loop() {
  int r0 = analogRead(A0);
  int r1 = analogRead(A1);
  int r2 = analogRead(A2);

  Serial.print(r0);
  Serial.print(", ");
  Serial.print(r1);
  Serial.print(", ");
  Serial.println(r2);
  delay(10);
}