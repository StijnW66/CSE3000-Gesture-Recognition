#include<Arduino.h>
#include<inttypes.h>
#include<iostream>
#include<vector>
#include<algorithm>
#include<QuickMedianLib.h>

class SimpleHampel {

    private:
        int windowSize;
        float k = 1.4826;
        std::vector<uint16_t> window;

    public:
        SimpleHampel(int windowSize) : windowSize(windowSize) {
            window = std::vector<uint16_t>(2*windowSize + 1);
        }
        
        void filter1(uint16_t * signal) {

            for (int i = 0; i < 2*windowSize + 1; i++ ) {
                window[i] = signal[i];
            }

            uint16_t med = QuickMedian<uint16_t>::GetMedian(window.data(), 2*windowSize + 1);

            for (int i = 0; i < 2*windowSize + 1; i++ ) {
                window[i] = abs(window[i] - med);
            }

            uint16_t msd = QuickMedian<uint16_t>::GetMedian(window.data(), 2*windowSize + 1);

            if(abs(signal[windowSize] - med) > 3 * k * msd) {
                signal[windowSize] = med;
            }

        }
};