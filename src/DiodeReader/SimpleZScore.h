#include<inttypes.h>
#include<Arduino.h>

class SimpleZScore {

    public:
        SimpleZScore() {}

        void ComputeZScore(float * signal, int length, bool to1) {
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
            if (to1) {
                float max = 0;
                while(index < length) {
                    signal[index] = ((signal[index] - mean) / stdev + 1) / 2.0f;
                    if (signal[index] > max) max = signal[index]; 
                    index++;
                }
                index = 0;
                // while(index < length) {
                //     signal[index] /= max;
                //     index++;
                // }
            }
            else 
                while(index++ < length) {
                    signal[index] = (signal[index] - mean) / stdev;
                    index++;
                }
            

        }

        void ComputeZScore(float * signal1, float * signal2, int length, bool to1) {
            float mean = 0, stdev = 0;
            int index = 0;

            // Mean
            while(index < length) {
                mean += (signal1[index] + signal2[index]);
                index++;
            }

            mean /= (2*length);

            // Compute stddev
            index = 0;
            while(index < length) {
                stdev += (pow(signal1[index] - mean, 2) + pow(signal2[index] - mean, 2));
                index++;
            }
            stdev = pow(stdev, 0.5);

            // Compute the Z-score and put it in the destination
            index = 0;
            if (to1)
                while(index < length) {
                    signal1[index] = ((signal1[index] - mean) / stdev + 1) / 2.0f;
                    signal2[index] = ((signal2[index] - mean) / stdev + 1) / 2.0f;                    
                    index++;
                }
            else 
                while(index < length) {
                    signal1[index] = (signal1[index] - mean) / stdev;
                    signal2[index] = (signal2[index] - mean) / stdev;
                    index++;
                }
        }
};