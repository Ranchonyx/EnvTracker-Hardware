/*
 * CommHandler.c
 *
 *  Created on: Jan 21, 2025
 *      Author: Erik Lauter
 */

#include "CommHandler.h"
#include "LoRa_E5.h"
#include <stdio.h>

void CommHandler_Init(void) {
    // Initialisiere das LoRa-Modul
    LoRa_E5_Init();
}

// Verarbeitung von Nachrichten des LoRa-Moduls
void CommHandler_LoRaProcess(uint8_t *data, uint16_t length) {
    printf("LoRa Handler: Processing message: %.*s\n", length, data);

    // Beispiel: Eingehende Nachricht auswerten
    if (strncmp((char *)data, "+TEST: TX DONE", length) == 0) {
        printf("LoRa Handler: Message successfully sent.\n");
    } else if (strncmp((char *)data, "+TEST: RXLRPKT", length) == 0) {
        printf("LoRa Handler: New message received.\n");
        // Verarbeite weitere Details (z. B. RSSI, SNR)
    } else {
        printf("LoRa Handler: Unrecognized message: %.*s\n", length, data);
    }
}

