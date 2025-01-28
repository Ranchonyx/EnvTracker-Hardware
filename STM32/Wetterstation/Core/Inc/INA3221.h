/*
 * INA3221.h
 *
 *  Created on: Nov 5, 2024
 *      Author: erikl
 */

#ifndef INA3221_H
#define INA3221_H

#include "stm32l4xx_hal.h"

// Defaultaddress 0x40 or 0x41
#define INA3221_ADDRESS 0x40 << 1

// Registers
#define INA3221_REG_CONFIG                0x00
#define INA3221_REG_SHUNT_VOLTAGE_1       0x01
#define INA3221_REG_BUS_VOLTAGE_1         0x02
#define INA3221_REG_SHUNT_VOLTAGE_2       0x03
#define INA3221_REG_BUS_VOLTAGE_2         0x04
#define INA3221_REG_SHUNT_VOLTAGE_3       0x05
#define INA3221_REG_BUS_VOLTAGE_3         0x06
#define INA3221_REG_CRT_ALERT_LIMIT_1     0x07
#define INA3221_REG_WARN_ALERT_LIMIT_1    0x08
#define INA3221_REG_CRT_ALERT_LIMIT_2     0x09
#define INA3221_REG_WARN_ALERT_LIMIT_2    0x0A
#define INA3221_REG_CRT_ALERT_LIMIT_3     0x0B
#define INA3221_REG_WARN_ALERT_LIMIT_3    0x0C
#define INA3221_REG_SHUNT_VOLTAGE_SUM     0x0D
#define INA3221_REG_SHUNT_VOLTAGE_SUM_LIM 0x0E
#define INA3221_REG_MASK_ENABLE           0x0F
#define INA3221_REG_PWR_VALID_UPPER_LIM   0x10
#define INA3221_REG_PWR_VALID_LOWER_LIM   0x11
#define INA3221_REG_MANUFACTURER_ID       0xFE
#define INA3221_REG_DIE_ID                0xFF

// Functions
HAL_StatusTypeDef INA3221_Init(I2C_HandleTypeDef *hi2c);
int16_t INA3221_ReadShuntVoltage(I2C_HandleTypeDef *hi2c, uint8_t channel);
int16_t INA3221_ReadBusVoltage(I2C_HandleTypeDef *hi2c, uint8_t channel);
float INA3221_ReadCurrent(I2C_HandleTypeDef *hi2c, uint8_t channel, float shunt_resistance);
HAL_StatusTypeDef INA3221_WriteRegister(I2C_HandleTypeDef *hi2c, uint8_t reg, uint16_t value);
int16_t INA3221_ReadRegister(I2C_HandleTypeDef *hi2c, uint8_t reg);
HAL_StatusTypeDef INA3221_ReadAll(I2C_HandleTypeDef *hi2c, float shunt_resistance, float *bus_voltages, float *currents);

#endif // INA3221_H

