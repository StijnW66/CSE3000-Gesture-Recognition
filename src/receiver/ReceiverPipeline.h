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

#include "parameters.h"
#include "util.h"

class ReceiverPipeline {

private:
    float photodiodeDataFFTFiltered[NUM_PDs][FFT_SIGNAL_LENGTH];
    float normPhotodiodeData[NUM_PDs][GESTURE_BUFFER_LENGTH];
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
        int index = -1;
        while(++index < gestureSignalLength) {
            for (size_t i = 0; i < NUM_PDs; i++)
                rawData[i][index] = max(0, thresholds[i] - rawData[i][index]);
        }
        // Trim the signal
        bool trimmed = false;
        int trimCount = 0;
        while(index-- >= 0 && trimCount++ < DETECTION_END_WINDOW_LENGTH * DETECTION_END_WINDOW_TRIM) {
            bool zero = true;
            for (size_t i = 0; i < NUM_PDs; i++)
                zero = zero && (rawData[i][index] <= 2);
            
            if (zero) {
                trimmed = true;
                gestureSignalLength--;
            }
        }
        if(trimmed) gestureSignalLength++;

        sendSignal(rawData, gestureSignalLength);
        
        // ----------------------------------------
        Serial.println("FFT Filtering");

        int nonZeroStart[NUM_PDs];
        int nonZeroEnd[NUM_PDs];

        for (size_t i = 0; i < NUM_PDs; i++)
        {
            nonZeroStart[i] = 0;
            nonZeroEnd[i] = gestureSignalLength--;
        }

        for (size_t i = 0; i < NUM_PDs; i++) {

            bool updateStart = true;
            bool updateEnd = true;

            while(nonZeroStart[i] < nonZeroEnd[i]) { 
                if (updateStart)
                {
                    if(rawData[i][nonZeroStart[i]] <= 1) nonZeroStart[i]++;
                    else updateStart = false;
                }
                if (updateEnd)
                {
                    if(rawData[i][nonZeroEnd[i]] <= 1) nonZeroEnd[i]--;
                    else updateEnd = false;
                }
                if (!updateStart && !updateEnd) break;
            }
        }

        // Filter using FFT
        for (size_t i = 0; i < NUM_PDs; i++)
        {
            fftFilter[i].ZeroImag();
            fftFilter[i].Filter(rawData[i], gestureSignalLength, 10, 1000 / READ_PERIOD);
            fftFilter[i].MoveDataToBufferF(photodiodeDataFFTFiltered[i]);
        }

        // Adjust the zero out range and update the signal length
        for (size_t i = 0; i < NUM_PDs; i++)
        {
            nonZeroStart[i] = nonZeroStart[i] * FFT_SIGNAL_LENGTH / gestureSignalLength;
            nonZeroEnd[i] = nonZeroEnd[i] * FFT_SIGNAL_LENGTH / gestureSignalLength;
        }
 
        gestureSignalLength = FFT_SIGNAL_LENGTH;

        // Zero out all samples that are not in the previously computed range
        for (size_t di = 0; di < NUM_PDs; di++)
        {
            for (int i = 0; i < gestureSignalLength; i++)
            {
                if(i < nonZeroStart[di] || i > nonZeroEnd[di]) photodiodeDataFFTFiltered[di][i] = 0;
                else photodiodeDataFFTFiltered[di][i] = max(0, photodiodeDataFFTFiltered[di][i]);
            }    
        }
        
        sendSignal(photodiodeDataFFTFiltered, FFT_SIGNAL_LENGTH);

        for (int di = 0; di < NUM_PDs; di++)
        {
            for (int i = 0; i < gestureSignalLength; i++)
            {
                normPhotodiodeData[di][i] = photodiodeDataFFTFiltered[di][i];
            }
        }

        // ----------------------------------------
        Serial.println("Normalising ...");

        // Normalize dividing by the max
        for (size_t i = 0; i < NUM_PDs; i++)
            maxNormaliser.Normalise(normPhotodiodeData[i], gestureSignalLength);

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