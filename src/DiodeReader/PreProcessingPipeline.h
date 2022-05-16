#include <Arduino.h>
#include <inttypes.h>

#include "SimpleZScore.h"
#include "SimpleMaxNormaliser.h"
// #include "SimpleHampel.h"
#include "SimpleSmoothFilter.h"
#include "SimpleSignalStretcher.h"

#include "parameters.h"
#include "util.h"

class PreProcessingPipeline {

private:
    float normPhotodiodeData[NUM_PDs][READING_WINDOW_LENGTH];
    float output[NUM_PDs][ML_DATA_LENGTH];

    // SimpleZScore zScoreCalculator;
    SimpleMaxNormaliser maxNormaliser;
    // SimpleHampel hampel(5);
    SimpleSmoothFilter sf;
    SimpleSignalStretcher sstretch;

public:
    void RunPipeline(uint16_t rawData[NUM_PDs][GESTURE_BUFFER_LENGTH], int gestureSignalLength)
    {
        Serial.println("Smoothing");
        for (size_t i = 0; i < NUM_PDs; i++)
            sf.SmoothSignal(rawData[i], normPhotodiodeData[i], gestureSignalLength, 1);     

        sendSignal(normPhotodiodeData, gestureSignalLength);

        // ----------------------------------------

        Serial.println("Normalising ...");
        // Normalize with the Z-score
        for (size_t i = 0; i < NUM_PDs; i++)
            maxNormaliser.Normalise(normPhotodiodeData[i], gestureSignalLength);

        sendSignal(normPhotodiodeData, gestureSignalLength);   

        // -----------------------------------------

        Serial.println("Stretching ...");

        // Smooth the signal
        for (size_t i = 0; i < NUM_PDs; i++)
            sstretch.StretchSignal(
                normPhotodiodeData[i],
                gestureSignalLength,
                output[i],
                ML_DATA_LENGTH);

        sendSignal(output, ML_DATA_LENGTH);   

        Serial.println("Pipeline Done");
    }
};