#ifndef UTIL
#define UTIL

#include "Arduino.h"
#include "parameters.h"

template<typename T, int bl>
void sendSignal(T signal[NUM_PDs][bl], int length) {
    Serial.println("Start");
    for (int i = 0; i < length; i++)
    {
        for (size_t pdId = 0; pdId < NUM_PDs; pdId++)
        {
            Serial.print(signal[pdId][i]);
            Serial.print(" ");
        }
        Serial.println();
    }
    Serial.println("Done");
}

#endif