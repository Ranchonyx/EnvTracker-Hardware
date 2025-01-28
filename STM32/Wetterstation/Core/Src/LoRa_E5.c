/*
 * LoRa_E5.c
 *
 *  Created on: Jan 18, 2025
 *      Author: erikl
 */
// LoRa_E5.c

#include "LoRa_E5.h"
#include <string.h>
#include <stdio.h>
#include "stm32l4xx_hal.h"
#include "LoRa_E5.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define Serial_Print printf


// Serielle Schnittstelle simulieren (ersetzten Sie dies durch Ihre spezifische Implementierung)
extern char uart1_rx_buffer[];
extern volatile uint8_t uart1_command_ready;
extern volatile uint16_t uart1_rx_index;
extern UART_HandleTypeDef huart1;   // UART1-Handle
extern uint8_t rx_buff1[1];        // Empfangspuffer


// Serial_Read implementieren
int Serial_Read(char* buffer, size_t length) {
    if (uart1_command_ready) { // Prüfen, ob eine Nachricht bereitsteht
        strncpy(buffer, uart1_rx_buffer, length); // Nachricht in den Puffer kopieren
        buffer[length - 1] = '\0'; // Sicherstellen, dass die Nachricht nullterminiert ist
        uart1_command_ready = 0; // Flag zurücksetzen
        return strlen(buffer); // Rückgabe der Nachrichtengröße
    }
    return 0; // Keine neuen Daten verfügbar
}
// Timeout in Millisekunden
#define TIMEOUT_MS 2000

// Empfangspuffer
#define BUFFER_SIZE 512
static char recvBuffer[BUFFER_SIZE];


// Warte auf spezifische Antwort
static bool WaitForResponse(const char* expectedResponse, uint32_t timeoutMs) {
    uint32_t startTime = HAL_GetTick(); // Startzeit in Millisekunden
    memset(recvBuffer, 0, sizeof(recvBuffer));

    while ((HAL_GetTick() - startTime) < timeoutMs) {
        if (Serial_Read(recvBuffer, sizeof(recvBuffer)) > 0) {
            if (strstr(recvBuffer, expectedResponse) != NULL) {
                return true; // Erwartete Antwort gefunden
            }
        }
    }

    return false; // Timeout
}

// Modul initialisieren
bool SeeedE5_Init(void) {
    HAL_UART_Receive_IT(&huart1, rx_buff1, 1); // UART Empfang starten
    printf("AT\r\n");
    return WaitForResponse("+AT: OK", TIMEOUT_MS);
}


// Konfiguration setzen
bool SeeedE5_SetConfig(const SeeedE5Config* config) {
    char cmd[128];
    snprintf(cmd, sizeof(cmd), "AT+MODE=TEST\r\n");
    if (!WaitForResponse("+MODE: TEST", TIMEOUT_MS)) {
        return false;
    }

    snprintf(cmd, sizeof(cmd), "AT+TEST=RFCFG,%lu,%s,%d,%d,%d,%d,%s,%s,%s\r\n",
             config->frequency, config->spreadingFactor, config->bandwidth, config->txPreamble,
             config->rxPreamble, config->power, config->crc ? "ON" : "OFF",
             config->iq ? "ON" : "OFF", config->network ? "ON" : "OFF");


    Serial_Print(cmd);
    return WaitForResponse("+TEST: RFCFG", TIMEOUT_MS);
}

// Nachricht senden
bool SeeedE5_Send(const char* message) {
    char cmd[128];
    snprintf(cmd, sizeof(cmd), "AT+TEST=TXLRPKT,\"%s\"\r\n", message);
    Serial_Print(cmd);
    return WaitForResponse("+TEST: TX DONE", TIMEOUT_MS);
}

// Nachricht empfangen
bool SeeedE5_Receive(char* buffer, size_t bufferSize) {
    Serial_Print("AT+TEST=RXLRPKT\r\n");
    if (WaitForResponse("+TEST: RXLRPKT", TIMEOUT_MS)) {
        strncpy(buffer, recvBuffer, bufferSize);
        return true;
    }
    return false;
}
