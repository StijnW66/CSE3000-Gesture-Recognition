#ifndef PARAMETERS 
#define PARAMETERS

#define DETECTION_BUFFER_LENGTH         10
#define DETECTION_WINDOW_LENGTH         5
#define DETECTION_END_WINDOW_LENGTH     100
#define DETECTION_END_WINDOW_TRIM       0.90f
#define DETECTION_THRESHOLD_COEFF       0.70f
#define CUTT_OFF_THRESHOLD_COEFF        1

#define GESTURE_BUFFER_LENGTH           500
#define THRESHOLD_ADJ_BUFFER_LENGTH     100

#define READ_PERIOD                     10
#define GESTURE_MIN_TIME_MS             100

#define READING_WINDOW_LENGTH   200
#define ML_DATA_LENGTH 100

#define NUM_PDs 3

#define PD1 A0
#define PD2 A1
#define PD3 A2

uint8_t pds[NUM_PDs] = {
    PD1,
    PD2,
    PD3
};

#endif