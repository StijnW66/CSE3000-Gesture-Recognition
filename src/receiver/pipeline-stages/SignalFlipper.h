/**
 * @brief A class implementing signals flipping from one range to another.
 * 
 */
class SignalFlipper {

    public: 
        SignalFlipper() {}

        /**
         * @brief Flip a signal of some data type. The input signal
         *      is in the range [0, MAXIMUM] and the output is in
         *      the range [ \p low, \p upper ].
         * 
         * @tparam T        - Signal data type. 
         * @param signal    - Signal data.
         * @param length    - Signal length.
         * @param low       - The lower limit of the output range.
         * @param upper     - The upper limit of the output range.
         */
        template<typename T>        
        void FlipSignal(T * signal, int length, T low, T upper) {
            for (int i = 0; i < length; i++)
            {
                signal[i] = low + upper - signal[i];
            }            
        }

};