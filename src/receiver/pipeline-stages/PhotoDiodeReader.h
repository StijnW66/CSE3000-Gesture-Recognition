#include<inttypes.h>

class PhotoDiodeReader {

    public:
        PhotoDiodeReader() {}

        void read(uint8_t d, uint16_t * dest) {
            *dest = analogRead(d);
        }
};