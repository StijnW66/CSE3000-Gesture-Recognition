#include"../parameters.h"

class MaxNormaliser
{

public:
    MaxNormaliser() {}

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