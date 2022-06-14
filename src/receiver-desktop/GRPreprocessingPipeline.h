#include <inttypes.h>
#include <initializer_list>
#include <vector>
#include <cmath>

#include "pipeline-stages/MaxNormaliser.h"
// #include "pipeline-stages/HampelOutlierDetector.h"
#include "pipeline-stages/FFTCutOffFilter.h"
// #include "pipeline-stages/SmoothFilter.h"
#include "pipeline-stages/SignalStretcher.h"
#include "pipeline-stages/SignalFlipper.h"

#include "receiver-parameters.h"
#include "receiver-util.h"

using namespace std;

class GRPreprocessingPipeline {

private:
    uint16_t trimmedData[NUM_PDs][GESTURE_BUFFER_LENGTH];
    float photodiodeDataFFTFiltered[NUM_PDs][FFT_SIGNAL_LENGTH];
    float (* normPhotodiodeData)[FFT_SIGNAL_LENGTH];
    float output[NUM_PDs][ML_DATA_LENGTH];

    MaxNormaliser maxNormaliser;
    SignalStretcher sstretch;
    SignalFlipper sFlipper;
    FFTCutOffFilter fftFilter[NUM_PDs];

public:
    auto getPipelineOutput() {
        return output;
    }

    void FindThresholdUsingLower(
        uint16_t rawData[NUM_PDs][GESTURE_BUFFER_LENGTH], 
        uint16_t outData[NUM_PDs][GESTURE_BUFFER_LENGTH], 
        int gestureSignalLength, uint16_t thresholds[NUM_PDs], int threshWindow = 5, bool trimUpper = true) {
        
        for (size_t i = 0; i < NUM_PDs; i++)
        {
            thresholds[i] = 0;
        }

        // Find the lower stable signal - either on the right or on the left
        uint16_t stableLeft[NUM_PDs], stableRight[NUM_PDs];

        for (size_t di = 0; di < NUM_PDs; di++)
        {
            stableLeft[di] = 0;
            stableRight[di] = 0;

            for (size_t i = 0; i < threshWindow; i++)
            {
                stableLeft[di] += rawData[di][i];
                stableRight[di] += rawData[di][gestureSignalLength - 1 - i];
            }

            stableLeft[di] /= threshWindow;
            stableRight[di] /= threshWindow;
        }
        
        // Set the thresholds
        for (size_t di = 0; di < NUM_PDs; di++) {
            thresholds[di] = min(stableLeft[di], stableRight[di]);
        }

        int newStart[3];
        int newEnd[3];

        for (size_t i = 0; i < NUM_PDs; i++)
        {
            newStart[i] = 0;
            newEnd[i] = gestureSignalLength - 1;

            // If requested, trim the upper end of the signal to the threshold
            if(trimUpper) {
                while(rawData[i][newStart[i]] > thresholds[i]) {
                    newStart[i]++;
                }

                while(rawData[i][newEnd[i]] > thresholds[i]) {
                    newEnd[i]--;
                }
            }

            for (size_t index = 0; index < newEnd - newStart + 1; index++)
            {
                outData[i][index] = rawData[i][newStart[i] + index];
            }
        }
    }

    void FindThresholdUsingMean(
        uint16_t rawData[NUM_PDs][GESTURE_BUFFER_LENGTH], 
        uint16_t outData[NUM_PDs][GESTURE_BUFFER_LENGTH], 
        int gestureSignalLength, uint16_t thresholds[NUM_PDs], int threshWindow = 5, bool trimUpper = true) {
        
        for (size_t i = 0; i < NUM_PDs; i++)
        {
            thresholds[i] = 0;
        }

        // Find the lower stable signal - either on the right or on the left
        uint16_t stableLeft[NUM_PDs], stableRight[NUM_PDs];

        for (size_t di = 0; di < NUM_PDs; di++)
        {
            stableLeft[di] = 0;
            stableRight[di] = 0;

            for (size_t i = 0; i < threshWindow; i++)
            {
                stableLeft[di] += rawData[di][i];
                stableRight[di] += rawData[di][gestureSignalLength - 1 - i];
            }

            stableLeft[di] /= threshWindow;
            stableRight[di] /= threshWindow;
        }
        
        // Set the thresholds
        for (size_t di = 0; di < NUM_PDs; di++) {
            thresholds[di] = (stableLeft[di] + stableRight[di])/2;
        }

        int newStart[3];
        int newEnd[3];

        for (size_t i = 0; i < NUM_PDs; i++)
        {
            newStart[i] = 0;
            newEnd[i] = gestureSignalLength - 1;

            // If requested, trim the upper end of the signal to the threshold
            if(trimUpper) {
                while(rawData[i][newStart[i]] > thresholds[i]) {
                    newStart[i]++;
                }

                while(rawData[i][newEnd[i]] > thresholds[i]) {
                    newEnd[i]--;
                }
            }

            for (size_t index = 0; index < newEnd - newStart + 1; index++)
            {
                outData[i][index] = rawData[i][newStart[i] + index];
            }
        }
    }

    void RunPipeline(
        uint16_t rawData[NUM_PDs][GESTURE_BUFFER_LENGTH], 
        int gestureSignalLength, 
        uint16_t thresholds[NUM_PDs], 
        int samplingFrequncy, int thresholdScheme = 0, bool trimUpper = true)
    {
        if (thresholdScheme == 1) {
            FindThresholdUsingLower(rawData, trimmedData, gestureSignalLength, thresholds, trimUpper);
            rawData = trimmedData;
        } else if (thresholdScheme == 2) {
            FindThresholdUsingMean(rawData, trimmedData, gestureSignalLength, thresholds, trimUpper);
            rawData = trimmedData;
        }

        for (size_t i = 0; i < NUM_PDs; i++)
        {
            thresholds[i] = thresholds[i] * CUTT_OFF_THRESHOLD_COEFF;
        }

        // ----------------------------------------
        // CutOff and Flip : USe 1.1% the threshold
        FOR(di, i, NUM_PDs, gestureSignalLength, 
            rawData[di][i] = max(0.0f, thresholds[di] * CUTT_OFF_THRESHOLD_COEFF_PRE_FFT - rawData[di][i])
        );

        // Trim the signal
        bool trimmed = false;
        int trimCount = 0;
        int i = gestureSignalLength;
        while(i-- >= 0 && trimCount++ < DETECTION_END_WINDOW_LENGTH * DETECTION_END_WINDOW_TRIM) {
            bool zero = true;
            for (size_t di = 0; di < NUM_PDs; di++)
                zero = zero && (rawData[di][i] == 0);
            
            if (zero) {
                trimmed = true;
                gestureSignalLength--;
            } else break;
        }
        if(trimmed) gestureSignalLength++;

        trimmed = false;
        int trimmedStart = 0;
        int prevGestureSignalLength = gestureSignalLength;
        while(trimmedStart < prevGestureSignalLength) {
            bool zero = true;
            for (size_t di = 0; di < NUM_PDs; di++)
                zero = zero && (rawData[di][trimmedStart] == 0);
            
            if (zero) {
                trimmed = true;
                trimmedStart++;
                gestureSignalLength--;
            } else break;
        }
        if (trimmed) {
            trimmedStart--;
            gestureSignalLength++;
        }

        // ----------------------------------------
        // Filter using FFT
        for (size_t i = 0; i < NUM_PDs; i++)
        {
            fftFilter[i].ZeroImag();
            fftFilter[i].Filter(rawData[i] + trimmedStart, gestureSignalLength, 5, samplingFrequncy);
            fftFilter[i].MoveDataToBufferF(photodiodeDataFFTFiltered[i]);
        }
 
        gestureSignalLength = FFT_SIGNAL_LENGTH;

        FOR(di, i, NUM_PDs, gestureSignalLength, 
            photodiodeDataFFTFiltered[di][i] = max(0.0f, photodiodeDataFFTFiltered[di][i] - thresholds[di] * CUTT_OFF_THRESHOLD_COEFF_POST_FFT);
        );

        // Trim the signal
        trimmed = false;
        trimCount = 0;
        i = gestureSignalLength;
        while(i-- >= 0) {
            bool zero = true;
            for (size_t di = 0; di < NUM_PDs; di++)
                zero = zero && (photodiodeDataFFTFiltered[di][i] == 0);
            
            if (zero) {
                trimmed = true;
                gestureSignalLength--;
            } else break;
        }
        if(trimmed) gestureSignalLength++;

        trimmed = false;
        trimmedStart = 0;
        prevGestureSignalLength = gestureSignalLength;
        while(trimmedStart < prevGestureSignalLength) {
            bool zero = true;
            for (size_t di = 0; di < NUM_PDs; di++)
                zero = zero && (photodiodeDataFFTFiltered[di][trimmedStart] == 0);
            
            if (zero) {
                trimmed = true;
                trimmedStart++;
                gestureSignalLength--;
            } else break;
        }
        if (trimmed) {
            trimmedStart--;
            gestureSignalLength++;
        }

        normPhotodiodeData = photodiodeDataFFTFiltered;

        // ----------------------------------------
        // Normalize dividing by the max
        for (size_t i = 0; i < NUM_PDs; i++)
            maxNormaliser.Normalise(normPhotodiodeData[i] + trimmedStart, gestureSignalLength);

        // -----------------------------------------
        // Stretch and Flip back
        for (size_t i = 0; i < NUM_PDs; i++) {
            sstretch.StretchSignal(
                normPhotodiodeData[i] + trimmedStart,
                gestureSignalLength,
                output[i],
                ML_DATA_LENGTH);

            // Flip the signal
            sFlipper.FlipSignal(
                output[i],
                ML_DATA_LENGTH,
                0.0f,1.0f);
        }
    }
};