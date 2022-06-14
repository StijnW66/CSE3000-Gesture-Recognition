#ifndef ML_ARDUINO_PREDICTION_ENUMS_H
#define ML_ARDUINO_PREDICTION_ENUMS_H

enum Gesture {
    SWIPE_UP = 6, SWIPE_DOWN = 3, SWIPE_LEFT = 4, SWIPE_RIGHT = 5,
    TAP_SINGLE = 7, TAP_DOUBLE = 2,
    ROTATE_CLOCKWISE = 0, ROTATE_COUNTERCLOCKWISE = 1,
    UNIDENTIFIED = 69
};

#endif
