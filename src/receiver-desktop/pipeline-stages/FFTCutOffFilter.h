#include<inttypes.h>
#include<arduinoFFT.h>

#include"../receiver-parameters.h"
#include"SignalStretcher.h"

class FFTCutOffFilter {
    
    private:
        double sFFT[FFT_SIGNAL_LENGTH];
        double imag[FFT_SIGNAL_LENGTH];

        SignalStretcher str;
        arduinoFFT fft;

    public:
        FFTCutOffFilter() {}

        void ZeroImag() {
            for (size_t i = 0; i < FFT_SIGNAL_LENGTH; i++)
            {
                imag[i] = 0;
            }
        }

        void Filter(uint16_t * signal, int length, int cutOffFreq, int samplingFrequency) {
            str.StretchSignal(signal, length, sFFT, FFT_SIGNAL_LENGTH);
            
            double actualSamplingFrequency = (samplingFrequency * length * 1.0) / FFT_SIGNAL_LENGTH;

            fft = arduinoFFT(sFFT, imag, FFT_SIGNAL_LENGTH, actualSamplingFrequency);

            fft.Compute(FFT_FORWARD);

            int piFreqIndex = FFT_SIGNAL_LENGTH/2;

            int cutOff = round(cutOffFreq * 2 * piFreqIndex / actualSamplingFrequency);

            for (int freqI = 0; freqI < piFreqIndex - cutOff; freqI++) {
                    sFFT[piFreqIndex - freqI] = imag[piFreqIndex - freqI] = 0;
                    sFFT[piFreqIndex + freqI] = imag[piFreqIndex + freqI] = 0;
            }

            fft.Compute(FFT_REVERSE);
        }

        void MoveDataToBufferU16(uint16_t signal[FFT_SIGNAL_LENGTH]) {
            for (size_t i = 0; i < FFT_SIGNAL_LENGTH; i++)
            {
                signal[i] = round(sFFT[i]);
            }
        }

        void MoveDataToBufferF(float signal[FFT_SIGNAL_LENGTH]) {
            for (size_t i = 0; i < FFT_SIGNAL_LENGTH; i++)
            {
                signal[i] = round(sFFT[i]);
            }
        }
};