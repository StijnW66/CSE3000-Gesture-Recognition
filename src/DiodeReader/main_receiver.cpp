#include<Arduino.h>
#include<inttypes.h>
#include <QuickMedianLib.h>
#include <SimpleTimer.h>

#include "SimplePhotoDiodeReader.h"
#include "SimpleGestureEdgeDetector.h"

#include "parameters.h"
#include "pipeline.h"

#define PD1 A0
#define PD2 A1

int count = 0;
bool detectionWindowFull = false;

uint16_t thresholdAdjustmentBuffer[2][THRESHOLD_ADJ_BUFFER_LENGTH];
uint16_t * taBuffer1 = thresholdAdjustmentBuffer[0];
uint16_t * taBuffer2 = thresholdAdjustmentBuffer[1];

uint16_t photodiodeData[2][READING_WINDOW_LENGTH];
uint16_t * photodiodeData1 = photodiodeData[0];
uint16_t * photodiodeData2 = photodiodeData[1];
int gestureSignalLength;

SimplePhotoDiodeReader reader;
SimpleGestureEdgeDetector edgeDetector1(DETECTION_WINDOW_LENGTH, DETECTION_END_WINDOW_LENGTH, 750);
SimpleGestureEdgeDetector edgeDetector2(DETECTION_WINDOW_LENGTH, DETECTION_END_WINDOW_LENGTH, 750);

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
         reader.read(PD1, taBuffer1++);
         reader.read(PD2, taBuffer2++);

         count++;

         if (count == DETECTION_BUFFER_LENGTH) {
             photodiodeData1 -= 1;
             photodiodeData2 -= 1;
             detectionWindowFull = true;
         }

    } else {

        // If the detection window is already filled, shift it left with 1
        // and put the new sample in the last place
        for (size_t i = 0; i < DETECTION_BUFFER_LENGTH - 1; i++)
        {
            photodiodeData[0][i] = photodiodeData[0][i+1];
            photodiodeData[1][i] = photodiodeData[1][i+1];
        }

        reader.read(PD1, photodiodeData1);
        reader.read(PD2, photodiodeData2);
        reader.read(PD1, taBuffer1++);
        reader.read(PD2, taBuffer2++);
    }

    // If there was no gesture recently, update the threshold
    if (taBuffer1 - thresholdAdjustmentBuffer[0] >= THRESHOLD_ADJ_BUFFER_LENGTH) {
        
        edgeDetector1.setThreshold((QuickMedian<uint16_t>::GetMedian(thresholdAdjustmentBuffer[0], THRESHOLD_ADJ_BUFFER_LENGTH) * 4) / 5);
        edgeDetector2.setThreshold((QuickMedian<uint16_t>::GetMedian(thresholdAdjustmentBuffer[1], THRESHOLD_ADJ_BUFFER_LENGTH) * 4) / 5);
        taBuffer1 = thresholdAdjustmentBuffer[0];
        taBuffer2 = thresholdAdjustmentBuffer[1];
    }
    
    // Try to detect a start on one of the photodiodes
    if (detectionWindowFull && (edgeDetector2.DetectStart(photodiodeData2) || edgeDetector1.DetectStart(photodiodeData1))) {

        bool endDetected = false;

        // Read enough more data to avoid buffer overflow when checking end
        // of gesture if more samples are checked for end than for start
        while(count++ < DETECTION_END_WINDOW_LENGTH - DETECTION_BUFFER_LENGTH) {
            reader.read(PD2, ++photodiodeData2);
            reader.read(PD1, ++photodiodeData1);
            delay(READ_PERIOD);
        }

        // Read new data and check for end of gesture
        while(count++ < READING_WINDOW_LENGTH) {
            reader.read(PD2, ++photodiodeData2);
            reader.read(PD1, ++photodiodeData1);

            delay(READ_PERIOD);

            if(edgeDetector2.DetectEnd(photodiodeData2) && edgeDetector1.DetectEnd(photodiodeData1)) {

                // Determine the gesture length
                gestureSignalLength = photodiodeData1 - photodiodeData[0] + 1;

                // Reject gestures that took too short time
                if (gestureSignalLength < GESTURE_MIN_TIME_MS / READ_PERIOD + 1) {
                    Serial.println("Gesture took too little time! Rejecting and starting over ...");
                    break;
                }
                else endDetected = true;

// ---------------------------------------------------
                // Flip the signal
                int index = -1;
                while(++index < gestureSignalLength) {
                    photodiodeData[1][index] = max(edgeDetector2.getThreshold() - photodiodeData[1][index], 0);
                    photodiodeData[0][index] = max(edgeDetector1.getThreshold() - photodiodeData[0][index], 0);
                }

                Serial.println("Start");
                for (int i = 0; i < gestureSignalLength; i++)
                {
                    Serial.print(photodiodeData[0][i]);
                    Serial.print(" ");
                    Serial.println(photodiodeData[1][i]);
                }
                Serial.println("Done");
// ----------------------------------------

                RunPipeline(photodiodeData, gestureSignalLength);
                
                break;
            }
        }

        // Reset the buffer pointers for PD gesture data collection
        detectionWindowFull = false;
        photodiodeData1 = photodiodeData[0];
        photodiodeData2 = photodiodeData[1];
        taBuffer1 = thresholdAdjustmentBuffer[0];
        taBuffer2 = thresholdAdjustmentBuffer[1];
        count = 0;

        // Gesture took too long -> Light Intensity Change -> Threshold Recalculation
        if(!endDetected) {
            edgeDetector1.setThreshold((QuickMedian<uint16_t>::GetMedian(photodiodeData[0], READING_WINDOW_LENGTH) * 4) / 5);
            edgeDetector2.setThreshold((QuickMedian<uint16_t>::GetMedian(photodiodeData[1], READING_WINDOW_LENGTH) * 4) / 5);
        }

        timer.restartTimer(timID);
    }
}

void loop() {
    timer.run();
}