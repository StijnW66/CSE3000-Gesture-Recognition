#include <Arduino.h>
#include <inttypes.h>
#include <QuickMedianLib.h>
#include <SimpleTimer.h>

#include "SimplePhotoDiodeReader.h"
#include "SimpleGestureEdgeDetector.h"
#include "parameters.h"
#include "PreProcessingPipeline.h"
#include "util.h"


int count = 0;
bool detectionWindowFull = false;

// Buffers for dynamic threshold adjustment
uint16_t thresholdAdjustmentBuffer[NUM_PDs][THRESHOLD_ADJ_BUFFER_LENGTH];
uint16_t * taBuffer[NUM_PDs];

// Buffers for gesture signal capture
uint16_t photodiodeData[NUM_PDs][READING_WINDOW_LENGTH];
uint16_t * photodiodeDataPtr[NUM_PDs];
int gestureSignalLength;

SimplePhotoDiodeReader      reader;
SimpleGestureEdgeDetector   edgeDetector[NUM_PDs];
PreProcessingPipeline       pipeline;

SimpleTimer timer;
int timID;


void receiverLoopMain() {
  if(detectionWindowFull == false) {
         // If the detection window is not filled, fill it
         for (size_t i = 0; i < NUM_PDs; i++)
         {
             reader.read(pds[i], photodiodeDataPtr[i]++);
             reader.read(pds[i], taBuffer[i]++);
         }

         count++;

         if (count == DETECTION_BUFFER_LENGTH) {
             for (size_t i = 0; i < NUM_PDs; i++) photodiodeDataPtr[i]--;
             detectionWindowFull = true;
         }

    } else {

        // If the detection window is already filled, shift it left with 1
        // and put the new sample in the last place
        for (size_t i = 0; i < DETECTION_BUFFER_LENGTH - 1; i++)
            for (size_t pdId = 0; pdId < NUM_PDs; pdId++) 
                photodiodeData[pdId][i] = photodiodeData[pdId][i+1];

        for (size_t i = 0; i < NUM_PDs; i++)
         {
             reader.read(pds[i], photodiodeDataPtr[i]);
             reader.read(pds[i], taBuffer[i]++);
         }       
    }

    // If there was no gesture recently, update the threshold
    if (taBuffer[0] - thresholdAdjustmentBuffer[0] >= THRESHOLD_ADJ_BUFFER_LENGTH) {
        
        for (size_t i = 0; i < NUM_PDs; i++)
        {
            edgeDetector[i].setThreshold((QuickMedian<uint16_t>::GetMedian(thresholdAdjustmentBuffer[i], THRESHOLD_ADJ_BUFFER_LENGTH) * 4) / 5);
            taBuffer[i] = thresholdAdjustmentBuffer[i];
        }
    }

    bool startEdgeDetected = false;
    for (size_t i = 0; i < NUM_PDs; i++) 
        startEdgeDetected = startEdgeDetected || edgeDetector[i].DetectStart(photodiodeDataPtr[i]);
    
    // Try to detect a start on one of the photodiodes
    if (detectionWindowFull && startEdgeDetected) {

        Serial.println("Gesture started");

        bool endDetected = false;

        // Read enough more data to avoid buffer overflow when checking end
        // of gesture if more samples are checked for end than for start
        while(count++ < DETECTION_END_WINDOW_LENGTH - DETECTION_BUFFER_LENGTH) {
            for (size_t i = 0; i < NUM_PDs; i++)
            {
                photodiodeDataPtr[i]++;
                reader.read(pds[i], photodiodeDataPtr[i]);
            }
            delay(READ_PERIOD);
        }

        // Read new data and check for end of gesture
        while(count++ < READING_WINDOW_LENGTH) {
            for (size_t i = 0; i < NUM_PDs; i++)
            {
                photodiodeDataPtr[i]++;
                reader.read(pds[i], photodiodeDataPtr[i]);
            }

            delay(READ_PERIOD);

            bool endEdgeDetected = true;
            for (size_t i = 0; i < NUM_PDs; i++)
                endEdgeDetected = endEdgeDetected && edgeDetector[i].DetectEnd(photodiodeDataPtr[i]);

            if(endEdgeDetected) {

                // Determine the gesture length
                gestureSignalLength = photodiodeDataPtr[0] - photodiodeData[0] + 1;

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
                    for (size_t i = 0; i < NUM_PDs; i++)
                        photodiodeData[i][index] = max(edgeDetector[i].getThreshold() - photodiodeData[i][index], 0);
                }

                sendSignal(photodiodeData, gestureSignalLength);
// ----------------------------------------

                // Run the pipeline
                pipeline.RunPipeline(photodiodeData, gestureSignalLength);
                
                break;
            }
        }

        // Reset the buffer pointers for PD gesture data collection
        detectionWindowFull = false;
        for (size_t i = 0; i < NUM_PDs; i++)
        {
            photodiodeDataPtr[i] = photodiodeData[i];
            taBuffer[i] = thresholdAdjustmentBuffer[i];
        }

        count = 0;

        // Gesture took too long -> Light Intensity Change -> Threshold Recalculation
        if(!endDetected)
            for (size_t i = 0; i < NUM_PDs; i++)
                edgeDetector[i].setThreshold((QuickMedian<uint16_t>::GetMedian(photodiodeData[i], READING_WINDOW_LENGTH) * 4) / 5);

        timer.restartTimer(timID);
    }
}

void receiverSetup() {
  for (size_t i = 0; i < NUM_PDs; i++)
    {
        pinMode(pds[i], INPUT);
        taBuffer[i]          =   thresholdAdjustmentBuffer[i];
        photodiodeDataPtr[i] =   photodiodeData[i];
        edgeDetector[i]      =   SimpleGestureEdgeDetector(DETECTION_WINDOW_LENGTH, DETECTION_END_WINDOW_LENGTH, 750);
    }

    Serial.begin(9600);

    timID = timer.setInterval(READ_PERIOD, receiverLoopMain);
}

void receiverLoop() {
  timer.run();

    // Serial.print(analogRead(PD1));
    // Serial.print(" "); 
    // Serial.print(analogRead(PD2));
    // Serial.print(" ");
    // Serial.println(analogRead(PD3));

    // delay(READ_PERIOD);
}
