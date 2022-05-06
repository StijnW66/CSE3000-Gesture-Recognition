#include<inttypes.h>
#include<Arduino.h>

class SimpleZScore {

    public:
        SimpleZScore() {}

        void ComputeZScore(uint16_t * signal, float * dest, int length, bool to1) {
            float mean = 0, stdev = 0;
            int index = 0;

            // Mean
            while(index++ < length) mean += signal[index];
            mean /= length;

            // Compute stddev
            index = 0;
            while(index++ < length) stdev += pow(signal[index] - mean, 2);
            stdev = pow(stdev, 0.5);

            // Compute the Z-score and put it in the destination
            index = 0;
            if (to1)
                while(index++ < length) dest[index] = ((signal[index] - mean) / stdev + 1) / 2;
            else while(index++ < length) dest[index] = (signal[index] - mean) / stdev;
        }

};