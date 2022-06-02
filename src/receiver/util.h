#ifndef UTIL
#define UTIL

#include "Arduino.h"
#include "parameters.h"

#define FOR(i1, i2, L1, L2, codeline) for (int i1 = 0; i1 < L1; i1++) for (int i2 = 0; i2 < L2; i2++) codeline;

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

template<typename T, int bl>
void sendSignal1(T signal[bl], int length) {
    Serial.println("Start");
    for (int i = 0; i < length; i++)
    {
        Serial.println(signal[i]);
    }
    Serial.println("Done");
}



#endif