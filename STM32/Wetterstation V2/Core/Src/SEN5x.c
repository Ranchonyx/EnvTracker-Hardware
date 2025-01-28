/*
 * SEN5x.c
 *
 *  Created on: May 27, 2024
 *      Author: erikl
 */
#include "SEN5X.h"
#include "crc.h"

extern I2C_HandleTypeDef hi2c1;

void SEN5X_Init(void) {
  // No specific initialization command for SHT4X, just ensure the I2C bus is initialized
}

void SEN5X_StartMeasurement(void) {
  uint8_t cmd[] = { (SEN5X_CMD_START_MEASUREMENT >> 8) & 0xFF, SEN5X_CMD_START_MEASUREMENT & 0xFF };
  HAL_I2C_Master_Transmit(&hi2c1, SEN5X_I2C_DEV_ID, cmd, 2, HAL_MAX_DELAY);
//  HAL_Delay(50); // Wait for the sensor to start measurement
}

void SEN5X_StopMeasurement(void) {
  uint8_t cmd[] = { (SEN5X_CMD_STOP_MEASUREMENT >> 8) & 0xFF, SEN5X_CMD_STOP_MEASUREMENT & 0xFF };
  HAL_I2C_Master_Transmit(&hi2c1, SEN5X_I2C_DEV_ID, cmd, 2, HAL_MAX_DELAY);
//  HAL_Delay(200); // Wait for the sensor to respond
}

uint8_t SEN5X_ReadDataReadyFlag(void) {
  uint8_t cmd[] = { (SEN5X_CMD_READ_DATAREADY_FLAG >> 8) & 0xFF, SEN5X_CMD_READ_DATAREADY_FLAG & 0xFF };
  uint8_t buffer[3];

  HAL_I2C_Master_Transmit(&hi2c1, SEN5X_I2C_DEV_ID, cmd, 2, HAL_MAX_DELAY);
  HAL_Delay(20); // Wait for the sensor to respond
  HAL_I2C_Master_Receive(&hi2c1, SEN5X_I2C_DEV_ID, buffer, 3, HAL_MAX_DELAY);

  if (CalcCrc(buffer) != buffer[2]) {
    // Handle CRC error
    return 0;
  }

  return buffer[1];
}

void SEN5X_ReadAllMeasuredValues(float* pm1p0, float* pm2p5, float* pm4p0, float* pm10, float* humidity, float* temperature, float* voc_index, float* nox_index) {
  uint8_t cmd[] = { (SEN5X_CMD_READ_MEASURED_VALUES >> 8) & 0xFF, SEN5X_CMD_READ_MEASURED_VALUES & 0xFF };
  uint8_t buffer[24];

  HAL_I2C_Master_Transmit(&hi2c1, SEN5X_I2C_DEV_ID, cmd, 2, HAL_MAX_DELAY);
  HAL_Delay(20); // Wait for the sensor to prepare data
  HAL_I2C_Master_Receive(&hi2c1, SEN5X_I2C_DEV_ID, buffer, 24, HAL_MAX_DELAY);

  // Verify CRC for each 2-byte data packet
  for (int i = 0; i < 24; i += 3) {
    uint8_t data[2] = { buffer[i], buffer[i+1] };
    uint8_t crc = buffer[i+2];
    if (CalcCrc(data) != crc) {
      // Handle CRC error
      return;
    }
  }

  // Parse the buffer data
  uint16_t pm1p0_raw = (buffer[0] << 8) | buffer[1];
  uint16_t pm2p5_raw = (buffer[3] << 8) | buffer[4];
  uint16_t pm4p0_raw = (buffer[6] << 8) | buffer[7];
  uint16_t pm10_raw = (buffer[9] << 8) | buffer[10];
  int16_t humidity_raw = (buffer[12] << 8) | buffer[13];
  int16_t temperature_raw = (buffer[15] << 8) | buffer[16];
  int16_t voc_raw = (buffer[18] << 8) | buffer[19];
  int16_t nox_raw = (buffer[21] << 8) | buffer[22];

  // Convert raw values to meaningful units
  *pm1p0 = pm1p0_raw / 10.0;
  *pm2p5 = pm2p5_raw / 10.0;
  *pm4p0 = pm4p0_raw / 10.0;
  *pm10 = pm10_raw / 10.0;
  *humidity = humidity_raw / 100.0;
  *temperature = temperature_raw / 200.0;
  *voc_index = voc_raw / 10.0;
  *nox_index = nox_raw / 10.0;
}

void SEN5X_StartFanClean(void) {
  uint8_t cmd[] = { (SEN5X_CMD_START_FAN_CLEAN >> 8) & 0xFF, SEN5X_CMD_START_FAN_CLEAN & 0xFF };
  HAL_I2C_Master_Transmit(&hi2c1, SEN5X_I2C_DEV_ID, cmd, 2, HAL_MAX_DELAY);
}

void SEN5X_ReadProductName(char* product_name) {
  uint8_t cmd[] = { (SEN5X_CMD_READ_PRODUCT_NAME >> 8) & 0xFF, SEN5X_CMD_READ_PRODUCT_NAME & 0xFF };
  uint8_t buffer[48];

  HAL_I2C_Master_Transmit(&hi2c1, SEN5X_I2C_DEV_ID, cmd, 2, HAL_MAX_DELAY);
  HAL_Delay(20); // Wait for the sensor to prepare data
  HAL_I2C_Master_Receive(&hi2c1, SEN5X_I2C_DEV_ID, buffer, 48, HAL_MAX_DELAY);

  // Copy the data to the product_name buffer
  for (int i = 0; i < 48; i += 3) {
    uint8_t data[2] = { buffer[i], buffer[i+1] };
    uint8_t crc = buffer[i+2];
    if (CalcCrc(data) != crc) {
      // Handle CRC error
      return;
    }
    product_name[i / 3 * 2] = data[0];
    product_name[i / 3 * 2 + 1] = data[1];
  }
}

void SEN5X_ReadSerialNumber(char* serial_number) {
  uint8_t cmd[] = { (SEN5X_CMD_READ_SERIAL_NUMBER >> 8) & 0xFF, SEN5X_CMD_READ_SERIAL_NUMBER & 0xFF };
  uint8_t buffer[24];

  HAL_I2C_Master_Transmit(&hi2c1, SEN5X_I2C_DEV_ID, cmd, 2, HAL_MAX_DELAY);
  HAL_Delay(20); // Wait for the sensor to prepare data
  HAL_I2C_Master_Receive(&hi2c1, SEN5X_I2C_DEV_ID, buffer, 24, HAL_MAX_DELAY);

  // Copy the data to the serial_number buffer
  for (int i = 0; i < 24; i += 3) {
    uint8_t data[2] = { buffer[i], buffer[i+1] };
    uint8_t crc = buffer[i+2];
    if (CalcCrc(data) != crc) {
      // Handle CRC error
      return;
    }
    serial_number[i / 3 * 2] = data[0];
    serial_number[i / 3 * 2 + 1] = data[1];
  }
}

void SEN5X_ReadFirmwareVersion(char* firmware_version) {
  uint8_t cmd[] = { (SEN5X_CMD_READ_FIRMWARE_VERSION >> 8) & 0xFF, SEN5X_CMD_READ_FIRMWARE_VERSION & 0xFF };
  uint8_t buffer[6];

  HAL_I2C_Master_Transmit(&hi2c1, SEN5X_I2C_DEV_ID, cmd, 2, HAL_MAX_DELAY);
  HAL_Delay(20); // Wait for the sensor to prepare data
  HAL_I2C_Master_Receive(&hi2c1, SEN5X_I2C_DEV_ID, buffer, 6, HAL_MAX_DELAY);

  // Copy the data to the firmware_version buffer
  for (int i = 0; i < 6; i += 3) {
    uint8_t data[2] = { buffer[i], buffer[i+1] };
    uint8_t crc = buffer[i+2];
    if (CalcCrc(data) != crc) {
      // Handle CRC error
      return;
    }
    firmware_version[i / 3 * 2] = data[0];
    firmware_version[i / 3 * 2 + 1] = data[1];
  }
}

uint16_t SEN5X_ReadDeviceStatus(void) {
  uint8_t cmd[] = { (SEN5X_CMD_READ_DEVICE_STATUS >> 8) & 0xFF, SEN5X_CMD_READ_DEVICE_STATUS & 0xFF };
  uint8_t buffer[3];

  HAL_I2C_Master_Transmit(&hi2c1, SEN5X_I2C_DEV_ID, cmd, 2, HAL_MAX_DELAY);
  HAL_Delay(20); // Wait for the sensor to prepare data
  HAL_I2C_Master_Receive(&hi2c1, SEN5X_I2C_DEV_ID, buffer, 3, HAL_MAX_DELAY);

  if (CalcCrc(buffer) != buffer[2]) {
    // Handle CRC error
    return 0;
  }

  return (buffer[0] << 8) | buffer[1];
}

void SEN5X_ClearDeviceStatus(void) {
  uint8_t cmd[] = { (SEN5X_CMD_CLEAR_DEVICE_STATUS >> 8) & 0xFF, SEN5X_CMD_CLEAR_DEVICE_STATUS & 0xFF };
  HAL_I2C_Master_Transmit(&hi2c1, SEN5X_I2C_DEV_ID, cmd, 2, HAL_MAX_DELAY);
}

void SEN5X_Reset(void) {
  uint8_t cmd[] = { (SEN5X_CMD_RESET >> 8) & 0xFF, SEN5X_CMD_RESET & 0xFF };
  HAL_I2C_Master_Transmit(&hi2c1, SEN5X_I2C_DEV_ID, cmd, 2, HAL_MAX_DELAY);
}

