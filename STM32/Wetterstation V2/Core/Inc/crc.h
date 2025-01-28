/*
 * crc.h
 *
 *  Created on: May 27, 2024
 *      Author: erikl
 */
#ifndef CRC_H
#define CRC_H

#include "stm32l4xx_hal.h"

uint8_t CalcCrc(uint8_t data[2]);

#endif /* CRC_H */
