#include<Arduino.h>
#include<inttypes.h>

#include "SimplePhotoDiodeReader.h"
#include "SimpleGestureEdgeDetector.h"
#include "SimpleZScore.h"
#include "SimpleHampel.h"

#define DEBUG


#define PD1 A0
#define PD2 A1

#define DETECTION_WINDOW_LENGTH 10
#define READING_WINDOW_LENGTH 1000
#define DETECTION_THRESHOLD 700
#define READ_PERIOD 5

uint16_t detectionWindow[DETECTION_WINDOW_LENGTH];
uint16_t photodiodeData[2][READING_WINDOW_LENGTH];

SimpleZScore zScoreCalculator;
SimplePhotoDiodeReader reader;
SimpleGestureEdgeDetector edgeDetector(DETECTION_WINDOW_LENGTH, DETECTION_THRESHOLD);
SimpleHampel hampel(5);


void printSignal(uint16_t * signal, int length) {
    int count = 0;

    while(count++ < length) {
        int c = 0;
        while(c++ < signal[count])
            Serial.println("-");
    }
}


void setup() {

    pinMode(PD1, INPUT);
    pinMode(PD2, INPUT);

    Serial.begin(9600);
}


int test_hampel() {

    uint16_t signal[11];

    for (size_t i = 0; i < 11; i++)
    {
        signal[i] = i % 4;
    }
    
    long s = micros();

    hampel.filter1(signal);
    
    long e = micros();

    Serial.print("Time for Hampel on 11 samples: ");
    Serial.print(e - s);
    Serial.println(" micros");
}

void loop_main() {

    int count = DETECTION_WINDOW_LENGTH;
    int gestureSignalLength;
    uint16_t * photodiodeData1 = photodiodeData[0];
    uint16_t * photodiodeData2 = photodiodeData[1];

    while(count-- > 0) {
        reader.read(PD1, photodiodeData1++);
        reader.read(PD2, photodiodeData2++);

        delay(READ_PERIOD);
    }

    // long s = micros();
    // hampel.filter1(photodiodeData1);
    // long e = micros();

    // Serial.print("Hampel: ");
    // Serial.println(e - s);

    // int count2 = 0;
    // while(count2++ < DETECTION_WINDOW_LENGTH) {
    //         Serial.print(photodiodeData[0][count2]);
    //         Serial.print(" ");
    //         Serial.println(photodiodeData[1][count2]);
    // }

    if (edgeDetector.DetectStart(photodiodeData2 - 1)) {  
        Serial.println("Gesture Detected");

        bool endDetected = false;

        while(count++ < READING_WINDOW_LENGTH) {
            reader.read(PD2, photodiodeData2++);

            delay(READ_PERIOD);

            if(edgeDetector.DetectEnd(photodiodeData2 - 1)) {
                endDetected = true;

                // Determine the gesture length
                gestureSignalLength = photodiodeData2 - photodiodeData[1];

                Serial.print("End of Gesture Detected. Samples in the gesture ");
                Serial.println(photodiodeData2 - photodiodeData[1]);

                // Flip the signal
                int index = 0;
                while(index++ < gestureSignalLength) photodiodeData[1][index] = abs(photodiodeData[1][index] - DETECTION_THRESHOLD);

                // Normalize with the Z-score
                zScoreCalculator.ComputeZScore(photodiodeData[0], photodiodeData[0], gestureSignalLength);
                zScoreCalculator.ComputeZScore(photodiodeData[1], photodiodeData[1], gestureSignalLength);

                break;
            }
        }

        if(!endDetected) {
            Serial.println("Gesture took more samples then space allowed");
        }

    } else {
        Serial.println("Gesture Not Detected");
    }

    // Serial.print("A0: ");
    // Serial.println(analogRead(PD1));
    // Serial.print("A1: ");
    // Serial.println(analogRead(PD2));
    delay(READ_PERIOD);

}

void loop() {
    loop_main();

    // test_hampel();
    // delay(1000);
}