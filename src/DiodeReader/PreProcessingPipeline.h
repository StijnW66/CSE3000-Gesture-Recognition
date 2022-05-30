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
    SimpleFFTCutOffFilter fftFilter[NUM_PDs];

public:
    void RunPipeline(uint16_t rawData[NUM_PDs][GESTURE_BUFFER_LENGTH], int gestureSignalLength, uint16_t thresholds[NUM_PDs])
    {
        Serial.println("FFT Filtering");

        // Filter using FFT
        for (size_t i = 0; i < NUM_PDs; i++)
        {
            fftFilter[i].ZeroImag();
            fftFilter[i].Filter(rawData[i], gestureSignalLength, 10, 1000 / READ_PERIOD);
            fftFilter[i].MoveDataToBufferF(photodiodeDataFFTFiltered[i]);
        }

        gestureSignalLength = FFT_SIGNAL_LENGTH;

        // Cut off using the thresholds
        for (size_t di = 0; di < NUM_PDs; di++)
        {
            for (size_t i = 0; i < gestureSignalLength; i++)
            {
                photodiodeDataFFTFiltered[di][i] = max(0, thresholds[di] - photodiodeDataFFTFiltered[di][i]);
            }
        }
        
        sendSignal(photodiodeDataFFTFiltered, FFT_SIGNAL_LENGTH);

        // ----------------------------------------

        Serial.println("Smoothing");

        // Smooth using an averaging filter
        std::vector<float> ws = {0.25f, 0.5f, 0.25f};
        for (size_t i = 0; i < NUM_PDs; i++)
            smf.SmoothSignal(photodiodeDataFFTFiltered[i], normPhotodiodeData[i], gestureSignalLength, 1, ws.data());     

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