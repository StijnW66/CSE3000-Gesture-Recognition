#ifndef SS
#define SS

#include<inttypes.h>

/**
 * @brief A class the implements horizontal stretching of a signal using
 *      Linear Interpolation. 
 * 
 */
class SignalStretcher {

    public:
        SignalStretcher() {}

        /**
         * @brief Stretch a signal from length \p sLength to length \p dLength using
         *      Linear Interpolation. Works with integer input data.
         * 
         * @param signal    - Signal data - integer type.
         * @param sLength   - Signal length.
         * @param dest      - Output buffer. Has to be of length at least \p dLength .
         * @param dLength   - Output length to stretch up/down to.
         */
        void StretchSignal(uint16_t * signal, int sLength, float * dest, int dLength) {

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
    
        /**
         * @brief Stretch a signal from length \p sLength to length \p dLength using
         *      Linear Interpolation. Works with float input data.
         * 
         * @param signal    - Signal data - float type.
         * @param sLength   - Signal length.
         * @param dest      - Output buffer. Has to be of length at least \p dLength .
         * @param dLength   - Output length to stretch up/down to.
         */
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

#endif