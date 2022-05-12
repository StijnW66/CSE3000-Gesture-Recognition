#include <Arduino.h>
#include <inttypes.h>

#include "SimpleZScore.h"
// #include "SimpleHampel.h"
#include "SimpleSmoothFilter.h"
#include "SimpleSignalStretcher.h"

#include "parameters.h"

float normPhotodiodeData[2][READING_WINDOW_LENGTH];
float output[2][ML_DATA_LENGTH];

SimpleZScore zScoreCalculator;
// SimpleHampel hampel(5);
SimpleSmoothFilter sf;
SimpleSignalStretcher sstretch;

void RunPipeline(uint16_t rawData[2][GESTURE_BUFFER_LENGTH], int gestureSignalLength)
{
    Serial.println("Smoothing");
    sf.SmoothSignal(rawData[0], normPhotodiodeData[0], gestureSignalLength, 1);
    sf.SmoothSignal(rawData[1], normPhotodiodeData[1], gestureSignalLength, 1);

    Serial.println("Start");
    for (int i = 0; i < gestureSignalLength; i++)
    {
        Serial.print(normPhotodiodeData[0][i]);
        Serial.print(" ");
        Serial.println(normPhotodiodeData[1][i]);
    }
    Serial.println("Done");

    // ----------------------------------------

    Serial.println("Normalising ...");
    // Normalize with the Z-score
    zScoreCalculator.ComputeZScore(normPhotodiodeData[0], gestureSignalLength, true);
    zScoreCalculator.ComputeZScore(normPhotodiodeData[1], gestureSignalLength, true);

    Serial.println("Start");
    for (int i = 0; i < gestureSignalLength; i++)
    {
        Serial.print(normPhotodiodeData[0][i]);
        Serial.print(" ");
        Serial.println(normPhotodiodeData[1][i]);
    }
    Serial.println("Done");

    // -----------------------------------------

    Serial.println("Stretching ...");

    // Smooth the signal
    sstretch.StretchSignal(
        normPhotodiodeData[0],
        gestureSignalLength,
        output[0],
        ML_DATA_LENGTH);
    sstretch.StretchSignal(
        normPhotodiodeData[1],
        gestureSignalLength,
        output[1],
        ML_DATA_LENGTH);

    Serial.println("Start");
    for (int i = 0; i < ML_DATA_LENGTH; i++)
    {
        Serial.print(output[0][i]);
        Serial.print(" ");
        Serial.println(output[1][i]);
    }
    Serial.println("Done");

    Serial.println("Pipeline Done");
}