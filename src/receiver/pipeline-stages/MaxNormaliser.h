#include"../receiver-parameters.h"

/**
 * @brief A class that implements data normalisation through division by maximum.
 * 
 */
class MaxNormaliser
{

public:
    MaxNormaliser() {}

    /**
     * @brief Normalises a single signal by finding its maximum and dividing the signal by it.
     *      The result is expected to be in the range [0,1].
     * 
     * @param signal - Input signal to normalise. Expected to be in range [0, MAXIMUM].
     * @param length - Length of input signal.
     */
    void Normalise(float *signal, int length)
    {
        // Compute max
        int index = 0;
        float max = 0;
        while (index < length)
        {
            if (signal[index] > max)
                max = signal[index];
            index++;
        }
        
        index = 0;
        
        if (max == 0)
            return;

        // Normalise
        while (index < length)
        {
            signal[index] /= max;
            index++;
        }
    }

    /**
     * @brief Normalises a set of signals by finding the maximum in the total data and 
     *      dividing by it. The result is expected to be in the range [0,1].
     * 
     * @param signal - Array of input signals. 
     * @param length - Length of each independent signal.
     */
    void Normalise(float signal[NUM_PDs][GESTURE_BUFFER_LENGTH], int length) {
        // Compute max
        int index = 0;
        float max = 0;
        while (index < length)
        {
            for (size_t i = 0; i < NUM_PDs; i++)
            {
                /* code */
            
            if (signal[i][index] > max)
                max = signal[i][index];
            index++;
            }
        }
        
        index = 0;
        
        if (max == 0)
            return;

        // Normalise
        while (index < length)
        {
            for (size_t i = 0; i < NUM_PDs; i++)
                signal[i][index] /= max;
            index++;
        }

    }
};