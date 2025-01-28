/*
 * SEN5X.h
 *
 *  Created on: May 27, 2024
 *      Author: erikl
 */

#ifndef INC_SEN5X_H_
#define INC_SEN5X_H_

#include "stm32l4xx_hal.h"

// SEN5x I2C address
#define SEN5X_I2C_DEV_ID (0x69 << 1) // I2C address for SEN55

// SEN5x commands
#define SEN5X_CMD_START_MEASUREMENT 0x0021
#define SEN5X_CMD_S_M_RHT_GAS_ONLY 0x0037
#define SEN5X_CMD_STOP_MEASUREMENT 0x0104
#define SEN5X_CMD_READ_DATAREADY_FLAG 0x0202
#define SEN5X_CMD_READ_MEASURED_VALUES 0x03C4
#define SEN5X_CMD_READWRITE_TEMP_COMPE_PARAM 0x60B2
#define SEN5X_CMD_READWRITE_WARM_START_PARAM 0x60C6
#define SEN5X_CMD_READWRITE_VOC_ALGO_TUNE_PARAM 0x60D0
#define SEN5X_CMD_READWRITE_NOX_ALGO_TUNE_PARAM 0x61E1
#define SEN5X_CMD_READWRITE_RHT_ACCELERATION_MODE 0x60F7
#define SEN5X_CMD_READWRITE_VOC_ALGO_STATE 0x6181
#define SEN5X_CMD_START_FAN_CLEAN 0x5607
#define SEN5X_CMD_READWRITE_AUTO_CLEAN_INTERVAL 0x8004
#define SEN5X_CMD_READ_PRODUCT_NAME 0xD014
#define SEN5X_CMD_READ_SERIAL_NUMBER 0xD033
#define SEN5X_CMD_READ_FIRMWARE_VERSION 0xD100
#define SEN5X_CMD_READ_DEVICE_STATUS 0xD206
#define SEN5X_CMD_CLEAR_DEVICE_STATUS 0xD210
#define SEN5X_CMD_RESET 0xD304

void SEN5X_Init(void);
void SEN5X_StartMeasurement(void);
void SEN5X_StopMeasurement(void);
uint8_t SEN5X_ReadDataReadyFlag(void);
void SEN5X_ReadAllMeasuredValues(float* pm1p0, float* pm2p5, float* pm4p0, float* pm10, float* humidity, float* temperature, float* voc_index, float* nox_index);
void SEN5X_StartFanClean(void);
void SEN5X_ReadProductName(char* product_name);
void SEN5X_ReadSerialNumber(char* serial_number);
void SEN5X_ReadFirmwareVersion(char* firmware_version);
void SEN5X_ClearDeviceStatus(void);
void SEN5X_Reset(void);

#endif /* INC_SEN5X_H_ */
