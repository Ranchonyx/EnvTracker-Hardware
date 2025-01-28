/*
 * SHT4x.c
 *
 *  Created on: May 27, 2024
 *      Author: erikl
 */
#include "SHT4x.h"
#include "crc.h"

extern I2C_HandleTypeDef hi2c1;

void SHT4X_Init(void) {
  // No specific initialization command for SHT4X, just ensure the I2C bus is initialized
}

void SHT4X_SoftReset(void) {
  uint8_t cmd = SHT4X_CMD_SOFT_RESET;
  HAL_I2C_Master_Transmit(&hi2c1, SHT4X_I2C_DEV_ID, &cmd, 1, HAL_MAX_DELAY);
  HAL_Delay(1); // Wait for the reset to complete
}

void SHT4X_ReadSerialNumber(char* serial_number) {
  uint8_t cmd = SHT4X_CMD_READ_SERIAL_NUMBER;
  uint8_t buffer[6];

  HAL_I2C_Master_Transmit(&hi2c1, SHT4X_I2C_DEV_ID, &cmd, 1, HAL_MAX_DELAY);
  HAL_Delay(1); // Wait for the sensor to respond
  HAL_I2C_Master_Receive(&hi2c1, SHT4X_I2C_DEV_ID, buffer, 6, HAL_MAX_DELAY);

  // Verify CRC for each 2-byte data packet
  for (int i = 0; i < 6; i += 3) {
    uint8_t data[2] = { buffer[i], buffer[i + 1] };
    uint8_t crc = buffer[i + 2];
    if (CalcCrc(data) != crc) {
      // Handle CRC error
      return;
    }
    serial_number[i / 3 * 2] = data[0];
    serial_number[i / 3 * 2 + 1] = data[1];
  }
}

void SHT4X_Measure(float* temperature, float* humidity, uint8_t cmd, int* MeasurementDuration) {
  uint8_t buffer[6];

  HAL_I2C_Master_Transmit(&hi2c1, SHT4X_I2C_DEV_ID, &cmd, 1, HAL_MAX_DELAY);
  HAL_Delay(MeasurementDuration); // Wait for the measurement to complete
  HAL_I2C_Master_Receive(&hi2c1, SHT4X_I2C_DEV_ID, buffer, 6, HAL_MAX_DELAY);

  // Verify CRC for each 2-byte data packet
  for (int i = 0; i < 6; i += 3) {
    uint8_t data[2] = { buffer[i], buffer[i + 1] };
    uint8_t crc = buffer[i + 2];
    if (CalcCrc(data) != crc) {
      // Handle CRC error
      return;
    }
  }

  // Parse the buffer data
  uint16_t temperature_raw = (buffer[0] << 8) | buffer[1];
  uint16_t humidity_raw = (buffer[3] << 8) | buffer[4];

  // Convert raw values to meaningful units
  *temperature = -45 + 175 * ((float)temperature_raw / 65535.0);
  *humidity = -6 + 125 * ((float)humidity_raw / 65535.0);
}

void SHT4X_MeasureHighPrecision(float* temperature, float* humidity) {
  SHT4X_Measure(temperature, humidity, SHT4X_CMD_MEASURE_HIGH_PRECISION, 10);
}

void SHT4X_MeasureMediumPrecision(float* temperature, float* humidity) {
  SHT4X_Measure(temperature, humidity, SHT4X_CMD_MEASURE_MEDIUM_PRECISION, 5);
}

void SHT4X_MeasureLowestPrecision(float* temperature, float* humidity) {
  SHT4X_Measure(temperature, humidity, SHT4X_CMD_MEASURE_LOWEST_PRECISION, 2);
}

void SHT4X_MeasureHeated200mW_1000ms(float* temperature, float* humidity) {
  SHT4X_Measure(temperature, humidity, SHT4X_CMD_M_H_P_HEATED_200mW_1000ms, 1110);
}

void SHT4X_MeasureHeated200mW_100ms(float* temperature, float* humidity) {
  SHT4X_Measure(temperature, humidity, SHT4X_CMD_M_H_P_HEATED_200mW_100ms, 110);
}

void SHT4X_MeasureHeated110mW_1000ms(float* temperature, float* humidity) {
  SHT4X_Measure(temperature, humidity, SHT4X_CMD_M_H_P_HEATED_110mW_1000ms, 1110);
}

void SHT4X_MeasureHeated110mW_100ms(float* temperature, float* humidity) {
  SHT4X_Measure(temperature, humidity, SHT4X_CMD_M_H_P_HEATED_110mW_100ms, 110);
}

void SHT4X_MeasureHeated20mW_1000ms(float* temperature, float* humidity) {
  SHT4X_Measure(temperature, humidity, SHT4X_CMD_M_H_P_HEATED_20mW_1000ms, 1110);
}

void SHT4X_MeasureHeated20mW_100ms(float* temperature, float* humidity) {
  SHT4X_Measure(temperature, humidity, SHT4X_CMD_M_H_P_HEATED_20mW_100ms, 110);
}


