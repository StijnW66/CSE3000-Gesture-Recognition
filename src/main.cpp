#include <Arduino.h>

void setup() {
  // Initialise the serial port
  Serial.begin(19200);
  Serial1.begin(19200);
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
}

void loop() {
  Serial.println((String)analogRead(A0) + "  " + analogRead(A1) + "  " + analogRead(A2));
  Serial1.println((String)analogRead(A0) + "  " + analogRead(A1) + "  " + analogRead(A2));
  delay(50);

}