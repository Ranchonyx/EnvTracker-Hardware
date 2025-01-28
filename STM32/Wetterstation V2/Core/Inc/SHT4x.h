/*
 * SHT4x.h
 *
 *  Created on: May 27, 2024
 *      Author: erikl
 */

#ifndef INC_SHT4X_H_
#define INC_SHT4X_H_

#include "stm32l4xx_hal.h"

// SHT4x I2C address
#define SHT4X_I2C_DEV_ID (0x44 << 1)

// SHT4x commands
#define SHT4X_CMD_MEASURE_HIGH_PRECISION 0xFD
#define SHT4X_CMD_MEASURE_MEDIUM_PRECISION 0xF6
#define SHT4X_CMD_MEASURE_LOWEST_PRECISION 0xE0
#define SHT4X_CMD_READ_SERIAL_NUMBER 0x89
#define SHT4X_CMD_SOFT_RESET 0x94
#define SHT4X_CMD_M_H_P_HEATED_200mW_1000ms 0x39
#define SHT4X_CMD_M_H_P_HEATED_200mW_100ms 0x32
#define SHT4X_CMD_M_H_P_HEATED_110mW_1000ms 0x2F
#define SHT4X_CMD_M_H_P_HEATED_110mW_100ms 0x24
#define SHT4X_CMD_M_H_P_HEATED_20mW_1000ms 0x1E
#define SHT4X_CMD_M_H_P_HEATED_20mW_100ms 0x15

void SHT4X_Init(void);
void SHT4X_SoftReset(void);
void SHT4X_ReadSerialNumber(char* serial_number);
void SHT4X_MeasureHighPrecision(float* temperature, float* humidity);
void SHT4X_MeasureMediumPrecision(float* temperature, float* humidity);
void SHT4X_MeasureLowestPrecision(float* temperature, float* humidity);
void SHT4X_MeasureHeated200mW_1000ms(float* temperature, float* humidity);
void SHT4X_MeasureHeated200mW_100ms(float* temperature, float* humidity);
void SHT4X_MeasureHeated110mW_1000ms(float* temperature, float* humidity);
void SHT4X_MeasureHeated110mW_100ms(float* temperature, float* humidity);
void SHT4X_MeasureHeated20mW_1000ms(float* temperature, float* humidity);
void SHT4X_MeasureHeated20mW_100ms(float* temperature, float* humidity);

#endif /* INC_SHT4X_H_ */
