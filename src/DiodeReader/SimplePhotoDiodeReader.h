#include<inttypes.h>

class SimplePhotoDiodeReader {

    public:
        SimplePhotoDiodeReader() {}

        void read(auto d, uint16_t * dest) {
            *dest = analogRead(d);
        }
};