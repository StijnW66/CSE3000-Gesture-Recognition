#include<Arduino.h>
#include<inttypes.h>

#define DEBUG

class PDReader {

    public:
        PDReader() {}

        void read(auto d, uint16_t * dest) {
            *dest = analogRead(d);
        }
};

class GestureEdgeDetector {

    private: 
        int detectionWindowLength;
        int threshold;

    public: 
        GestureEdgeDetector(int detWL, int t) : detectionWindowLength(detWL), threshold(t) {}

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

class Z_Score {

    public:
        Z_Score() {}

        void ComputeZScore(uint16_t * signal, uint16_t * dest, int length) {
            uint16_t mean = 0, stdev = 0;
            int index = 0;

            // Mean
            while(index++ < length) mean += signal[index];
            mean /= length;

            // Compute stddev
            index = 0;
            while(index++ < length) stdev += pow(signal[index] - mean, 2);
            stdev = pow(stdev, 0.5);

            // Compute the Z-score and put it in the destination
            index = 0;
            while(index++ < length) dest[index] = (signal[index] - mean) / stdev;
        }

};


#define PD1 A0
#define PD2 A1

#define DETECTION_WINDOW_LENGTH 10
#define READING_WINDOW_LENGTH 1000
#define DETECTION_THRESHOLD 700
#define READ_PERIOD 5

uint16_t detectionWindow[DETECTION_WINDOW_LENGTH];
uint16_t photodiodeData[2][READING_WINDOW_LENGTH];

Z_Score zScoreCalculator;
PDReader reader;
GestureEdgeDetector edgeDetector(DETECTION_WINDOW_LENGTH, DETECTION_THRESHOLD);


void printSignal(uint16_t * signal, int length) {
    int count = 0;

    while(count++ < length) {
        int c = 0;
        while(c++ < signal[count])
            Serial.println("-");
    }
}


void setup() {

    pinMode(PD1, INPUT);
    pinMode(PD2, INPUT);

    Serial.begin(9600);
}

void loop() {

    int count = DETECTION_WINDOW_LENGTH;
    int gestureSignalLength;
    uint16_t * photodiodeData1 = photodiodeData[0];
    uint16_t * photodiodeData2 = photodiodeData[1];

    while(count-- > 0) {
        reader.read(PD1, photodiodeData1++);
        reader.read(PD2, photodiodeData2++);

        delay(READ_PERIOD);
    }

    // count2 = 0;
    // while(count2++ < DETECTION_WINDOW_LENGTH) {
    //         Serial.print(photodiodeData[0][count2]);
    //         Serial.print(" ");
    //         Serial.println(photodiodeData[1][count2]);
    // }

    if (edgeDetector.DetectStart(photodiodeData2 - 1)) {  
        Serial.println("Gesture Detected");

        bool endDetected = false;

        while(count++ < READING_WINDOW_LENGTH) {
            reader.read(PD2, photodiodeData2++);

            delay(READ_PERIOD);

            if(edgeDetector.DetectEnd(photodiodeData2 - 1)) {
                endDetected = true;

                // Determine the gesture length
                gestureSignalLength = photodiodeData2 - photodiodeData[1];

                Serial.print("End of Gesture Detected. Samples in the gesture ");
                Serial.println(photodiodeData2 - photodiodeData[1]);

                // Flip the signal
                int index = 0;
                while(index++ < gestureSignalLength) photodiodeData[1][index] = abs(photodiodeData[1][index] - DETECTION_THRESHOLD);

                // Normalize with the Z-score
                zScoreCalculator.ComputeZScore(photodiodeData[0], photodiodeData[0], gestureSignalLength);
                zScoreCalculator.ComputeZScore(photodiodeData[1], photodiodeData[1], gestureSignalLength);

                break;
            }
        }

        if(!endDetected) {
            Serial.println("Gesture took more samples then space allowed");
        }

    } else {
        Serial.println("Gesture Not Detected");
    }

    // Serial.print("A0: ");
    // Serial.println(analogRead(PD1));
    // Serial.print("A1: ");
    // Serial.println(analogRead(PD2));
    delay(READ_PERIOD);

}