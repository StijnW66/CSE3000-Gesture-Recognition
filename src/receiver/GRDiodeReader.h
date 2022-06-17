#include<inttypes.h>

#include "receiver-parameters.h"

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

        template<int size> 
        void readAll(uint16_t dest[NUM_PDs][size],int index) {
            for (size_t i = 0; i < NUM_PDs; i++)
            {
                this->read(pds[i], &dest[i][index]);
            }
        }
};