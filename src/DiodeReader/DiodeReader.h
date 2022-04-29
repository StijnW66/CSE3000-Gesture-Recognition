#include<Arduino.h>
#include<vector>
#include<functional>
#include<algorithm>
#include<numeric>

class DiodeReader {

    private:
        uint16_t intervalLength;
        uint16_t sampleFrequency;
        uint16_t samplePeriodMicroSec;
        uint8_t diode1;
        uint8_t diode2;

        size_t nSamplesToTake;
        std::vector<uint8_t> readings1;
        std::vector<uint8_t> readings1P;

        std::vector<uint8_t> readings2;
        std::vector<uint8_t> readings2P;

    public:
        DiodeReader(uint16_t inputIntervalLenSec, uint16_t sampleFrequencyPerSec, uint8_t diode1, uint8_t diode2) 
        : 
        intervalLength(inputIntervalLenSec),
        sampleFrequency(sampleFrequencyPerSec),
        diode1(diode1),
        diode2(diode2)
        {
            pinMode(diode1, INPUT);
            pinMode(diode2, INPUT);

            samplePeriodMicroSec = 1000000 / sampleFrequencyPerSec;
            nSamplesToTake = inputIntervalLenSec * sampleFrequencyPerSec;
            readings1.reserve(nSamplesToTake);
            readings2.reserve(nSamplesToTake);
        }

        uint8_t * getReadings1() {
            return readings1P.data();
        }

        uint8_t * getReadings2() {
            return readings2P.data();
        }

        size_t getLength() {
            return readings1.size();
        }

        uint8_t discretiseReading(uint16_t reading, u_int8_t bins, uint16_t max_value) {
            return (reading  *  bins  / max_value);
        }

        void readAndUpdate(std::function<void(uint8_t, uint8_t)> funUtil = [](uint8_t a, uint8_t b){}) {
            for (size_t i = 0; i < nSamplesToTake; i++)
            {
                readings1[i] = discretiseReading(analogRead(diode1), 255, 820);
                readings2[i] = discretiseReading(analogRead(diode2), 255, 820);

                funUtil(readings1[i], readings2[i]);

                delayMicroseconds(samplePeriodMicroSec);
            }

            // applyHampel1();
            // applyHampel2();
        }

        void applyHampel1() {
            // copy the readings
            readings1P = readings1;

            std::array<uint8_t, 7> window;
            auto b = window.begin();
            auto e = window.end();

            uint8_t median, stdev, mean, curR;

            for (size_t i = 3; i < readings1P.size() - 3; i++) {
                mean = 0;
                stdev = 0;
                curR = readings1P[i];

                for (size_t j = i-3; j < i+3; j++) {
                    uint8_t r = readings1P[j];
                    window[j - i + 3] = r;
                    mean += r;
                }

                std::sort(b, e);

                // Median
                median = window[3];

                // Mean
                mean /= 7;

                // Stddev
                stdev = std::inner_product(window.begin(), window.end(), window.begin(), 0.0);
                stdev = sqrt(stdev / 7);
                
                // Remove outliers
                if(abs(curR - median) >= 3*stdev) {

                    readings1P[i] = median;

                }
            }
        }

        void applyHampel2() {
            // copy the readings
            readings2P = readings2;

            std::array<uint8_t, 7> window;
            auto b = window.begin();
            auto e = window.end();

            uint8_t median, stdev, mean, curR;

            for (int i = 3; i < readings2P.size() - 3; i++) {
                mean = 0;
                stdev = 0;
                curR = readings2P[i];

                for (int j = i-3; j < i+3; j++) {
                    uint8_t r = readings2P[j];
                    window[j - i + 3] = r;
                    mean += r;
                }

                std::sort(b, e);

                // Median
                median = window[3];

                // Mean
                mean /= 7;

                // Stddev
                stdev = std::inner_product(window.begin(), window.end(), window.begin(), 0.0);
                stdev = sqrt(stdev / 7);
                
                // Remove outliers
                if(abs(curR - median) >= 3*stdev) {

                    readings2P[i] = median;

                }
            }
        }

        void normalise() {
            
        }
};