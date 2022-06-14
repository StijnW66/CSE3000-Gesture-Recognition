#include<inttypes.h>
#include<arduinoFFT.h>

#include"../receiver-parameters.h"
#include"SignalStretcher.h"

/**
 * A class the implements low-pass filtering using FFT and iFFT.
 * - 2 inner buffers to store the real and imaginary parts of
 * the FFT and iFFT input and output
 * - str object to stretch the input signal to the desired power of 2
 * - fft object that computes FFT and iFFT
 */
class FFTCutOffFilter {
    
    private:
        double sFFT[FFT_SIGNAL_LENGTH];
        double imag[FFT_SIGNAL_LENGTH];

        SignalStretcher str;
        arduinoFFT fft;

    public:
        FFTCutOffFilter() {}

        /**
         * Initialise the buffers before actual filtering
         * by zeroing the imaginary components for the 
         * input signal left from the previous usage of
         * the filter.
         */
        void ZeroImag() {
            for (size_t i = 0; i < FFT_SIGNAL_LENGTH; i++)
            {
                imag[i] = 0;
            }
        }

        /**
         * Perform the low-pass filtering. Save the result in the inner
         * buffers.
         */
        void Filter(uint16_t * signal, int length, int cutOffFreq, int samplingFrequency) {
            
            // Stretch the signal to the desired power of 2 and put in the 
            // inner real value buffer.
            str.StretchSignal(signal, length, sFFT, FFT_SIGNAL_LENGTH);

            // Update the samplingFrequency for the stretched data            
            double actualSamplingFrequency = (samplingFrequency * length * 1.0) / FFT_SIGNAL_LENGTH;

            fft = arduinoFFT(sFFT, imag, FFT_SIGNAL_LENGTH, actualSamplingFrequency);

            // Compute the FFT
            fft.Compute(FFT_FORWARD);

            // Zero-out high frequencies in the FFT frequency-domain result
            int piFreqIndex = FFT_SIGNAL_LENGTH/2;
            int cutOff = round(cutOffFreq * 2 * piFreqIndex / actualSamplingFrequency);
            for (int freqI = 0; freqI < piFreqIndex - cutOff; freqI++) {
                    sFFT[piFreqIndex - freqI] = imag[piFreqIndex - freqI] = 0;
                    sFFT[piFreqIndex + freqI] = imag[piFreqIndex + freqI] = 0;
            }

            // Compute the iFFT - result is in the inner buffers
            fft.Compute(FFT_REVERSE);
        }


        // Move inner buffer data to external u16 type buffer
        void MoveDataToBufferU16(uint16_t signal[FFT_SIGNAL_LENGTH]) {
            for (size_t i = 0; i < FFT_SIGNAL_LENGTH; i++)
            {
                signal[i] = round(sFFT[i]);
            }
        }

        // Move inner buffer data to external float type buffer
        void MoveDataToBufferF(float signal[FFT_SIGNAL_LENGTH]) {
            for (size_t i = 0; i < FFT_SIGNAL_LENGTH; i++)
            {
                signal[i] = round(sFFT[i]);
            }
        }
};