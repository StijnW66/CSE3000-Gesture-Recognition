#include<inttypes.h>
#include<arduinoFFT.h>

#include"parameters.h"
#include"SimpleSignalStretcher.h"

class SimpleFFTCutOffFilter {
    
    private:
        double sFFT[FFT_SIGNAL_LENGTH];
        double imag[FFT_SIGNAL_LENGTH];

        SimpleSignalStretcher str;
        arduinoFFT fft;

    public:
        SimpleFFTCutOffFilter() {}

        void ZeroImag() {
            for (size_t i = 0; i < FFT_SIGNAL_LENGTH; i++)
            {
                imag[i] = 0;
            }
        }

        void Filter(uint16_t * signal, int length, int cutOff, int samplingFrequency) {
            str.StretchSignal(signal, length, sFFT, FFT_SIGNAL_LENGTH);

            fft = arduinoFFT(sFFT, imag, FFT_SIGNAL_LENGTH, samplingFrequency);

            fft.Compute(FFT_FORWARD);

            int piFreqIndex = FFT_SIGNAL_LENGTH/2;

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