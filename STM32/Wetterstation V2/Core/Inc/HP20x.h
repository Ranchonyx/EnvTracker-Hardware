/*
 * HP20x.h
 *
 *  Created on: May 25, 2024
 *      Author: erikl
 */
#ifndef HP20X_H
#define HP20X_H

#include "stm32l4xx_hal.h"
#include "kalman.h"

// HP20x I2C address
#define HP20X_I2C_DEV_ID (0x76 << 1)

// HP20x commands
#define HP20X_CMD_SOFT_RST       0x06
#define HP20X_CMD_WR_CONVERT     0x40
#define HP20X_CONVERT_OSR4096    (0 << 2)
#define HP20X_CONVERT_OSR2048    (1 << 2)
#define HP20X_CONVERT_OSR1024    (2 << 2)
#define HP20X_CONVERT_OSR512     (3 << 2)
#define HP20X_CONVERT_OSR256     (4 << 2)
#define HP20X_CONVERT_OSR128     (5 << 2)

#define HP20X_CMD_READ_PRESSURE         0x30 // read_pressure command
#define HP20X_CMD_READ_ALTITUDE         0x31 // read_altitude command
#define HP20X_CMD_READ_TEMPERATURE      0x32 // read_temperature command
#define HP20X_CMD_READ_PPRES_TEMP       0x10 // read_pressure & temperature command
#define HP20X_CMD_READ_ALT_TEMP         0x11 // read_altitude & temperature command
#define HP20X_CMD_READ_CAL              0x28 // RE-CAL ANALOG

extern KalmanFilter t_filter;
extern KalmanFilter p_filter;
extern KalmanFilter a_filter;

void HP20x_Init(void);
void HP20x_ReadAllMeasuredValues(float* temperature, float* pressure, float* altitude);
void HP20x_SoftReset(void);
float HP20x_ReadTemperature(void);
float HP20x_ReadPressure(void);
float HP20x_ReadAltitude(void);

#endif /* HP20X_H */
