#include "definitions.h"
#include "led.h"
#include "sd.h"

float pressure, temperature, altitude;
float altitude_bf;

void initSensors() {
    ledState lState;

    lState.BMP280 = BMx280_init(I2C_BMP280, 101325);
    BMx280_normal_measure();

    lState.SD = sd_init(SPI_SD);

    // ts

    setLEDState(lState);
}

void readSensors() {
    // BMP280_get_measure()
}

void checkApogee() {

}

void writeSD() {

}

void activRS() {

}

void sleepMode() {

}