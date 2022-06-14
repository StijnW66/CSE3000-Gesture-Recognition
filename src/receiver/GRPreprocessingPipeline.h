#include <Arduino.h>
#include <inttypes.h>
#include <initializer_list>
#include <vector>

#include "pipeline-stages/MaxNormaliser.h"
// #include "pipeline-stages/HampelOutlierDetector.h"
#include "pipeline-stages/FFTCutOffFilter.h"
// #include "pipeline-stages/SmoothFilter.h"
#include "pipeline-stages/SignalStretcher.h"
#include "pipeline-stages/SignalFlipper.h"

#include "receiver-parameters.h"
#include "receiver-util.h"

class GRPreprocessingPipeline {

private:
    float photodiodeDataFFTFiltered[NUM_PDs][FFT_SIGNAL_LENGTH];
    float (* normPhotodiodeData)[FFT_SIGNAL_LENGTH];
    float output[NUM_PDs][ML_DATA_LENGTH];

    MaxNormaliser maxNormaliser;
    // HampelOutlierDetection hampel(5);
    // SmoothFilter smf;
    SignalStretcher sstretch;
    SignalFlipper sFlipper;
    FFTCutOffFilter fftFilter[NUM_PDs];

public:
    auto getPipelineOutput() {
        return output;
    }

    void RunPipeline(uint16_t rawData[NUM_PDs][GESTURE_BUFFER_LENGTH], int gestureSignalLength, uint16_t thresholds[NUM_PDs])
    {
        // ----------------------------------------
        Serial.println("Cutting off, Flipping and Trimming");
        
        FOR(di, i, NUM_PDs, gestureSignalLength, 
            rawData[di][i] = max(0, thresholds[di] * CUTT_OFF_THRESHOLD_COEFF_PRE_FFT - rawData[di][i])
        );

        // Trim the signal
        bool trimmed = false;
        int trimCount = 0;
        int i = gestureSignalLength;
        while(i-- >= 0 && trimCount++ < DETECTION_END_WINDOW_LENGTH * DETECTION_END_WINDOW_TRIM) {
            bool zero = true;
            for (size_t di = 0; di < NUM_PDs; di++)
                zero = zero && (rawData[di][i] <= 1);
            
            if (zero) {
                trimmed = true;
                gestureSignalLength--;
            }
        }
        if(trimmed) gestureSignalLength++;

        sendSignal(rawData, gestureSignalLength);
        
        // ----------------------------------------
        Serial.println("FFT Filtering");

        // Filter using FFT
        for (size_t i = 0; i < NUM_PDs; i++)
        {
            fftFilter[i].ZeroImag();
            fftFilter[i].Filter(rawData[i], gestureSignalLength, 5, 1000 / READ_PERIOD);
            fftFilter[i].MoveDataToBufferF(photodiodeDataFFTFiltered[i]);
        }
 
        gestureSignalLength = FFT_SIGNAL_LENGTH;

        FOR(di, i, NUM_PDs, gestureSignalLength, 
            photodiodeDataFFTFiltered[di][i] = max(0, photodiodeDataFFTFiltered[di][i] - thresholds[di] * CUTT_OFF_THRESHOLD_COEFF_POST_FFT);
        );
        
        sendSignal(photodiodeDataFFTFiltered, FFT_SIGNAL_LENGTH);

        normPhotodiodeData = photodiodeDataFFTFiltered;

        // ----------------------------------------
        Serial.println("Normalising ...");

        // Normalize dividing by the max
        for (size_t i = 0; i < NUM_PDs; i++)
            maxNormaliser.Normalise(normPhotodiodeData[i], gestureSignalLength);

        sendSignal(normPhotodiodeData, gestureSignalLength);   

        // -----------------------------------------
        Serial.println("Stretching ...");

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