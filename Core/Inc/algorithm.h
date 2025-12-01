#ifndef ALGORITHM_H
#define ALGORITHM_H

#include "stdbool.h"

float temp[5] = {0.0f};

enum statusFlight {
    WAITING = 0,
    FLIGHT,
    APOGEE,
    LANDING,
    GROUND
};

float calcTemp(uint16_t adc_value);
void initSensors();
void readSensors();
void checkApogee();
void writeSD();
bool activRS();
void sleepMode();

#endif