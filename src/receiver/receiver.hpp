#include <Arduino.h>
#include <inttypes.h>
#include <QuickMedianLib.h>
#include <SimpleTimer.h>

#include "GRDiodeReader.h"
#include "GREdgeDetector.h"
#include "GRPreprocessingPipeline.h"

#include "receiver-parameters.h"
#include "receiver-util.h"

#include "../ml-arduino/prediction_enums.hpp"
#include "../ml-arduino/main_arduino.hpp"

enum class State
{
    INITIALISING,
    DETECTING_START,
    DETECTING_END,
    UPDATING_THRESHOLD,
    RESETTING
};

State state = State::INITIALISING;

int count = 0;
bool detectionWindowFull = false;
bool endDetected = false;

// Buffers for dynamic threshold adjustment
uint16_t thresholdAdjustmentBuffer[NUM_PDs][THRESHOLD_ADJ_BUFFER_LENGTH];
uint16_t *taBuffer[NUM_PDs];

// Buffers for gesture signal capture
uint16_t photodiodeData[NUM_PDs][GESTURE_BUFFER_LENGTH];
uint16_t *photodiodeDataPtr[NUM_PDs];
int gestureSignalLength;

GRDiodeReader reader;
GREdgeDetector edgeDetector[NUM_PDs];
GRPreprocessingPipeline pipeline;

SimpleTimer timer;
int timID;

void receiverOperationUpdateThreshold()
{
    for (size_t i = 0; i < NUM_PDs; i++)
    {
        uint16_t stable = QuickMedian<uint16_t>::GetMedian(thresholdAdjustmentBuffer[i], THRESHOLD_ADJ_BUFFER_LENGTH);
        edgeDetector[i].setThreshold(stable * DETECTION_THRESHOLD_COEFF);
        edgeDetector[i].setCutOffThreshold(stable * CUTT_OFF_THRESHOLD_COEFF);
        taBuffer[i] = thresholdAdjustmentBuffer[i];
    }

    state = State::DETECTING_START;
}

void receiverOperationInitialising()
{
    // If the detection window is not filled, fill it
    for (size_t i = 0; i < NUM_PDs; i++)
    {
        reader.read(pds[i], photodiodeDataPtr[i]++);
        reader.read(pds[i], taBuffer[i]++);
    }
    count++;
    if (count == DETECTION_BUFFER_LENGTH)
    {
        for (size_t i = 0; i < NUM_PDs; i++)
            photodiodeDataPtr[i]--;
        state = State::DETECTING_START;
    }
}

void receiverOperationDetectingStart()
{
    // If the detection window is already filled, shift it left with 1
    // and put the new sample in the last place
    FOR(di, i, NUM_PDs, DETECTION_BUFFER_LENGTH - 1, photodiodeData[di][i] = photodiodeData[di][i + 1])

    for (size_t i = 0; i < NUM_PDs; i++)
    {
        reader.read(pds[i], photodiodeDataPtr[i]);
        reader.read(pds[i], taBuffer[i]++);
    }

    // If there was no gesture recently, update the threshold
    if (taBuffer[0] - thresholdAdjustmentBuffer[0] >= THRESHOLD_ADJ_BUFFER_LENGTH)
    {
        state = State::UPDATING_THRESHOLD;
        return;
    }

    // Try to detect a start on one of the photodiodes
    bool startEdgeDetected = false;
    for (size_t i = 0; i < NUM_PDs; i++)
        startEdgeDetected = startEdgeDetected || edgeDetector[i].DetectStart(photodiodeDataPtr[i]);

    if (detectionWindowFull && startEdgeDetected)
    {
        state = State::DETECTING_END;
        #ifdef DEBUG_RECEIVER
            Serial.println("Gesture started");
        #endif
    }
}

void receiverOperationDetectingEnd()
{
    // Read enough more data to avoid buffer overflow when checking end
    // of gesture if more samples are checked for end than for start
    if (count++ < DETECTION_END_WINDOW_LENGTH - DETECTION_BUFFER_LENGTH)
    {
        for (size_t i = 0; i < NUM_PDs; i++)
        {
            photodiodeDataPtr[i]++;
            reader.read(pds[i], photodiodeDataPtr[i]);
        }
        return;
    }

    // Read new data and check for end of gesture
    if (count++ < GESTURE_BUFFER_LENGTH)
    {
        // Read next sample
        for (size_t i = 0; i < NUM_PDs; i++)
        {
            photodiodeDataPtr[i]++;
            reader.read(pds[i], photodiodeDataPtr[i]);
        }

        // Try to detect end in all photodiodes
        bool endEdgeDetected = true;
        for (size_t i = 0; i < NUM_PDs; i++)
            endEdgeDetected = endEdgeDetected && edgeDetector[i].DetectEnd(photodiodeDataPtr[i]);

        if (endEdgeDetected)
        {
            // Determine the gesture length
            gestureSignalLength = photodiodeDataPtr[0] - photodiodeData[0] + 1;
            
            // Reject gestures that took too short time
            if (gestureSignalLength < GESTURE_MIN_TIME_MS / READ_PERIOD + 1)
            {
                #ifdef DEBUG_RECEIVER
                                Serial.println("Gesture took too little time! Rejecting and starting over ...");
                #endif
                state = State::RESETTING;
                timer.restartTimer(timID);
            }
            else {
                endDetected = true;
            }

            // ------------------------------------------
            // Run the pipeline
            uint16_t thresholds[NUM_PDs];

            for (size_t i = 0; i < NUM_PDs; i++)
            {
                thresholds[i] = edgeDetector[i].getCutOffThreshold();
            }

            pipeline.RunPipeline(photodiodeData, gestureSignalLength, thresholds);

            float(*output)[100] = pipeline.getPipelineOutput();

            Gesture g = inferGesture2d(output);

            Serial.print("Gesture: ");
            Serial.println(g);

            state = State::RESETTING;
            timer.restartTimer(timID);
        }
    } else {
        // Gesture took too long -> Light Intensity Change -> Threshold Recalculation   
        state = State::UPDATING_THRESHOLD;
        timer.restartTimer(timID);
    }
}

void receiverOperationResetting()
{
    // Reset the buffer pointers for PD gesture data collection
    detectionWindowFull = false;
    for (size_t i = 0; i < NUM_PDs; i++)
    {
        photodiodeDataPtr[i] = photodiodeData[i];
        taBuffer[i] = thresholdAdjustmentBuffer[i];
    }
    
    if (!endDetected) {
        state = State::UPDATING_THRESHOLD;
    } else {
        state = State::INITIALISING;
    }
    timer.restartTimer(timID);
}

void receiverRunOperation()
{
    switch (state)
    {
    case State::INITIALISING:
        receiverOperationInitialising();
        break;

    case State::DETECTING_START:
        receiverOperationDetectingStart();
        break;

    case State::DETECTING_END:
        receiverOperationDetectingEnd();
        break;

    case State::UPDATING_THRESHOLD:
        receiverOperationUpdateThreshold();
        break;

    case State::RESETTING:
        receiverOperationResetting();
        break;

    default:
        break;
    }
}

void receiverSetup()
{
    for (size_t i = 0; i < NUM_PDs; i++)
    {
        pinMode(pds[i], INPUT);
        taBuffer[i] = thresholdAdjustmentBuffer[i];
        photodiodeDataPtr[i] = photodiodeData[i];
        edgeDetector[i] = GREdgeDetector(DETECTION_WINDOW_LENGTH, DETECTION_END_WINDOW_LENGTH, 750);
    }

    Serial.begin(9600);
    timID = timer.setInterval(READ_PERIOD, receiverRunOperation);
}

void receiverLoop()
{
    timer.run();
}