/*
 * SEN5x.c
 *
 *  Created on: May 27, 2024
 *      Author: erikl
 */
#include "command_processor.h"
#include "usart.h"
#include "LoRa_E5.h"
#include "HP20x.h"
#include "SEN5x.h"
#include "SHT4x.h"
#include "INA3221.h"
#include <string.h>
#include <stdio.h>

extern UART_HandleTypeDef huart2; // Debug UART
extern I2C_HandleTypeDef hi2c1;  // I2C für Sensoren
extern uint8_t interval_minutes; // Intervallzeit in Minuten

#define MAX_RESPONSE_SIZE 512

static void SendResponse(const char* response) {
    // Nachricht über LoRa senden
    LoRa_E5_SendMessage(response);

    // Debug-Ausgabe über UART2
    HAL_UART_Transmit(&huart2, (uint8_t*)response, strlen(response), HAL_MAX_DELAY);
}

void ProcessCommand(char* command) {
    char response[MAX_RESPONSE_SIZE] = {0};

    if (strncmp(command, "HELP", 4) == 0) {
        // Teile die Liste der Befehle in kleinere Blöcke
        snprintf(response, sizeof(response),
                 "Available Commands (1/2):\r\n"
                 "SET INTERVAL xx (in minutes)\r\n"
                 "READ SENSORS\r\n"
                 "HP20X TEMP\r\n"
                 "HP20X PRESS\r\n"
                 "HP20X ALT\r\n"
                 "INA3221 READ\r\n"
                 "INA3221 READ VOLTAGE x (x=1,2,3)\r\n"
                 "INA3221 READ CURRENT x (x=1,2,3)\r\n");
        SendResponse(response);

        snprintf(response, sizeof(response),
                 "Available Commands (2/2):\r\n"
                 "SEN5X START\r\n"
                 "SEN5X STOP\r\n"
                 "SEN5X DATAREADY\r\n"
                 "SEN5X READ\r\n"
                 "SEN5X FAN CLEAN\r\n"
                 "SEN5X PRODUCT NAME\r\n"
                 "SEN5X SERIAL NUMBER\r\n"
                 "SEN5X FIRMWARE VERSION\r\n"
                 "SEN5X CLEAR STATUS\r\n"
                 "SEN5X RESET\r\n"
                 "SHT4X HIGH PRECISION\r\n"
                 "SHT4X MEDIUM PRECISION\r\n"
                 "SHT4X LOW PRECISION\r\n"
                 "SHT4X HEATED 200 1000\r\n"
                 "SHT4X HEATED 200 100\r\n"
                 "SHT4X HEATED 110 1000\r\n"
                 "SHT4X HEATED 110 100\r\n"
                 "SHT4X HEATED 20 1000\r\n"
                 "SHT4X HEATED 20 100\r\n");
        SendResponse(response);
    }
    else if (strncmp(command, "SET INTERVAL", 12) == 0) {
        uint8_t interval;
        sscanf(command + 13, "%hhu", &interval);
        interval_minutes = interval;
        snprintf(response, sizeof(response), "Interval set to %d minutes\r\n", interval);
        SendResponse(response);

    } else if (strncmp(command, "READ SENSORS", 12) == 0) {
        // INA3221 Werte lesen
        float bus_voltages[3] = {0};
        float currents[3] = {0};
        float shunt_resistance = 1.0; // Beispielwert
        if (INA3221_ReadAll(&hi2c1, shunt_resistance, bus_voltages, currents) == HAL_OK) {
            snprintf(response, sizeof(response),
                     "INA3221: Bus Voltages: %.2fV, %.2fV, %.2fV; Currents: %.2fA, %.2fA, %.2fA\r\n",
                     bus_voltages[0], bus_voltages[1], bus_voltages[2],
                     currents[0], currents[1], currents[2]);
        } else {
            snprintf(response, sizeof(response), "Error reading INA3221 values.\r\n");
        }
        SendResponse(response);

    } else if (strncmp(command, "INA3221 READ VOLTAGE", 20) == 0) {
        uint8_t channel;
        sscanf(command + 21, "%hhu", &channel);
        if (channel >= 1 && channel <= 3) {
            float voltage = INA3221_ReadBusVoltage(&hi2c1, channel);
            snprintf(response, sizeof(response), "INA3221 Channel %d Voltage: %.2fV\r\n", channel, voltage);
        } else {
            snprintf(response, sizeof(response), "Invalid INA3221 channel. Use 1, 2, or 3.\r\n");
        }
        SendResponse(response);

    } else if (strncmp(command, "INA3221 READ CURRENT", 20) == 0) {
        uint8_t channel;
        sscanf(command + 21, "%hhu", &channel);
        if (channel >= 1 && channel <= 3) {
            float shunt_resistance = 1.0; // Beispielwert
            float current = INA3221_ReadCurrent(&hi2c1, channel, shunt_resistance);
            snprintf(response, sizeof(response), "INA3221 Channel %d Current: %.2fA\r\n", channel, current);
        } else {
            snprintf(response, sizeof(response), "Invalid INA3221 channel. Use 1, 2, or 3.\r\n");
        }
        SendResponse(response);

    } else if (strncmp(command, "HP20X TEMP", 10) == 0) {
        float temp = HP20x_ReadTemperature();
        snprintf(response, sizeof(response), "HP20X Temperature: %.2f C\r\n", temp);
        SendResponse(response);

    } else if (strncmp(command, "HP20X PRESS", 11) == 0) {
        float press = HP20x_ReadPressure();
        snprintf(response, sizeof(response), "HP20X Pressure: %.2f hPa\r\n", press);
        SendResponse(response);

    } else if (strncmp(command, "HP20X ALT", 9) == 0) {
        float alt = HP20x_ReadAltitude();
        snprintf(response, sizeof(response), "HP20X Altitude: %.2f m\r\n", alt);
        SendResponse(response);

    } else if (strncmp(command, "SEN5X START", 11) == 0) {
        SEN5X_StartMeasurement();
        snprintf(response, sizeof(response), "SEN5X Measurement Started\r\n");
        SendResponse(response);

    } else if (strncmp(command, "SEN5X STOP", 10) == 0) {
        SEN5X_StopMeasurement();
        snprintf(response, sizeof(response), "SEN5X Measurement Stopped\r\n");
        SendResponse(response);

    } else if (strncmp(command, "SEN5X READ", 10) == 0) {
        float pm1p0, pm2p5, pm4p0, pm10, humidity, temperature, voc_index, nox_index;
        SEN5X_ReadAllMeasuredValues(&pm1p0, &pm2p5, &pm4p0, &pm10, &humidity, &temperature, &voc_index, &nox_index);
        snprintf(response, sizeof(response),
                 "SEN5X: PM1.0: %.1f, PM2.5: %.1f, PM4.0: %.1f, PM10: %.1f, Humidity: %.2f, Temp: %.2f, VOC: %.1f, NOx: %.1f\r\n",
                 pm1p0, pm2p5, pm4p0, pm10, humidity, temperature, voc_index, nox_index);
        SendResponse(response);

    } else if (strncmp(command, "SHT4X HIGH PRECISION", 20) == 0) {
        float temp, humidity;
        SHT4X_MeasureHighPrecision(&temp, &humidity);
        snprintf(response, sizeof(response), "SHT4X High Precision: Temp: %.2f C, Humidity: %.2f%%\r\n", temp, humidity);
        SendResponse(response);

    } else {
        snprintf(response, sizeof(response), "Unknown command: %s\r\n", command);
        SendResponse(response);
    }
}

