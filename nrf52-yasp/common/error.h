#ifndef YASP_ERROR_H
#define YASP_ERROR_H

#include "packetize.h"

#define LOOPBACK_CMD    0
#define ERROR_CMD       1

void error_msg(uint8_t * payload, data_length_t length, fifo_t * fifo);
RET_CODE_E cmd_error_handler(command_t cmd, handle_t handle, uint8_t * payload, data_length_t length, fifo_t * resp_fifo);

void loopback(uint8_t * payload, data_length_t length, fifo_t * fifo);
void multi_section_loopback(uint16_t sections, uint8_t * payload, data_length_t length, fifo_t * fifo);
RET_CODE_E cmd_loopback_handler(command_t cmd, handle_t handle, uint8_t * payload, data_length_t length, fifo_t * resp_fifo);

#endif //YASP_ERROR_H
