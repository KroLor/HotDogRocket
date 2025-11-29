#include "definitions.h"
#include "led.h"
#include "BMx280.h"
#include "fatfs.h"

float pressure, temperature, altitude;
float altitude_bf;

void initSensors() {
    ledState lState;

    // BMP280
    if (BMx280_init(&I2C_BMP280, 101325) == HAL_OK) { lState.BMP280 = 1; }
    BMx280_normal_measure();

    // SD
    FATFS FatFs;
    FIL Fil;
    FRESULT FR_Status;

    if (f_mount(&FatFs, "", 1) == FR_OK) { lState.SD = 1; }

    FR_Status = f_open(&Fil, "Logs.txt", FA_WRITE | FA_CREATE_ALWAYS);
    // if(FR_Status != FR_OK) {}
    f_puts("/// SD-CARD INIT ///\n", &Fil); // Write to file
    f_close(&Fil);

    // TS


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