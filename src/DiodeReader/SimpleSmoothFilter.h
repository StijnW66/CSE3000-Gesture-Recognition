#include<vector>
#include<Arduino.h>

class SimpleSmoothFilter {

    public:
        SimpleSmoothFilter() {}

        void SmoothSignal(float * signal, int length, int windSide, float weights[]) {
            int windSize = 2 * windSide + 1;
            float window[windSize];
            float weightsWindow[windSize];

            int si = windSide;
            int indexToPlace = 0;
            int wi = 0;
            float avg = 0;
            
            // Load the initial window
            // Keep first and last signal data without filtering
            do {
                window[indexToPlace] = signal[indexToPlace];
                weightsWindow[indexToPlace] = weights[indexToPlace];
            } while(++indexToPlace < windSize);

            indexToPlace = windSize - 1;

            // Average in the window
            // Put in destination
            // Put the new value in the window
            while(si < length - windSide) {

                window[indexToPlace] = signal[si + windSide];
                weightsWindow[indexToPlace] = weights[(si + windSide) % windSize];
                indexToPlace = (indexToPlace + 1) % windSize;

                while(wi < windSize) {
                    avg += weightsWindow[wi] * window[wi];
                    Serial.println(avg);
                    wi++;
                }

                signal[si++] = avg;
                avg = 0;
                wi = 0;
            }
        }

        void SmoothSignal(float * signal, float * dest, int length, int windSide, float weights[]) {
            int windSize = 2 * windSide + 1;
            float window[windSize];
            float weightsWindow[windSize];

            int si = windSide;
            int indexToPlace = 0;
            int wi = 0;
            float avg = 0;
            
            // Load the initial window
            // Keep first and last signal data without filtering
            do {
                window[indexToPlace] = signal[indexToPlace];
                weightsWindow[indexToPlace] = weights[indexToPlace];
            } while(++indexToPlace < windSize);

            indexToPlace = windSize - 1;

            // Average in the window
            // Put in destination
            // Put the new value in the window
            while(si < length - windSide) {

                avg = 0;

                window[indexToPlace] = signal[si + windSide];
                indexToPlace = (indexToPlace + 1) % windSize;                

                while(wi < windSize) {
                    avg += (weightsWindow[wi] * window[wi]);
                    wi++;
                }

                weightsWindow[0] = weightsWindow[windSize-1];
                for (size_t i = 1; i < windSize; i++)
                {
                    weightsWindow[i] = weights[i-1];
                }

                dest[si++] = avg;
                wi = 0;
            }
        }

        void SmoothSignal(uint16_t * signal, float * dest, int length, int windSide, float weights[]) {
            int windSize = 2 * windSide + 1;
            float window[windSize];
            float weightsWindow[windSize];

            int si = windSide;
            int indexToPlace = 0;
            int wi = 0;
            float avg = 0;
            
            // Load the initial window
            // Keep first and last signal data without filtering
            do {
                window[indexToPlace] = signal[indexToPlace];
                weightsWindow[indexToPlace] = weights[indexToPlace];
            } while(++indexToPlace < windSize);

            indexToPlace = windSize - 1;

            // Average in the window
            // Put in destination
            // Put the new value in the window
            while(si < length - windSide) {

                avg = 0;

                window[indexToPlace] = signal[si + windSide];
                indexToPlace = (indexToPlace + 1) % windSize;                

                while(wi < windSize) {
                    avg += (weightsWindow[wi] * window[wi]);
                    wi++;
                }

                weightsWindow[0] = weightsWindow[windSize-1];
                for (size_t i = 1; i < windSize; i++)
                {
                    weightsWindow[i] = weights[i-1];
                }

                dest[si++] = avg;
                wi = 0;
            }
        }
};