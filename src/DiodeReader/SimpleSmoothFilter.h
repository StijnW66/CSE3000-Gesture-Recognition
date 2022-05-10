class SimpleSmoothFilter {

    public:
        SimpleSmoothFilter() {}

        void SmoothSignal(float * signal, int length, int windSide) {
            int windSize = 2 * windSide + 1;
            float window[windSize];

            int si = windSide;
            int indexToPlace = 0;
            int wi = 0;
            float avg = 0;
            
            // Load the initial window
            // Keep first and last signal data without filtering
            do {
                window[indexToPlace] = signal[indexToPlace];
            } while(++indexToPlace < windSize);

            indexToPlace = windSize - 1;

            // Average in the window
            // Put in destination
            // Put the new value in the window
            while(si < length - windSide) {

                window[indexToPlace] = signal[si + windSide];
                indexToPlace = (indexToPlace + 1) % windSize;

                while(wi < windSize) {
                    avg += window[wi++];
                }

                avg /= windSize;

                signal[si++] = avg;
                avg = 0;
                wi = 0;
            }

        }
    
};