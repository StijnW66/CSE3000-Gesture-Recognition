#include "Arduino.h"
#include "diode_calibration/diode_calibration.h"
#include "mbed.h"

#include "receiver/receiver.hpp"
#include "ml-arduino/main_arduino.hpp"


rtos::Thread plotter_thread;
LightIntensityRegulator* regulator;

void plotter() {
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

void setup() {
  Serial.begin(9600);
  //while(!Serial);

  // Start visualization thread
  plotter_thread.start(plotter);

  // Setup the lightintensity regulator.
  regulator = new LightIntensityRegulator();

  tensorflowSetup();
  receiverSetup();
}


void loop() {
 

  receiverLoop();

}
