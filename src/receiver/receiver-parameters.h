#ifndef PARAMETERS 
#define PARAMETERS

/**
 * @brief Parameters for tuning the receiver operations.
 */

// Parameters for edge detection
#define DETECTION_BUFFER_LENGTH         10
#define DETECTION_WINDOW_LENGTH         5
#define DETECTION_END_WINDOW_LENGTH     100
#define DETECTION_END_WINDOW_TRIM       0.90f
#define DETECTION_THRESHOLD_COEFF       0.70f

// Parameters for threshold computation from stable signal value,
// trimming and cutting-off the signal data in the first stages
// of the pipeline.
#define CUTT_OFF_THRESHOLD_COEFF        (DETECTION_THRESHOLD_COEFF + (1 - DETECTION_THRESHOLD_COEFF) / 2.0f)
#define CUTT_OFF_THRESHOLD_COEFF_PRE_FFT    1.1f    
#define CUTT_OFF_THRESHOLD_COEFF_POST_FFT   (CUTT_OFF_THRESHOLD_COEFF_PRE_FFT - 1.0f)    

// Maximum gesture length
#define GESTURE_BUFFER_LENGTH           500
// Buffer for threshold adjustment
#define THRESHOLD_ADJ_BUFFER_LENGTH     100
#define THRESHOLD_UPD_BUFFER_LENGTH     20

// Sampling period and minimum expected gesture duration
#define READ_PERIOD                     10
#define GESTURE_MIN_TIME_MS             100

// Signal length to stretch to before using the FFT
// !!! Has to be a power of 2
#define FFT_SIGNAL_LENGTH         128

// Length of pre-processed data/pipeline output
#define ML_DATA_LENGTH 100

// Number of photodiodes used by the system
#define NUM_PDs 3

#ifdef USE_ARDUINO
    // Arduino pins
    // TODO: Change macro names to LEFT, RIGHT, UP
    #define PD1 A0
    #define PD2 A1
    #define PD3 A2

    // Used to save and adjust which column-index is associated with each photodiode position.
    uint8_t pds[NUM_PDs] = {
        PD3,
        PD2,
        PD1
    };

// #define DEBUG_RECEIVER
// #define PLOT_RECEIVER

#endif

#endif