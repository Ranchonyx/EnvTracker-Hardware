/*
 * INA3221.c
 *
 *  Created on: Nov 5, 2024
 *      Author: erikl
 */

#include "INA3221.h"

// Initialize INA3221 with default configuration
HAL_StatusTypeDef INA3221_Init(I2C_HandleTypeDef *hi2c) {
    uint16_t config = 0x7127;  // Example configuration, adjust as needed
    return INA3221_WriteRegister(hi2c, INA3221_REG_CONFIG, config);
}

// Function to write a 16-bit value to a specific register
HAL_StatusTypeDef INA3221_WriteRegister(I2C_HandleTypeDef *hi2c, uint8_t reg, uint16_t value) {
    uint8_t data[3] = {reg, (value >> 8) & 0xFF, value & 0xFF};
    return HAL_I2C_Master_Transmit(hi2c, INA3221_ADDRESS, data, 3, HAL_MAX_DELAY);
}

// Function to read a 16-bit value from a specific register
int16_t INA3221_ReadRegister(I2C_HandleTypeDef *hi2c, uint8_t reg) {
    uint8_t data[2];
    if (HAL_I2C_Master_Transmit(hi2c, INA3221_ADDRESS, &reg, 1, HAL_MAX_DELAY) != HAL_OK)
        return -1;

    if (HAL_I2C_Master_Receive(hi2c, INA3221_ADDRESS, data, 2, HAL_MAX_DELAY) != HAL_OK)
        return -1;

    return (data[0] << 8) | data[1];
}

// Read shunt voltage form 1, 2, or 3
int16_t INA3221_ReadShuntVoltage(I2C_HandleTypeDef *hi2c, uint8_t channel) {
    return INA3221_ReadRegister(hi2c, INA3221_REG_SHUNT_VOLTAGE_1 + (channel - 1) * 2);
}

// Read bus voltage form 1, 2, or 3
int16_t INA3221_ReadBusVoltage(I2C_HandleTypeDef *hi2c, uint8_t channel) {
    return INA3221_ReadRegister(hi2c, INA3221_REG_BUS_VOLTAGE_1 + (channel - 1) * 2);
}

// Calculate current based on shunt voltage and resistance
float INA3221_ReadCurrent(I2C_HandleTypeDef *hi2c, uint8_t channel, float shunt_resistance) {
    int16_t shuntVoltage = INA3221_ReadShuntVoltage(hi2c, channel);

    if (shuntVoltage == -1) {
        return -1.0;  // Error reading shunt voltage
    }

    // The shunt voltage is in microvolts (uV)!!!. Convert to volts and calculate the current.
    float shuntVoltage_V = shuntVoltage * 0.00004;  // 40 uV per LSB
    return shuntVoltage_V / shunt_resistance;
}

// Read all voltages and currents for all channels
HAL_StatusTypeDef INA3221_ReadAll(I2C_HandleTypeDef *hi2c, float shunt_resistance, float *bus_voltages, float *currents) {
    for (uint8_t channel = 1; channel <= 3; channel++) {
        int16_t busVoltage = INA3221_ReadBusVoltage(hi2c, channel);
        int16_t shuntVoltage = INA3221_ReadShuntVoltage(hi2c, channel);

        if (busVoltage == -1 || shuntVoltage == -1)
            return HAL_ERROR;

        bus_voltages[channel - 1] = busVoltage * 0.001;  // Conversion to volts (1 mV per LSB)
        currents[channel - 1] = shuntVoltage * 0.00004 / shunt_resistance;  // Current in amperes
    }
    return HAL_OK;
}
