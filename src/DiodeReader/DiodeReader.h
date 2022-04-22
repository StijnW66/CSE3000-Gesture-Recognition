#include<Arduino.h>
#include<vector>

class DiodeReader {

    private:
        uint16_t intervalLength;
        uint16_t sampleFrequency;
        uint16_t samplePeriodMicroSec;
        uint8_t diode1;
        uint8_t diode2;

        size_t nSamplesToTake;
        std::vector<uint8_t> readings1;
        std::vector<uint8_t> readings2;

    public:
        DiodeReader(uint16_t inputIntervalLenSec, uint16_t sampleFrequencyPerSec, uint8_t diode1, uint8_t diode2) 
        : 
        intervalLength(inputIntervalLenSec),
        sampleFrequency(sampleFrequency),
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

        const uint8_t * getReadings1() {
            return readings1.data();
        }

        const uint8_t * getReadings2() {
            return readings2.data();
        }

        uint8_t discretiseReading(uint16_t reading, u_int8_t bins, uint16_t max_value) {
            return (reading  *  bins  / max_value);
        }

        void readAndUpdate() {
            for (size_t i = 0; i < nSamplesToTake; i++)
            {
                readings1[i] = 1 + discretiseReading(analogRead(diode1), 10, 820);
                readings2[i] = 1 + discretiseReading(analogRead(diode2), 10, 820);

                delayMicroseconds(samplePeriodMicroSec);
            }
        }

        void readAndUpdateDebug(auto ser) {
            for (size_t i = 0; i < nSamplesToTake; i++)
            {
                readings1[i] = 1 + discretiseReading(analogRead(diode1), 10, 820);
                readings2[i] = 1 + discretiseReading(analogRead(diode2), 10, 820);

                ser.print("Read values: ");
                ser.print(readings1[i]);
                ser.print(" ");
                ser.print(readings2[i]);
                ser.println();

                delayMicroseconds(samplePeriodMicroSec);
            }
            ser.println("----- Finished Reading");
        }

        void filterReadings() {
            
        }

};