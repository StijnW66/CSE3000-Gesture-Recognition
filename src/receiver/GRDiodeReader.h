#include<inttypes.h>

class GRDiodeReader {

    public:
        GRDiodeReader() {}

        void read(uint8_t d, uint16_t * dest) {
            *dest = analogRead(d);
        }
};