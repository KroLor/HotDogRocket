#ifndef ALGORITHM_H
#define ALGORITHM_H

#include "stdbool.h"

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

void activRS();
bool checkRS();

void sleepMode();

#endif