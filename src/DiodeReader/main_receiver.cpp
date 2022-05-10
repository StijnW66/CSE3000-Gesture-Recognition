#include<Arduino.h>
#include<inttypes.h>
#include <QuickMedianLib.h>
#include <TimerInterrupt_Generic.h>

#include "SimplePhotoDiodeReader.h"
#include "SimpleGestureEdgeDetector.h"
#include "SimpleZScore.h"
#include "SimpleHampel.h"

// #define DEBUG
// #define SEND_PLOT


#define PD1 A0
#define PD2 A1

#define DETECTION_WINDOW_LENGTH         5
#define DETECTION_END_WINDOW_LENGTH     10
#define READING_WINDOW_LENGTH           250
#define THRESHOLD_ADJUSTMENT_LENGTH     100
#define READ_PERIOD                     30
#define GESTURE_MIN_TIME_MS             100

uint16_t DETECTION_THRESHOLD = 750;
int count = DETECTION_WINDOW_LENGTH;
bool detectionWindowFull = false;

uint16_t thresholdAdjustmentBuffer[THRESHOLD_ADJUSTMENT_LENGTH];
uint16_t * taBuffer = thresholdAdjustmentBuffer;

uint16_t photodiodeData[2][READING_WINDOW_LENGTH];
uint16_t * photodiodeData1 = photodiodeData[0];
uint16_t * photodiodeData2 = photodiodeData[1];
int gestureSignalLength;

float normPhotodiodeData[2][READING_WINDOW_LENGTH];

SimpleZScore zScoreCalculator;
SimplePhotoDiodeReader reader;
SimpleGestureEdgeDetector edgeDetector(DETECTION_WINDOW_LENGTH, DETECTION_END_WINDOW_LENGTH, DETECTION_THRESHOLD);
SimpleHampel hampel(5);

NRF52_MBED_Timer ITimer1(NRF_TIMER_2);

void printSignal(uint16_t * signal, int length) {
    int count = 0;

    while(count++ < length) {
        Serial.print(signal[count]);
        Serial.print(" ");
    }
    Serial.println();
}

void printSignalF(float * signal, int length) {
    int count = 0;

    while(count++ < length) {
        Serial.print(signal[count]);
        Serial.print(" ");
    }
    Serial.println();
}


void setup() {

    pinMode(PD1, INPUT);
    pinMode(PD2, INPUT);

    Serial.begin(9600);

    ITimer1.attachInterruptInterval(READ_PERIOD * 1000, loop_main);
    ITimer1.restartTimer();
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

    if(detectionWindowFull == false) {
         // If the detection window is not filled, fill it
         reader.read(PD1, photodiodeData1++);
         reader.read(PD2, photodiodeData2++);
         reader.read(PD1, taBuffer++);

         if (--count == 0) {
             photodiodeData1 -= 1;
             photodiodeData2 -= 1;
             count = DETECTION_WINDOW_LENGTH;
             detectionWindowFull = true;
         }

    } else {
        // If the detection window is already filled, shift it left with 1
        // and put the new sample in the last place
        for (size_t i = 0; i < DETECTION_WINDOW_LENGTH - 1; i++)
        {
            photodiodeData[0][i] = photodiodeData[0][i+1];
            photodiodeData[1][i] = photodiodeData[1][i+1];
        }

        reader.read(PD1, photodiodeData1);
        reader.read(PD2, photodiodeData2);
        reader.read(PD1, taBuffer++);
    }

    // If there was no gesture recently, update the threshold
    if (taBuffer - thresholdAdjustmentBuffer >= THRESHOLD_ADJUSTMENT_LENGTH) {
        DETECTION_THRESHOLD = (QuickMedian<uint16_t>::GetMedian(thresholdAdjustmentBuffer, THRESHOLD_ADJUSTMENT_LENGTH) * 4) / 5 ;
        edgeDetector.setThreshold(DETECTION_THRESHOLD);
        taBuffer = thresholdAdjustmentBuffer;

#ifdef DEBUG        
        Serial.print("New threshold is "); 
        Serial.println(DETECTION_THRESHOLD);
#endif

    }

    // Try to detect a start on one of the photodiodes
    if (detectionWindowFull && (edgeDetector.DetectStart(photodiodeData2 - 1) || edgeDetector.DetectStart(photodiodeData1 - 1))) {

        ITimer1.stopTimer();

#ifdef DEBUG        
        Serial.println("Gesture Detected");
        printSignal(photodiodeData[0], photodiodeData1 - photodiodeData[0]);
        Serial.println("------------");
        printSignal(photodiodeData[1], photodiodeData2 - photodiodeData[1]);
#endif

        bool endDetected = false;

        // Read enough more data to avoid buffer overflow when checking end
        // of gesture if more samples are checked for end than for start
        while(count++ < DETECTION_END_WINDOW_LENGTH - DETECTION_WINDOW_LENGTH) {
            reader.read(PD2, photodiodeData2++);
            reader.read(PD1, photodiodeData1++);
            delay(READ_PERIOD);
        }

        // Read new data and check for end of gesture
        while(count++ < READING_WINDOW_LENGTH) {
            reader.read(PD2, photodiodeData2++);
            reader.read(PD1, photodiodeData1++);

            delay(READ_PERIOD);

            if(edgeDetector.DetectEnd(photodiodeData2 - 1) && edgeDetector.DetectEnd(photodiodeData1 - 1)) {

                // Determine the gesture length
                gestureSignalLength = photodiodeData2 - photodiodeData[1];

                // Reject gestures that took too short time
                if (gestureSignalLength < GESTURE_MIN_TIME_MS / READ_PERIOD + 1) {
                    Serial.println("Gesture took too little time! Rejecting and starting over ...");
                    break;
                }
                else endDetected = true;

#ifdef DEBUG
                Serial.print("End of Gesture Detected. Samples in the gesture ");
                Serial.println(photodiodeData2 - photodiodeData[1]);
#endif

                // Flip the signal
                int index = 0;
                while(index++ < gestureSignalLength) {
                    photodiodeData[1][index] = abs(photodiodeData[1][index] - DETECTION_THRESHOLD);
                    photodiodeData[0][index] = abs(photodiodeData[0][index] - DETECTION_THRESHOLD);
                }

                // Normalize with the Z-score
                zScoreCalculator.ComputeZScore(photodiodeData[0], normPhotodiodeData[0], gestureSignalLength, true);
                zScoreCalculator.ComputeZScore(photodiodeData[1], normPhotodiodeData[1], gestureSignalLength, true);

#ifdef DEBUG
                Serial.println("Gesture data after Z-score normalization: ");
                printSignalF(normPhotodiodeData[0], gestureSignalLength);
                printSignalF(normPhotodiodeData[1], gestureSignalLength);
                Serial.println("-------------------------------------");
#endif

#ifdef SEND_PLOT
                Serial.println("Start");
                for (size_t i = 0; i < gestureSignalLength; i++)
                {
                    Serial.print(normPhotodiodeData[0][i]);
                    Serial.print(" ");
                    Serial.println(normPhotodiodeData[1][i]);
                }
                Serial.println("Done");
#endif

                break;
            }
        }

        // Reset the buffer pointers for PD gesture data collection
        detectionWindowFull = false;
        photodiodeData1 = photodiodeData[0];
        photodiodeData2 = photodiodeData[1];
        taBuffer = thresholdAdjustmentBuffer;

        // Gesture took too long -> Light Intensity Change -> Threshold Recalculation
        if(!endDetected) {
            DETECTION_THRESHOLD = (QuickMedian<uint16_t>::GetMedian(photodiodeData[0], READING_WINDOW_LENGTH) * 4) / 5;
            edgeDetector.setThreshold(DETECTION_THRESHOLD);

#ifdef DEBUG
            Serial.println("Gesture took more samples then space allowed. Try recalculating the threshold ...");
            Serial.print("New threshold is "); 
            Serial.println(DETECTION_THRESHOLD);
#endif
        }

        ITimer1.restartTimer();
    }
}

void utilPrintPhotoDiodes() {
    Serial.print("A0: ");
    Serial.println(analogRead(PD1));
    Serial.print("A1: ");
    Serial.println(analogRead(PD2));
    delay(READ_PERIOD);
}

void utilTestMedianFinder() {
    uint16_t data[READING_WINDOW_LENGTH];

    for (size_t i = 0; i < READING_WINDOW_LENGTH; i++)
    {
        data[i] = analogRead(PD1);
        delay(1);
    }

    long s = micros();
    int t = QuickMedian<uint16_t>::GetMedian(data, READING_WINDOW_LENGTH);
    long e = micros();

    Serial.print("Median took for ");
    Serial.print(READING_WINDOW_LENGTH);
    Serial.print(" : ");
    Serial.println(e - s);

    delay(1000);
}

void loop() {
    // loop_main();

    // utilTestMedianFinder();

    // utilPrintPhotoDiodes();
}