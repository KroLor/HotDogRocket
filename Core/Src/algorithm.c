#include "algorithm.h"
#include "definitions.h"
#include "led.h"
#include "BMx280.h"
#include "fatfs.h"
#include "adc.h"
#include "tim.h"

#define AVERAGE_INITIAL_PRESSURE 100525
#define NTC_SERIES_RESISTOR 10000.0f
#define NTC_NOMINAL_RESISTANCE 10000.0f
#define NTC_NOMINAL_TEMP 25.0f
#define NTC_B_COEFFICIENT 3950.0f
#define ADC_MAX_VALUE 4095.0f

FATFS FatFs;
FIL Fil; // File
FRESULT FR_Status;

float pressure, temperature, altitude;
float altitude_bf;

float calcTemp(uint16_t adc_value) {
    float res = NTC_SERIES_RESISTOR * (adc_value / ADC_MAX_VALUE / (1.0f - adc_value / ADC_MAX_VALUE));
    float temp;

    temp = res / NTC_NOMINAL_RESISTANCE;
    temp = log(temp);
    temp /= NTC_B_COEFFICIENT;
    temp += 1.0f / (NTC_NOMINAL_TEMP + 273.15f);
    temp = 1.0f / temp;
    temp -= 273.15f;

    return temp;
}

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
    HAL_ADC_Start_IT(&hadc1);
    HAL_TIM_Base_Start_IT(&htim8);

    setLEDState(lState);
}

void readSensors() {
    // BMP280
    BMP280_get_measure(&temperature, &pressure, &altitude);
}

void checkApogee() {

}

void writeSD() {
    
}

bool activRS() {

}

void sleepMode() {
    HAL_ADC_Stop_IT(&hadc1);
    HAL_TIM_Base_Stop(&htim2);
    HAL_TIM_Base_Stop_IT(&htim3);
    HAL_TIM_Base_Stop_IT(&htim4);
    HAL_TIM_Base_Stop_IT(&htim5);
    HAL_TIM_Base_Stop_IT(&htim8);
    HAL_TIM_Base_Stop_IT(&htim9);
    HAL_TIM_Base_Stop_IT(&htim12);
}