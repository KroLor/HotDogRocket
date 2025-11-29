#ifndef LED_H
#define LED_H

#include <stdbool.h>

typedef struct ledState {
    bool BMP280;
    bool SD;
    bool ts;
} ledState;

void setLEDState(ledState state);

#endif