
class SimpleMaxNormaliser
{

public:
    SimpleMaxNormaliser() {}

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
};