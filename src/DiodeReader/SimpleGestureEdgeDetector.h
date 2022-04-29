#include<inttypes.h>

class SimpleGestureEdgeDetector {

    private: 
        int detectionWindowLength;
        int threshold;

    public: 
        SimpleGestureEdgeDetector(int detWL, int t) : detectionWindowLength(detWL), threshold(t) {}

        bool DetectStart(uint16_t * signal) {
            int count = detectionWindowLength;

            while(count-- > 0) {
                if(*(signal--) >= threshold) return false;
            }

            return true;
        }

        bool DetectEnd(uint16_t * signal) {
            int count = detectionWindowLength;

            while(count-- > 0) {
                if(*(signal--) < threshold) return false;
            }

            return true;
        }

};