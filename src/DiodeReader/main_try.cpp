#include<Arduino.h>
#include<inttypes.h>

#define PD1 A0
#define PD2 A1

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

#define DETECTION_WINDOW_LENGTH 10
#define READING_WINDOW_LENGTH 100

uint16_t detectionWindow[DETECTION_WINDOW_LENGTH];
uint16_t photodiodeData[2][READING_WINDOW_LENGTH];

PDReader reader;
GestureEdgeDetector edgeDetector(DETECTION_WINDOW_LENGTH, 700);

void setup() {

    pinMode(PD1, INPUT);
    pinMode(PD2, INPUT);

    Serial.begin(9600);
}

void loop() {

    int count = DETECTION_WINDOW_LENGTH;
    uint16_t * photodiodeData1 = photodiodeData[0];
    uint16_t * photodiodeData2 = photodiodeData[1];

    while(count-- > 0) {
        reader.read(PD1, photodiodeData1++);
        reader.read(PD2, photodiodeData2++);

        delay(5);
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

            delay(5);

            if(edgeDetector.DetectEnd(photodiodeData2 - 1)) {
                endDetected = true;
                Serial.print("End of Gesture Detected. Samples in the gesture ");
                Serial.println(photodiodeData2 - photodiodeData[1]);
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
    delay(10);

}