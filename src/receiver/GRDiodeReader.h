#include<inttypes.h>

/**
 * @brief A class abstracting the reading of sensors and storing their input.
 * 
 */
class GRDiodeReader {

    public:
        GRDiodeReader() {}

        void read(uint8_t d, uint16_t * dest) {
            *dest = analogRead(d);
        }
};