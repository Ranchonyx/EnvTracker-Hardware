/*
 * command_processor.h
 *
 *  Created on: May 27, 2024
 *      Author: erikl
 */

#ifndef COMMAND_PROCESSOR_H
#define COMMAND_PROCESSOR_H

#include "stm32l4xx_hal.h"

/**
 * @brief Processes the received command string and executes the corresponding action.
 *
 * @param command Null-terminated string containing the received command.
 */
void ProcessCommand(char* command);

#endif /* COMMAND_PROCESSOR_H */
