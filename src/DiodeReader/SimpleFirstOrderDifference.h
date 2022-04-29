#include<inttypes.h>

class SimpleFirstOrderDifference {

    public:
        SimpleFirstOrderDifference() {}

        void computeFOD(uint16_t * signal, uint16_t * dest, int length) {
            while(--length >= 1) {
                dest[length-1] = signal[length] - signal[length - 1];
            }
        }

};