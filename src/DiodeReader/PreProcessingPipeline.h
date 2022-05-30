#include <Arduino.h>
#include <inttypes.h>
#include <initializer_list>
#include <vector>

#include "SimpleZScore.h"
#include "SimpleMaxNormaliser.h"
// #include "SimpleHampel.h"
#include "SimpleFFTCutOffFilter.h"
#include "SimpleSmoothFilter.h"
#include "SimpleSignalStretcher.h"
#include "SimpleSignalFlipper.h"

#include "parameters.h"
#include "util.h"

class PreProcessingPipeline {

private:
    float photodiodeDataFFTFiltered[NUM_PDs][FFT_SIGNAL_LENGTH];
    float normPhotodiodeData[NUM_PDs][GESTURE_BUFFER_LENGTH];
    float output[NUM_PDs][ML_DATA_LENGTH];

    SimpleMaxNormaliser maxNormaliser;
    // SimpleHampel hampel(5);
    SimpleSmoothFilter smf;
    SimpleSignalStretcher sstretch;
    SimpleSignalFlipper sFlipper;

public:
    void RunPipeline(uint16_t rawData[NUM_PDs][GESTURE_BUFFER_LENGTH], int gestureSignalLength)
    {
        Serial.println("FFT Filtering");

        // Filter using FFT
        for (size_t i = 0; i < NUM_PDs; i++)
        {
            fftFilter[i].ZeroImag();
            fftFilter[i].Filter(rawData[i], gestureSignalLength, 10, 1000 / READ_PERIOD);
            fftFilter[i].MoveDataToBufferF(photodiodeDataFFTFiltered[i]);
        }

        sendSignal(photodiodeDataFFTFiltered, FFT_SIGNAL_LENGTH);

        gestureSignalLength = FFT_SIGNAL_LENGTH;

        // ----------------------------------------

        Serial.println("Smoothing");

        // Smooth using an averaging filter
        std::vector<float> ws = {0.25f, 0.5f, 0.25f};
        for (size_t i = 0; i < NUM_PDs; i++)
            smf.SmoothSignal(rawData[i], normPhotodiodeData[i], gestureSignalLength, 1, ws.data());     

        sendSignal(normPhotodiodeData, gestureSignalLength);

        // ----------------------------------------

        Serial.println("Normalising ...");

        // Normalize dividing by the max
        for (size_t i = 0; i < NUM_PDs; i++)
            maxNormaliser.Normalise(normPhotodiodeData, gestureSignalLength);

        sendSignal(normPhotodiodeData, gestureSignalLength);   

        // -----------------------------------------

        Serial.println("Stretching ...");

        // Smooth the signal
        for (size_t i = 0; i < NUM_PDs; i++) {
            sstretch.StretchSignal(
                normPhotodiodeData[i],
                gestureSignalLength,
                output[i],
                ML_DATA_LENGTH);

            // Flip the signal
            sFlipper.FlipSignal(
                output[i],
                ML_DATA_LENGTH,
                0.0f,1.0f);
        }
        
        sendSignal(output, ML_DATA_LENGTH);   

        Serial.println("Pipeline Done");
    }
};