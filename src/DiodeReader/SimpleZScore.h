#include<inttypes.h>
#include<Arduino.h>

class SimpleZScore {

    public:
        SimpleZScore() {}

        void ComputeZScore(float * signal, int length, bool to1) {
            float mean = 0, stdev = 0;
            int index = 0;
            int non0Elements = 0;

            // Mean
            while(index++ < length) {
                if (signal[index] != 0) {
                    mean += signal[index];
                    non0Elements++;
                }
            }
            mean /= non0Elements;

            // Compute stddev
            index = 0;
            while(index++ < length) 
                if (signal[index] != 0)
                    stdev += pow(signal[index] - mean, 2);
            stdev = pow(stdev, 0.5);

            // Compute the Z-score and put it in the destination
            index = 0;
            if (to1) {
                float max = 0;
                while(index < length) {
                    if (signal[index] != 0)
                        signal[index] = ((signal[index] - mean) / stdev + 1) / 2.0f;
                        if (signal[index] > max) max = signal[index]; 
                    index++;
                }
                index = 0;
                while(index < length) {
                    if(signal[index] != 0) signal[index] /= max;
                    index++;
                }
            }
            else 
                while(index++ < length) {
                    if (signal[index] != 0)
                        signal[index] = (signal[index] - mean) / stdev;
                    index++;
                }
        }

        void ComputeZScore(float * signal1, float * signal2, int length, bool to1) {
            float mean = 0, stdev = 0;
            int index = 0;
            int non0Elements = 0;

            // Mean
            while(index < length) {
                if(signal1[index] != 0) {
                    mean += signal1[index];
                    non0Elements++;
                }
                if(signal2[index] != 0) {
                    mean += signal2[index];
                    non0Elements++;
                }
                index++;
            }

            mean /= non0Elements;

            // Compute stddev
            index = 0;
            while(index < length) {
                if(signal1[index] != 0) {
                    stdev += pow(signal1[index] - mean, 2);
                }
                if(signal2[index] != 0) {
                    stdev += pow(signal2[index] - mean, 2);
                }
                index++;
            }
            stdev = pow(stdev, 0.5);

            // Compute the Z-score and put it in the destination
            index = 0;
            if (to1)
                while(index < length) {
                    if(signal1[index] != 0)
                        signal1[index] = ((signal1[index] - mean) / stdev + 1) / 2.0f;
                    if(signal2[index] != 0) 
                        signal2[index] = ((signal2[index] - mean) / stdev + 1) / 2.0f;                    
                    index++;
                }
            else 
                while(index < length) {
                    if(signal1[index] != 0)
                        signal1[index] = (signal1[index] - mean) / stdev;
                    if(signal2[index] != 0) 
                        signal2[index] = (signal2[index] - mean) / stdev;                
                    index++;
                }
        }
};