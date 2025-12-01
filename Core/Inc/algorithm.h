#ifndef ALGORITHM_H
#define ALGORITHM_H

#include "stdbool.h"
#include "definitions.h"

enum statusFlight {
    WAITING = 0,
    FLIGHT,
    APOGEE,
    LANDING,
    GROUND
};

void initSensors();
void readSensors();
void checkApogee();
void writeSD();
bool activRS();
void sleepMode();

#endif