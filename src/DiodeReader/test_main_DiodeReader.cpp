#include <Arduino.h>
#include <DiodeReader.h>

DiodeReader reader(3, 200, A0, A1);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);

}

void loop_test_diodeReadingSpeed() {
  
  int count = 1000;
  
  long start = micros(), end;

  while(count-- > 0) {
    int reading = analogRead(A0);
  }

  end = micros();

  Serial.print(end - start);
  Serial.println(" microseconds for 1000 readings.");

}

void loop() {
    // reader.readAndUpdateDebug(Serial);
    // delay(1000);
    loop_test_diodeReadingSpeed();
    delay(1000);
}