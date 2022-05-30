#include<inttypes.h>

class SimpleSignalStretcher {

    public:
        SimpleSignalStretcher() {}

        void StretchSignal(uint16_t * signal, int sLength, double * dest, int dLength) {

            int i = 0;
            float coeff = (float)(sLength-1) / (dLength-1);

            while(i < dLength) {

                float index = i == 0 ? i : i == dLength - 1 ? sLength - 1 : i * coeff;
                int low = (int)index;
                int high = ceil(index);

                dest[i] = (index - low) * signal[high] + (low + 1 - index) * signal[low];

                i++;
            }

        }
    
        void StretchSignal(float * signal, int sLength, float * dest, int dLength) {

            int i = 0;
            float coeff = (float)(sLength-1) / (dLength-1);

            while(i < dLength) {

                float index = i == 0 ? i : i == dLength - 1 ? sLength - 1 : i * coeff;
                int low = (int)index;
                int high = ceil(index);

                dest[i] = (index - low) * signal[high] + (low + 1 - index) * signal[low];

                i++;
            }

        }
};