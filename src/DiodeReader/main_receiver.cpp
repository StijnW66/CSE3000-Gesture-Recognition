#include<Arduino.h>
#include<inttypes.h>
#include <QuickMedianLib.h>
#include <SimpleTimer.h>

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
#define READ_PERIOD                     20
#define GESTURE_MIN_TIME_MS             100

uint16_t DETECTION_THRESHOLD = 750;
int count = 0;
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

SimpleTimer timer;
int timID;

void loop_main();

void setup() {

    pinMode(PD1, INPUT);
    pinMode(PD2, INPUT);

    Serial.begin(9600);

    timID = timer.setInterval(READ_PERIOD, loop_main);
}

void loop_main() {

    if(detectionWindowFull == false) {
         // If the detection window is not filled, fill it
         reader.read(PD1, photodiodeData1++);
         reader.read(PD2, photodiodeData2++);
         reader.read(PD1, taBuffer++);

         count++;

         if (count == DETECTION_WINDOW_LENGTH) {
             photodiodeData1 -= 1;
             photodiodeData2 -= 1;
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
    }
    
    // Try to detect a start on one of the photodiodes
    if (detectionWindowFull && (edgeDetector.DetectStart(photodiodeData2) || edgeDetector.DetectStart(photodiodeData1))) {

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

                // Flip the signal
                int index = 0;
                while(index++ < gestureSignalLength) {
                    photodiodeData[1][index] = abs(photodiodeData[1][index] - DETECTION_THRESHOLD);
                    photodiodeData[0][index] = abs(photodiodeData[0][index] - DETECTION_THRESHOLD);
                }

                // Normalize with the Z-score
                zScoreCalculator.ComputeZScore(photodiodeData[0], normPhotodiodeData[0], gestureSignalLength, true);
                zScoreCalculator.ComputeZScore(photodiodeData[1], normPhotodiodeData[1], gestureSignalLength, true);

                Serial.println("Start");

                // Send the signals
                for (int i = 0; i < gestureSignalLength; i++)
                {
                    Serial.print(normPhotodiodeData[0][i]);
                    Serial.print(" ");
                    Serial.println(0);
                }
                Serial.println("Done");
            
                break;
            }
        }

        // Reset the buffer pointers for PD gesture data collection
        detectionWindowFull = false;
        photodiodeData1 = photodiodeData[0];
        photodiodeData2 = photodiodeData[1];
        taBuffer = thresholdAdjustmentBuffer;
        count = 0;

        // Gesture took too long -> Light Intensity Change -> Threshold Recalculation
        if(!endDetected) {
            DETECTION_THRESHOLD = (QuickMedian<uint16_t>::GetMedian(photodiodeData[0], READING_WINDOW_LENGTH) * 4) / 5;
            edgeDetector.setThreshold(DETECTION_THRESHOLD);
        }

        timer.restartTimer(timID);
    }
}

void loop() {
    timer.run();
}