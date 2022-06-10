class SignalFlipper {

    public: 
        SignalFlipper() {}

        template<typename T>        
        void FlipSignal(T * signal, int length, T low, T upper) {
            for (int i = 0; i < length; i++)
            {
                signal[i] = low + upper - signal[i];
            }            
        }

};