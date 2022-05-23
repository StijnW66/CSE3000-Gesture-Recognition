class SimpleSignalFlipper {

    public: 
        SimpleSignalFlipper() {}

        template<typename T>        
        void FlipSignal(T * signal, int length, T low, T upper) {
            for (size_t i = 0; i < length; i++)
            {
                signal[i] = low + upper - signal[i];
            }            
        }

};