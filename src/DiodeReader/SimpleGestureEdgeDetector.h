#include<inttypes.h>

class SimpleGestureEdgeDetector {

    private: 
        int detectionWindowLength;
        int detectionEndWindowLength;
        int threshold;

    public: 
        SimpleGestureEdgeDetector() {}
        SimpleGestureEdgeDetector(int detWL, int detEWL, int t) : detectionWindowLength(detWL), detectionEndWindowLength(detEWL), threshold(t) {}

        bool DetectStart(uint16_t * signal) {
            int count = detectionWindowLength;

            while(count > 0) {
                if(*signal >= threshold) return false;
                signal--;
                count--;
            }

            return true;
        }

        bool DetectEnd(uint16_t * signal) {
            int count = detectionEndWindowLength;

            while(count > 0) {
                if(*(signal) < threshold) return false;
                signal--;
                count--;
            }

            return true;
        }

        void setThreshold(int t) {
            threshold = t;
        }

        int getThreshold() {
            return threshold;
        }

};