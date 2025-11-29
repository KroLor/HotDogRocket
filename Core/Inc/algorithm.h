#ifndef ALGORITHM_H
#define ALGORITHM_H

#include "definitions.h"

enum statusFlight {
    WAITING,
    FLIGHT,
    APOGEE,
    LANDING,
    GROUND
};

void initSensors();
void readSensors();
void checkApogee();
void writeSD();
void activRS();
void sleepMode();

#endif