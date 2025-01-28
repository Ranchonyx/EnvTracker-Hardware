/*
 * CommHandler.h
 *
 *  Created on: Jan 21, 2025
 *      Author: Erik Lauter
 */

#ifndef INC_COMMHANDLER_H_
#define INC_COMMHANDLER_H_

#ifndef COMM_HANDLER_H
#define COMM_HANDLER_H

#include <stdint.h>
void CommHandler_Init(void);
void CommHandler_LoRaProcess(uint8_t *data, uint16_t length);

#endif // COMM_HANDLER_H


#endif /* INC_COMMHANDLER_H_ */
