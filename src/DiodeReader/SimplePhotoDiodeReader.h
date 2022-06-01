#include<inttypes.h>

class SimplePhotoDiodeReader {

    public:
        SimplePhotoDiodeReader() {}

        void read(uint8_t d, uint16_t * dest) {
            *dest = analogRead(d);
        }
};