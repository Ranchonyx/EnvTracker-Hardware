/*
 * HP20x.c
 *
 *  Created on: May 27, 2024
 *      Author: erikl
 */
#include "HP20x.h"
#include "crc.h"
#include "kalman.h"

extern I2C_HandleTypeDef hi2c1;

KalmanFilter t_filter;  // Temperaturfilter
KalmanFilter p_filter;  // Druckfilter
KalmanFilter a_filter;  // Höhenfilter

void HP20x_IIC_WriteCmd(uint8_t cmd) {
    HAL_I2C_Master_Transmit(&hi2c1, HP20X_I2C_DEV_ID, &cmd, 1, HAL_MAX_DELAY);
}

uint32_t HP20x_IIC_ReadData(uint8_t cmd) {
    uint8_t buffer[3];
    HAL_I2C_Master_Transmit(&hi2c1, HP20X_I2C_DEV_ID, &cmd, 1, HAL_MAX_DELAY);
    HAL_Delay(45); // Wait for the measurement to complete
    HAL_I2C_Master_Receive(&hi2c1, HP20X_I2C_DEV_ID, buffer, 3, HAL_MAX_DELAY);
    return ((uint32_t)buffer[0] << 16) | ((uint32_t)buffer[1] << 8) | buffer[2];
}

void HP20x_Init(void) {
    HP20x_IIC_WriteCmd(HP20X_CMD_SOFT_RST);
    HAL_Delay(100); // Wait for the sensor to reset

    // Send OSR and channel setting command
    uint8_t cmd = HP20X_CMD_WR_CONVERT | HP20X_CONVERT_OSR4096;
    HP20x_IIC_WriteCmd(cmd);
    HAL_Delay(1000); // Wait for the sensor to settle

    // Initialize Kalman filters
    KalmanFilter_Init(&t_filter, 0.022, 0.617, 1, 0);
    KalmanFilter_Init(&p_filter, 0.022, 0.617, 1, 0);
    KalmanFilter_Init(&a_filter, 0.022, 0.617, 1, 0);

    KalmanFilter_PreInitialize(&t_filter, HP20x_ReadTemperature);
    KalmanFilter_PreInitialize(&p_filter, HP20x_ReadPressure);
    KalmanFilter_PreInitialize(&a_filter, HP20x_ReadAltitude);
}

void HP20x_SoftReset(void) {
    HP20x_IIC_WriteCmd(HP20X_CMD_SOFT_RST);
    HAL_Delay(1000); // Wait for the sensor to reset
}

float HP20x_ReadPressure(void) {
    // Send OSR and channel setting command
    uint8_t cmd = HP20X_CMD_WR_CONVERT | HP20X_CONVERT_OSR4096;
    HP20x_IIC_WriteCmd(cmd);
    HAL_Delay(1000); // Wait for the sensor to settle
    return (float)HP20x_IIC_ReadData(HP20X_CMD_READ_PRESSURE) / 100.0;
}

float HP20x_ReadAltitude(void) {
    // Send OSR and channel setting command
    uint8_t cmd = HP20X_CMD_WR_CONVERT | HP20X_CONVERT_OSR4096;
    HP20x_IIC_WriteCmd(cmd);
    HAL_Delay(1000); // Wait for the sensor to settle
    return (float)HP20x_IIC_ReadData(HP20X_CMD_READ_ALTITUDE) / 100.0;
}

float HP20x_ReadTemperature(void) {
    // Send OSR and channel setting command
    uint8_t cmd = HP20X_CMD_WR_CONVERT | HP20X_CONVERT_OSR4096;
    HP20x_IIC_WriteCmd(cmd);
    HAL_Delay(1000); // Wait for the sensor to settle
    return (float)HP20x_IIC_ReadData(HP20X_CMD_READ_TEMPERATURE) / 100.0;
}

void HP20x_ReadAllMeasuredValues(float* temperature, float* pressure, float* altitude) {
    // Sende OSR und Kanal-Einstellungsbefehl
    uint8_t cmd = HP20X_CMD_WR_CONVERT | HP20X_CONVERT_OSR4096;
    HP20x_IIC_WriteCmd(cmd);
    HAL_Delay(1000); // Warten, bis sich der Sensor stabilisiert hat

    // Lese die Daten vom Sensor
    float rawPressure = (float)HP20x_IIC_ReadData(HP20X_CMD_READ_PRESSURE) / 100.0;
    float rawAltitude = (float)HP20x_IIC_ReadData(HP20X_CMD_READ_ALTITUDE) / 100.0;
    float rawTemperature = (float)HP20x_IIC_ReadData(HP20X_CMD_READ_TEMPERATURE) / 100.0;

    // Aktualisiere die Messwerte und wende den Kalman-Filter an (falls erforderlich)
    *temperature = KalmanFilter_Update(&t_filter, rawTemperature);
    *pressure = KalmanFilter_Update(&p_filter, rawPressure);
    *altitude = KalmanFilter_Update(&a_filter, rawAltitude);
}
