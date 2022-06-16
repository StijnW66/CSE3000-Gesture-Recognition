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

#include "../diode_calibration/diode_calibration.h"

/**
 * @brief An enum for the different receiver stages of computation. Used for creating FSM architecture.
 * 
 */
enum class State
{
    INITIALISING,
    DETECTING_START,
    DETECTING_END,
    UPDATING_THRESHOLD_AB,
    UPDATING_THRESHOLD_PB,
    UPDATING_THRESHOLD_ACTUAL,
    RESETTING
};

// The current receiver state in the FSM
State state = State::INITIALISING;

// flags that indicate whether enough PD data is read for start detection to begin
//      and whether an end is successfully detected from an actual gesture
bool detectionWindowFull    = false;
bool endDetected            = false;

// Buffers for dynamic threshold adjustment
uint16_t thresholdAdjustmentBuffer[NUM_PDs][THRESHOLD_ADJ_BUFFER_LENGTH];
int thresholdAdjDataIndex = 0;

// Buffers for gesture signal capture
uint16_t photodiodeData[NUM_PDs][GESTURE_BUFFER_LENGTH];

// Current index in the buffer for new photodiode data to be written to
int gestureDataIndex = 0;

// Detected gesture signal length
int gestureSignalLength;

GRDiodeReader reader;
GREdgeDetector edgeDetector[NUM_PDs];
GRPreprocessingPipeline pipeline;
LightIntensityRegulator regulator;

SimpleTimer timer;
int timID;

void receiverOperationUpdateThresholdFromPhoBuffer() {
    bool deltaLBigger = false, deltaLSmaller = false;

    for (size_t i = 0; i < NUM_PDs; i++)
    {
        uint16_t stable = QuickMedian<uint16_t>::GetMedian(photodiodeData[i], GESTURE_BUFFER_LENGTH);
        uint16_t deltaL = stable * DETECTION_THRESHOLD_COEFF - edgeDetector[i].getThreshold();

        if (deltaL > 100)
            deltaLBigger = true;
        if (deltaL < -100)
            deltaLSmaller = true;
    }

    if (deltaLBigger)
        regulator.resistorDown();
    else if (deltaLSmaller)
        regulator.resistorUp();

    // Calculate new threshold
    state = State::UPDATING_THRESHOLD_ACTUAL;
    timer.restartTimer(timID);
}

void receiverOperationUpdateThresholdFromAdjBuffer()
{
    bool deltaLBigger = false, deltaLSmaller = false;

    for (size_t i = 0; i < NUM_PDs; i++)
    {
        uint16_t stable = QuickMedian<uint16_t>::GetMedian(thresholdAdjustmentBuffer[i], THRESHOLD_ADJ_BUFFER_LENGTH);
        edgeDetector[i].setThreshold(stable * DETECTION_THRESHOLD_COEFF);
        edgeDetector[i].setCutOffThreshold(stable * CUTT_OFF_THRESHOLD_COEFF);
    }

    // if (deltaLBigger)
    //     regulator.resistorDown();
    // else if (deltaLSmaller)
    //     regulator.resistorUp();

    // Calculate new threshold
    state = State::RESETTING;
    timer.restartTimer(timID);
    thresholdAdjDataIndex = 0;
}

void receiverOperationUpdateThresholdActual() {
    if (thresholdAdjDataIndex < THRESHOLD_UPD_BUFFER_LENGTH - 1) 
    {
        for (size_t i = 0; i < NUM_PDs; i++)
        {
            reader.read(pds[i], &thresholdAdjustmentBuffer[i][thresholdAdjDataIndex]);
        }

        thresholdAdjDataIndex++;
    } 
    else 
    {
        for (size_t i = 0; i < NUM_PDs; i++)
        {
            uint16_t stable = QuickMedian<uint16_t>::GetMedian(thresholdAdjustmentBuffer[i], THRESHOLD_UPD_BUFFER_LENGTH);
            edgeDetector[i].setThreshold(stable * DETECTION_THRESHOLD_COEFF);
            edgeDetector[i].setCutOffThreshold(stable * CUTT_OFF_THRESHOLD_COEFF);
        }
        state = State::RESETTING;
        timer.restartTimer(timID);
    }
}

void receiverOperationInitialising()
{
    // If the detection window is not filled, fill it
    for (size_t i = 0; i < NUM_PDs; i++)
    {
        reader.read(pds[i], &photodiodeData[i][gestureDataIndex]);
        reader.read(pds[i], &thresholdAdjustmentBuffer[i][thresholdAdjDataIndex]);
    }

    thresholdAdjDataIndex++;
    
    if (gestureDataIndex < DETECTION_BUFFER_LENGTH - 1)
    {
        gestureDataIndex++;
    } 
    else {
        state = State::DETECTING_START;
        detectionWindowFull = true;
    }
}

void receiverOperationDetectingStart()
{
    // If the detection window is already filled, shift it left with 1
    // and put the new sample in the last place
    FOR(di, i, NUM_PDs, DETECTION_BUFFER_LENGTH - 1, photodiodeData[di][i] = photodiodeData[di][i + 1])

    for (size_t i = 0; i < NUM_PDs; i++)
    {
        reader.read(pds[i], &photodiodeData[i][gestureDataIndex]);
        reader.read(pds[i], &thresholdAdjustmentBuffer[i][thresholdAdjDataIndex]);
    }

    thresholdAdjDataIndex++;

    // If there was no gesture recently, update the threshold
    if (thresholdAdjDataIndex >= THRESHOLD_ADJ_BUFFER_LENGTH - 1)
    {
        state = State::UPDATING_THRESHOLD_AB;
        timer.restartTimer(timID);
        return;
    }

    // Try to detect a start on one of the photodiodes
    bool startEdgeDetected = false;
    for (size_t i = 0; i < NUM_PDs; i++){
        if(edgeDetector[i].DetectStart(&photodiodeData[i][gestureDataIndex])){
            state = State::DETECTING_END;
#ifdef DEBUG_RECEIVER
            Serial.println("Gesture started");
#endif
        }
    }
}

void receiverOperationDetectingEnd()
{
    // Read new data and check for end of gesture
    if (gestureDataIndex < GESTURE_BUFFER_LENGTH - 1)
    {
        gestureDataIndex++;

        // Read next sample
        for (size_t i = 0; i < NUM_PDs; i++)
        {
            reader.read(pds[i], &photodiodeData[i][gestureDataIndex]);
        }

        // Read enough more data to avoid buffer overflow when checking end
        // of gesture if more samples are checked for end than for start
        if (gestureDataIndex < DETECTION_END_WINDOW_LENGTH - 1) return;

        // Try to detect end in all photodiodes
        bool endEdgeDetected = true;
        for (size_t i = 0; i < NUM_PDs; i++)
            endEdgeDetected = endEdgeDetected && edgeDetector[i].DetectEnd(&photodiodeData[i][gestureDataIndex]);

        if (endEdgeDetected)
        {
            // Determine the gesture length
            gestureSignalLength = gestureDataIndex + 1;

            // Reject gestures that took too short time
            if (gestureSignalLength < GESTURE_MIN_TIME_MS / READ_PERIOD + 1)
            {
#ifdef DEBUG_RECEIVER
                Serial.println("Gesture took too little time! Rejecting and starting over ...");
#endif
                state = State::RESETTING;
                timer.restartTimer(timID);
            }
            else
            {
                endDetected = true;
            }

            // ------------------------------------------
            // Run the pipeline
            uint16_t thresholds[NUM_PDs];

            for (size_t i = 0; i < NUM_PDs; i++)
            {
                thresholds[i] = edgeDetector[i].getCutOffThreshold();
            }

            Serial.println("Running pipeline ...");
            pipeline.RunPipeline(photodiodeData, gestureSignalLength, thresholds);

            float(*output)[100] = pipeline.getPipelineOutput();

            Gesture g = inferGesture2d(output);

            Serial.print("Gesture: ");
            Serial.println(g);

            state = State::RESETTING;
            timer.restartTimer(timID);
        }
    }
    else
    {
        // Gesture took too long -> Light Intensity Change -> Threshold Recalculation
        state = State::UPDATING_THRESHOLD_PB;
        timer.restartTimer(timID);
    }
}

void receiverOperationResetting()
{
    // Reset the buffer pointers for PD gesture data collection
    detectionWindowFull = false;
    gestureDataIndex = 0;
    thresholdAdjDataIndex = 0;

    state = State::INITIALISING;
    timer.restartTimer(timID);
}

/**
 * @brief FSM state operation selection.
 * 
 * INITIALISING                 - collect enough data before beginning start detection.
 * DETECTING_START              - start detection
 * DETECTING_END                - end detection and pipeline running
 * UPDATING_THRESHOLD_AB        - update the edge detection threshold after a gesture wasnt performed for some time
 * UPDATING_THRESHOLD_PB        - update the edge detection threshold in case a gesture was too long
 * UPDATING_THRESHOLD_ACTUAL    - only this state updates the edge detection threshold by collecting data and find its medium
 * RESETTING                    - reset the counters and go back to INITIALISING
 */
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

    case State::UPDATING_THRESHOLD_AB:
        receiverOperationUpdateThresholdFromAdjBuffer();
        break;
    
    case State::UPDATING_THRESHOLD_PB:
        receiverOperationUpdateThresholdFromPhoBuffer();
        break;

    case State::UPDATING_THRESHOLD_ACTUAL:
        receiverOperationUpdateThresholdActual();
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
        edgeDetector[i] = GREdgeDetector(DETECTION_WINDOW_LENGTH, DETECTION_END_WINDOW_LENGTH, 500);
    }

    Serial.begin(9600);
    timID = timer.setInterval(READ_PERIOD, receiverRunOperation);
}

void receiverLoop()
{
    // Serial.println("Running operation: ");
    // Serial.println(static_cast<int>(state));
    receiverRunOperation();
}