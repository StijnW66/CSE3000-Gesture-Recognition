#ifndef UTIL
#define UTIL

#include "Arduino.h"
#include "receiver-parameters.h"
#include "plotting/plotting-util.h"

#define FOR(i1, i2, L1, L2, codeline) for (int i1 = 0; i1 < L1; i1++) for (int i2 = 0; i2 < L2; i2++) codeline;

#endif