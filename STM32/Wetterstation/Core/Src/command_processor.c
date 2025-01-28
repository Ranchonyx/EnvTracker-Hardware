/*
 * SEN5x.c
 *
 *  Created on: May 27, 2024
 *      Author: erikl
 */
#include "command_processor.h"
#include "usart.h"
#include "SHT4x.h"
#include "HP20x.h"
#include "SEN5x.h"
#include <string.h>
#include <stdio.h>

// Externe Variablen
extern void Transmit_AllSensorData(void);
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern uint8_t interval_minutes;

// Helper-Funktion für UART-Übertragungen
static void SendUARTMessage(const char* message) {
    HAL_UART_Transmit(&huart1, (uint8_t*)message, strlen(message), HAL_MAX_DELAY);
    HAL_UART_Transmit(&huart2, (uint8_t*)message, strlen(message), HAL_MAX_DELAY);
}

// Command-Funktionen
static void Command_Help(const char* params) {
    (void)params;
    const char* help_message =
        "Available Commands:\r\n"
        "SET INTERVAL xx (in minutes)\r\n"
        "READ SENSORS\r\n"
        "HP20X TEMP\r\n"
        "HP20X PRESS\r\n"
        "HP20X ALT\r\n"
        "SEN5X START\r\n"
        "SEN5X STOP\r\n"
        "SHT4X HIGH PRECISION\r\n";
    SendUARTMessage(help_message);
}

static void Command_SetInterval(const char* params) {
    uint8_t interval = atoi(params);
    if (interval > 0 && interval <= 60) {
        interval_minutes = interval;
        char response[50];
        snprintf(response, sizeof(response), "Interval set to %d minutes.\r\n", interval);
        SendUARTMessage(response);
    } else {
        SendUARTMessage("Invalid interval. Use 1-60 minutes.\r\n");
    }
}

static void Command_ReadSensors(void) {
    SEN5X_StartMeasurement();
    HAL_Delay(60000); // Wartezeit für Messungen
    SendUARTMessage("Wait 60s\r\n");
    Transmit_AllSensorData();
}

static void Command_HP20xTemp(void) {
    float temp = HP20x_ReadTemperature();
    char response[50];
    snprintf(response, sizeof(response), "HP20X Temperature: %.2f C\r\n", temp);
    SendUARTMessage(response);
}

// Lookup-Tabelle für Befehle
typedef struct {
    const char* command;
    void (*handler)(const char* params);
} CommandEntry;

static const CommandEntry command_table[] = {
    {"HELP", Command_Help},
    {"SET INTERVAL", Command_SetInterval},
    {"READ SENSORS", Command_ReadSensors},
    {"HP20X TEMP", Command_HP20xTemp},
    // Weitere Befehle hier hinzufügen
    {NULL, NULL} // Ende der Tabelle
};

void ProcessCommand(char* command) {
    // Befehl und Parameter trennen
    char* params = strchr(command, ' ');
    if (params) {
        *params = '\0'; // Befehl terminieren
        params++;       // Zeiger auf Parameter setzen
    }

    // Lookup-Tabelle durchlaufen
    for (const CommandEntry* entry = command_table; entry->command != NULL; entry++) {
        if (strcmp(command, entry->command) == 0) {
            if (entry->handler) {
                entry->handler(params);
            }
            return;
        }
    }

    // Unbekannter Befehl
    SendUARTMessage("Unknown command.\r\n");
}

