#include "BMx280.h"
#include "math.h"

BMx280_calibration_data calibration_data;
BMx280_sensor_settings_t BMx280_sensor_settings;

I2C_TypeDef *BMx280_hi2c;

int32_t BMx280_t_fine;
uint32_t BMx280_refPressure;
uint32_t BMx280_ADDRESS;

int32_t __BMx280_compensate_T_int32(int32_t adc_T) {
	int32_t var1, var2, T;

	var1 = ((((adc_T >> 3) - ((int32_t)calibration_data.dig_T1 << 1))) * ((int32_t)calibration_data.dig_T2)) >> 11;
	var2 = (((((adc_T >> 4) - ((int32_t)calibration_data.dig_T1)) * ((adc_T >> 4) - ((int32_t)calibration_data.dig_T1))) >> 12) *
		((int32_t)calibration_data.dig_T3)) >> 14;
	
	BMx280_t_fine = var1 + var2;
	
	T = (BMx280_t_fine * 5 + 128) >> 8;
	
	return T;
}

uint32_t __BMx280_compensate_P_int64(int32_t adc_P) {
	int64_t var1, var2, p;

	var1 = ((int64_t)BMx280_t_fine) - 128000;

	var2 = var1 * var1 * (int64_t)calibration_data.dig_P6;
	var2 = var2 + ((var1 * (int64_t)calibration_data.dig_P5) << 17);
	var2 = var2 + (((int64_t)calibration_data.dig_P4) << 35);

	var1 = ((var1 * var1 * (int64_t)calibration_data.dig_P3) >> 8) + ((var1 * (int64_t)calibration_data.dig_P2) << 12);
	var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)calibration_data.dig_P1) >> 33;

	if (var1 == 0)
	{
		return 0; // avoid exception caused by division by zero
	}

	p = 1048576 - adc_P;
	p = (((p << 31) - var2) * 3125) / var1;

	var1 = (((int64_t)calibration_data.dig_P9) * (p >> 13) * (p >> 13)) >> 25;
	var2 = (((int64_t)calibration_data.dig_P8) * p) >> 19;

	p = ((p + var1 + var2) >> 8) + (((int64_t)calibration_data.dig_P7) << 4);

	return (uint32_t)p;
}

int32_t __BMx280_compensate_H_int32(int32_t adc_H)
{
	int32_t v_x1_u32r;

	v_x1_u32r = (BMx280_t_fine - ((int32_t)76800));
	v_x1_u32r = (((((adc_H << 14) - (((int32_t)calibration_data.dig_H4) << 20) - (((int32_t)calibration_data.dig_H5) *
		v_x1_u32r)) + ((int32_t)16384)) >> 15) * (((((((v_x1_u32r *
		((int32_t)calibration_data.dig_H6)) >> 10) * (((v_x1_u32r * ((int32_t)calibration_data.dig_H3)) >> 11) +
		((int32_t)32768))) >> 10) + ((int32_t)2097152)) * ((int32_t)calibration_data.dig_H2) + 8192) >> 14));

	v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) * ((int32_t)calibration_data.dig_H1)) >> 4));
	v_x1_u32r = (v_x1_u32r < 0 ? 0 : v_x1_u32r);
	v_x1_u32r = (v_x1_u32r > 419430400 ? 419430400 : v_x1_u32r);

	return (uint32_t)(v_x1_u32r >> 12);
}

HAL_StatusTypeDef BMx280_init(I2C_TypeDef *hi2c_, uint32_t refPressure_) {
	HAL_StatusTypeDef status;
	uint8_t id;

	if ((status = I2C_Mem_Read(hi2c_, 0x76 << 1, BMx280_REGISTER_ID, &id, 1, 0xFF)) == HAL_OK) {
		BMx280_ADDRESS = 0x76 << 1;
	}
	else if ((status = I2C_Mem_Read(hi2c_, 0x77 << 1, BMx280_REGISTER_ID, &id, 1, 0xFF)) == HAL_OK) {
		BMx280_ADDRESS = 0x77 << 1;	
	}
	else {
		return status; // Оба адреса не ответили
	}

	if ((id == 0x60) || (id == 0x58))  {
		BMx280_hi2c = hi2c_;
		BMx280_refPressure = refPressure_;
		
		uint8_t H_bytes[3];

		// Чтение калибровочных данных температуры и давления
		if ((status = I2C_Mem_Read(BMx280_hi2c, BMx280_ADDRESS, BMx280_REGISTER_CALIBRATION, (uint8_t*)&calibration_data, 24, 0xFF)) != HAL_OK) {
			return status;
		}
		
		// Чтение калибровочных данных влажности (только для BME280)
		if (id == 0x60) {
			uint8_t dig_H1;
			if ((status = I2C_Mem_Read(BMx280_hi2c, BMx280_ADDRESS, 0xA1, &dig_H1, 1, 0xFF)) != HAL_OK) {
				return status;
			}
			calibration_data.dig_H1 = dig_H1;

			if ((status = I2C_Mem_Read(BMx280_hi2c, BMx280_ADDRESS, BMx280_REGISTER_CALIBRATION_H4, H_bytes, 3, 0xFF)) != HAL_OK) {
				return status;
			}
			calibration_data.dig_H4 = (H_bytes[0] << 4) | (H_bytes[1] & 0xF);
			calibration_data.dig_H5 = (H_bytes[2] << 4) | (H_bytes[1] >> 4);

			if ((status = I2C_Mem_Read(BMx280_hi2c, BMx280_ADDRESS, BMx280_REGISTER_CALIBRATION_H6, (uint8_t*)&calibration_data.dig_H6, 1, 0xFF)) != HAL_OK) {
				return status;
			}
		}

		return BMx280_config(1, 1, 1, 0, 0);
	}

	return HAL_ERROR; // Неизвестный ID датчика
}

HAL_StatusTypeDef BMx280_config(uint8_t T_OS, uint8_t P_OS, uint8_t H_OS, uint8_t STDB, uint8_t IIRF) {
	// Проверка границ параметров
	if (T_OS > 0b101) T_OS = 0b101;
	if (P_OS > 0b101) P_OS = 0b101;
	if (H_OS > 0b101) H_OS = 0b101;
	if (STDB > 0b111) STDB = 0b111;
	if (IIRF > 0b100) IIRF = 0b100;

	HAL_StatusTypeDef status;

	BMx280_sensor_settings.ctrl_meas = (T_OS << 5) | (P_OS << 2);
	BMx280_sensor_settings.config = (STDB << 5) | (IIRF << 2);
	BMx280_sensor_settings.ctrl_hum = H_OS;
	
	// Сначала устанавливаем ctrl_hum, затем config и ctrl_meas
	if ((status = I2C_Mem_Write(BMx280_hi2c, BMx280_ADDRESS, BMx280_REGISTER_CTRL_HUM, &BMx280_sensor_settings.ctrl_hum, 1, 0xFF)) != HAL_OK) {
		return status;
	}

	if ((status = I2C_Mem_Write(BMx280_hi2c, BMx280_ADDRESS, BMx280_REGISTER_CONFIG, &BMx280_sensor_settings.config, 1, 0xFF)) != HAL_OK) {
		return status;
	}
	
	if ((status = I2C_Mem_Write(BMx280_hi2c, BMx280_ADDRESS, BMx280_REGISTER_CTRL_MEAS, &BMx280_sensor_settings.ctrl_meas, 1, 0xFF)) != HAL_OK) {
		return status;
	}
	
	return status;
}

HAL_StatusTypeDef BME280_forced_measure(float *temp, float *press, float *hum, float *h) {
	HAL_StatusTypeDef status;
	uint8_t sensor_status;

	// Установка режима forced measurement
	uint8_t ctrl_meas = BMx280_sensor_settings.ctrl_meas;
	ctrl_meas = (ctrl_meas & 0xFC) | 0b01; // Forced mode
	
	if ((status = I2C_Mem_Write(BMx280_hi2c, BMx280_ADDRESS, BMx280_REGISTER_CTRL_MEAS, &ctrl_meas, 1, 0xFF)) != HAL_OK) {
		return status;
	}

	// Ожидание завершения измерения
	uint32_t timeout = 1000; // Таймаут для избежания бесконечного цикла
	do {
		HAL_Delay(10);
		if ((status = I2C_Mem_Read(BMx280_hi2c, BMx280_ADDRESS, BMx280_REGISTER_STATUS, &sensor_status, 1, 0xFF)) != HAL_OK) {
			return status;
		}
		timeout--;
	} while ((sensor_status & 0x08) && timeout > 0); // Бит 3 указывает на выполнение измерения

	if (timeout == 0) {
		return HAL_TIMEOUT;
	}

	// Чтение сырых данных
	uint8_t raw_data[8];
	if ((status = I2C_Mem_Read(BMx280_hi2c, BMx280_ADDRESS, BMx280_REGISTER_RAW_DATA, raw_data, 8, 0xFF)) != HAL_OK) {
		return status;
	}

	// Преобразование сырых данных
	int32_t ADC_data[3];
	ADC_data[0] = (int32_t)(((uint32_t)raw_data[0] << 12) | ((uint32_t)raw_data[1] << 4) | ((uint32_t)raw_data[2] >> 4)); // Давление
	ADC_data[1] = (int32_t)(((uint32_t)raw_data[3] << 12) | ((uint32_t)raw_data[4] << 4) | ((uint32_t)raw_data[5] >> 4)); // Температура
	ADC_data[2] = (int32_t)(((uint16_t)raw_data[6] << 8) | ((uint16_t)raw_data[7])); // Влажность

	// Компенсация и преобразование
	*temp = __BMx280_compensate_T_int32(ADC_data[1]) / 100.0f;
	*press = __BMx280_compensate_P_int64(ADC_data[0]) / 256.0f;
	*hum = __BMx280_compensate_H_int32(ADC_data[2]) / 1024.0f;

	// Вычисление высоты
	if (press > 0) {
		*h = 29.254f * ((*temp) + 273.15f) * logf(BMx280_refPressure / (*press));
	} else {
		*h = 0.0f;
	}

	return HAL_OK;
}

HAL_StatusTypeDef BMP280_forced_measure(float *temp, float *press, float *h) {
	HAL_StatusTypeDef status;
	uint8_t sensor_status;

	// Установка режима forced measurement
	uint8_t ctrl_meas = BMx280_sensor_settings.ctrl_meas;
	ctrl_meas = (ctrl_meas & 0xFC) | 0b01; // Forced mode
	
	if ((status = I2C_Mem_Write(BMx280_hi2c, BMx280_ADDRESS, BMx280_REGISTER_CTRL_MEAS, &ctrl_meas, 1, 0xFF)) != HAL_OK) {
		return status;
	}

	// Ожидание завершения измерения
	uint32_t timeout = 1000;
	do {
		HAL_Delay(10);
		if ((status = I2C_Mem_Read(BMx280_hi2c, BMx280_ADDRESS, BMx280_REGISTER_STATUS, &sensor_status, 1, 0xFF)) != HAL_OK) {
			return status;
		}
		timeout--;
	} while ((sensor_status & 0x08) && timeout > 0);

	if (timeout == 0) {
		return HAL_TIMEOUT;
	}

	// Чтение сырых данных
	uint8_t raw_data[6];
	if ((status = I2C_Mem_Read(BMx280_hi2c, BMx280_ADDRESS, BMx280_REGISTER_RAW_DATA, raw_data, 6, 0xFF)) != HAL_OK) {
		return status;
	}

	// Преобразование сырых данных
	int32_t ADC_data[2];
	ADC_data[0] = (int32_t)(((uint32_t)raw_data[0] << 12) | ((uint32_t)raw_data[1] << 4) | ((uint32_t)raw_data[2] >> 4)); // Давление
	ADC_data[1] = (int32_t)(((uint32_t)raw_data[3] << 12) | ((uint32_t)raw_data[4] << 4) | ((uint32_t)raw_data[5] >> 4)); // Температура

	// Компенсация и преобразование
	*temp = __BMx280_compensate_T_int32(ADC_data[1]) / 100.0f;
	*press = __BMx280_compensate_P_int64(ADC_data[0]) / 256.0f;

	// Вычисление высоты
	if (*press > 0) {
		*h = 29.254f * ((*temp) + 273.15f) * logf(BMx280_refPressure / (*press));
	} else {
		*h = 0.0f;
	}
	
	return HAL_OK;
}

HAL_StatusTypeDef BMx280_normal_measure() {
	uint8_t ctrl_meas = BMx280_sensor_settings.ctrl_meas;
	ctrl_meas = (ctrl_meas & 0xFC) | 0b11; // Normal mode
	
	return I2C_Mem_Write(BMx280_hi2c, BMx280_ADDRESS, BMx280_REGISTER_CTRL_MEAS, &ctrl_meas, 1, 0xFF);
}

HAL_StatusTypeDef BMx280_sleep() {
	uint8_t ctrl_meas = BMx280_sensor_settings.ctrl_meas;
	ctrl_meas = (ctrl_meas & 0xFC) | 0b00; // Sleep mode
	
	return I2C_Mem_Write(BMx280_hi2c, BMx280_ADDRESS, BMx280_REGISTER_CTRL_MEAS, &ctrl_meas, 1, 0xFF);
}

HAL_StatusTypeDef BME280_get_measure(float *temp, float *press, float *hum, float *h) {
	HAL_StatusTypeDef status;

	uint8_t raw_data[8];
	if ((status = I2C_Mem_Read(BMx280_hi2c, BMx280_ADDRESS, BMx280_REGISTER_RAW_DATA, raw_data, 8, 0xFF)) != HAL_OK) {
		return status;
	}
	
	int32_t ADC_data[3];
	ADC_data[0] = (int32_t)(((uint32_t)raw_data[0] << 12) | ((uint32_t)raw_data[1] << 4) | ((uint32_t)raw_data[2] >> 4)); // Давление
	ADC_data[1] = (int32_t)(((uint32_t)raw_data[3] << 12) | ((uint32_t)raw_data[4] << 4) | ((uint32_t)raw_data[5] >> 4)); // Температура
	ADC_data[2] = (int32_t)(((uint16_t)raw_data[6] << 8) | ((uint16_t)raw_data[7])); // Влажность

	*temp = __BMx280_compensate_T_int32(ADC_data[1]) / 100.0f;
	*press = __BMx280_compensate_P_int64(ADC_data[0]) / 256.0f;
	*hum = __BMx280_compensate_H_int32(ADC_data[2]) / 1024.0f;

	if (*press > 0) {
		*h = 29.254f * ((*temp) + 273.15f) * logf(BMx280_refPressure / (*press));
	} else {
		*h = 0.0f;
	}
	
	return HAL_OK;
}

HAL_StatusTypeDef BMP280_get_measure(float *temp, float *press, float *h) {
	HAL_StatusTypeDef status;

	uint8_t raw_data[6];
	if ((status = I2C_Mem_Read(BMx280_hi2c, BMx280_ADDRESS, BMx280_REGISTER_RAW_DATA, raw_data, 6, 0xFF)) != HAL_OK) {
		return status;
	}

	int32_t ADC_data[2];
	ADC_data[0] = (int32_t)(((uint32_t)raw_data[0] << 12) | ((uint32_t)raw_data[1] << 4) | ((uint32_t)raw_data[2] >> 4)); // Давление
	ADC_data[1] = (int32_t)(((uint32_t)raw_data[3] << 12) | ((uint32_t)raw_data[4] << 4) | ((uint32_t)raw_data[5] >> 4)); // Температура

	*temp = __BMx280_compensate_T_int32(ADC_data[1]) / 100.0f;
	*press = __BMx280_compensate_P_int64(ADC_data[0]) / 256.0f;

	if (*press > 0) {
		*h = 29.254f * ((*temp) + 273.15f) * logf(BMx280_refPressure / (*press));
	} else {
		*h = 0.0f;
	}
	
	return HAL_OK;
}
