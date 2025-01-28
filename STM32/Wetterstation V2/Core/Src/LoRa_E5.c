/*
 * LoRa_E5.c
 *
 *  Created on: Jan 21, 2025
 *      Author: erikl
 */
/*
 * LoRa_E5.c
 *
 *  Created on: Jan 21, 2025
 *      Author: erikl
 */
/*
 * LoRa_E5.c
 *
 *  Created on: Jan 21, 2025
 *      Author: erikl
 */

#include "LoRa_E5.h"
#include <stdio.h>
#include <string.h>
#include "usart.h"

// Empfangspuffer für LoRa
#define LORA_BUFFER_SIZE 512
uint8_t lora_rx_buffer[LORA_BUFFER_SIZE];

// UART-Handle für das LoRa E5
extern UART_HandleTypeDef huart1; // LoRa-Modul ist an UART1 angeschlossen
extern UART_HandleTypeDef huart2; // UART2 für Debugging

// Funktion zur Umwandlung von Text in Hexadezimal
void ConvertTextToHex(const char *text, char *hex_buffer, size_t buffer_size) {
    size_t length = strlen(text);
    for (size_t i = 0; i < length && (i * 2) < buffer_size - 1; i++) {
        snprintf(&hex_buffer[i * 2], 3, "%02X", (unsigned char)text[i]);
    }
}

// Funktion zum Senden eines Befehls und Lesen der Antwort
static void LoRa_E5_SendCommand(const char *command) {
    // Empfangspuffer vor der Verwendung leeren
    memset(lora_rx_buffer, 0, LORA_BUFFER_SIZE);

    // Befehl senden
    HAL_UART_Transmit(&huart1, (uint8_t *)command, strlen(command), HAL_MAX_DELAY);
    printf("LoRa E5: Sent command: %s\n", command);

    // Antwort per DMA empfangen
    HAL_UART_Receive_DMA(&huart1, lora_rx_buffer, LORA_BUFFER_SIZE);

    // Warten, um sicherzustellen, dass die Antwort vollständig empfangen wurde
    HAL_Delay(500);

    // Empfangene Daten anzeigen
    if (strlen((char *)lora_rx_buffer) > 0) {
        printf("LoRa E5: Received: %s\n", lora_rx_buffer);

        // Antwort über UART2 (Debugging-Port) ausgeben
        HAL_UART_Transmit(&huart2, lora_rx_buffer, strlen((char *)lora_rx_buffer), HAL_MAX_DELAY);

        // Zusätzlicher Zeilenumbruch für bessere Lesbarkeit
        uint8_t newline = '\n';
        HAL_UART_Transmit(&huart2, &newline, 1, HAL_MAX_DELAY);
    } else {
        printf("LoRa E5: No response received.\n");
    }
}

// Initialisierung des LoRa E5 Moduls
void LoRa_E5_Init(void) {
    printf("LoRa E5: Initializing...\n");
    LoRa_E5_SetModeTest();
}

// Testmodus aktivieren
void LoRa_E5_SetModeTest(void) {
    LoRa_E5_SendCommand("AT+MODE=TEST\r\n");
}

// Konfiguration setzen
void LoRa_E5_SetConfiguration(const char *config) {
    char cmd[128];
    snprintf(cmd, sizeof(cmd), "AT+TEST=RFCFG,%s\r\n", config);
    LoRa_E5_SendCommand(cmd);
}

// Nachricht im Testmodus senden
void LoRa_E5_SendMessage(const char *message) {
    char hex_buffer[2 * LORA_BUFFER_SIZE] = {0}; // Puffer für Hexadezimalnachricht
    char at_command[2 * LORA_BUFFER_SIZE + 50] = {0}; // Puffer für das AT-Kommando

    // Text in Hexadezimal umwandeln
    ConvertTextToHex(message, hex_buffer, sizeof(hex_buffer));

    // AT-Befehl formatieren
    snprintf(at_command, sizeof(at_command), "AT+TEST=TXLRPKT,%s\r\n", hex_buffer);

    // Befehl senden
    LoRa_E5_SendCommand(at_command);

    printf("LoRa E5: Sent Hex Message: %s\n", hex_buffer);
}

// Nachricht empfangen
void LoRa_E5_ReceiveMessage(void) {
    // Befehl zum Empfangen senden
    LoRa_E5_SendCommand("AT+TEST=RXLRPKT\r\n");

    // Prüfen, ob Daten empfangen wurden
    if (strlen((char *)lora_rx_buffer) > 0) {
        printf("LoRa E5: Received: %s\n", lora_rx_buffer);

        // Daten im Format +TEST: RX "HEXDATA" verarbeiten
        char *start = strstr((char *)lora_rx_buffer, "+TEST: RX \"");
        if (start) {
            start += 10; // Nach +TEST: RX " springen
            char *end = strchr(start, '"'); // Ende des HEXDATA-Abschnitts finden
            if (end) {
                *end = '\0'; // String hier beenden, um nur HEXDATA zu behalten

                // HEX-Daten in Text umwandeln
                size_t hex_length = strlen(start);
                char text_buffer[LORA_BUFFER_SIZE] = {0};
                for (size_t i = 0; i < hex_length; i += 2) {
                    char hex_byte[3] = {start[i], start[i + 1], '\0'};
                    text_buffer[i / 2] = (char)strtol(hex_byte, NULL, 16);
                }

                // Umgewandelten Text an den Command-Processor weiterleiten
                printf("LoRa E5: Decoded Text: %s\n", text_buffer);
                ProcessCommand(text_buffer);
            } else {
                printf("LoRa E5: Invalid data format (missing closing quote).\n");
            }
        } else {
            printf("LoRa E5: Invalid data format (missing +TEST: RX).\n");
        }
    } else {
        printf("LoRa E5: No response received.\n");
    }
}

// Empfangene Antwort verarbeiten
void LoRa_E5_ProcessResponse(void) {
    // Empfangene Daten ausgeben
    if (strlen((char *)lora_rx_buffer) > 0) {
        printf("LoRa E5: Response: %s\n", lora_rx_buffer);
    } else {
        printf("LoRa E5: No response received.\n");
    }
}
