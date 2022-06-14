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

    void RunPipeline(uint16_t rawData[NUM_PDs][GESTURE_BUFFER_LENGTH], int gestureSignalLength, uint16_t thresholds[NUM_PDs], int samplingFrequncy)
    {

        // // compute stable start and end
        // uint16_t threshold[NUM_PDs];
        // for (size_t i = 0; i < NUM_PDs; i++)
        // {
        //     thresholds[i] = 0;
        // }

        // FOR(di, i, NUM_PDs, 5, thresholds[di] += rawData[i])
        // FOR(di, i, NUM_PDs, 5, thresholds[di] += rawData[gestureSignalLength - 1 - i])

        // for (size_t i = 0; i < NUM_PDs; i++)
        // {
        //     thresholds[i] /= 10;
        // }

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