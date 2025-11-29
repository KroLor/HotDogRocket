#include "led.h"
#include "definitions.h"

void setLEDState(ledState state) {
    HAL_GPIO_WritePin(GPIO_LED_BPM280, GPIO_PIN_LED_BPM280, state.BMP280);

    HAL_GPIO_WritePin(GPIO_LED_SD, GPIO_PIN_LED_SD, state.SD);

    HAL_GPIO_WritePin(GPIO_LED_TS, GPIO_PIN_LED_TS, state.ts);
}