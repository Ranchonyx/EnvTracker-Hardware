/*
 * LoRa_E5.h
 *
 *  Created on: Jan 21, 2025
 *      Author: erikl
 */

#ifndef INC_LORA_E5_H_
#define INC_LORA_E5_H_

#include <stdint.h>

// LoRa E5 RX-Puffergröße
#define LORA_BUFFER_SIZE 512

// Externe Deklaration des Empfangspuffers
extern uint8_t lora_rx_buffer[LORA_BUFFER_SIZE];

// LoRa E5 API
void LoRa_E5_Init(void);
void LoRa_E5_SetModeTest(void);
void LoRa_E5_SetConfiguration(const char *config);
void LoRa_E5_SendMessage(const char *message);
void LoRa_E5_ReceiveMessage(void);


#endif /* INC_LORA_E5_H_ */
