/*
 * LoRa_E5.h
 *
 *  Created on: Jan 18, 2025
 *      Author: erikl
 */

#ifndef INC_LORA_E5_H_
#define INC_LORA_E5_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>


// Konfigurationsstruktur
typedef struct {
    uint32_t frequency;      // Frequenz in Hz (z.B. 868000000)
    char spreadingFactor[5]; // Spreading Factor (z.B. "SF12")
    uint16_t bandwidth;      // Bandbreite in kHz (z.B. 125)
    uint8_t txPreamble;      // Präambellänge beim Senden
    uint8_t rxPreamble;      // Präambellänge beim Empfangen
    uint8_t power;           // Sendeleistung in dBm
    bool crc;                // CRC aktivieren
    bool iq;                 // IQ-Invertierung aktivieren
    bool network;            // Netzwerk-Check aktivieren
} SeeedE5Config;

// Funktionen
bool SeeedE5_Init(void);
bool SeeedE5_SetConfig(const SeeedE5Config* config);
bool SeeedE5_Send(const char* message);
bool SeeedE5_Receive(char* buffer, size_t bufferSize);

#endif /* INC_LORA_E5_H_ */
