#include "definitions.h"
#include "led.h"
#include "BMx280.h"
#include "fatfs.h"

#define AVERAGE_INITIAL_PRESSURE 100525

FATFS FatFs;
FIL Fil; // File
FRESULT FR_Status;

float pressure, temperature, altitude;
float altitude_bf;

void initSensors() {
    ledState lState;

    // BMP280
    if (BMx280_init(&I2C_BMP280, AVERAGE_INITIAL_PRESSURE) == HAL_OK) { lState.BMP280 = 1; }
    BMx280_normal_measure();

    // SD
    if (f_mount(&FatFs, "", 1) == FR_OK) { lState.SD = 1; }

    FR_Status = f_open(&Fil, "Logs.txt", FA_WRITE | FA_CREATE_ALWAYS);
    // if(FR_Status != FR_OK) {}
    f_puts("/// SD-CARD INIT ///\n", &Fil); // Write to file
    f_close(&Fil);

    // TS


    setLEDState(lState);
}

void readSensors() {
    // BMP280
    BMP280_get_measure(&temperature, &pressure, &altitude);

    // TS

}

void checkApogee() {

}

void writeSD() {
    
}

bool activRS() {

}

void sleepMode() {

}