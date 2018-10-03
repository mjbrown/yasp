#ifndef YASP_PACKETIZE_H
#define YASP_PACKETIZE_H

#include "fifo.h"
#include <stdint.h>
#include <stdbool.h>

// TODO: Change based upon expected sync size
typedef uint16_t sync_t;
// TODO: Change based upon expected message size
typedef uint16_t data_length_t;
// TODO: Change based upon expected command count
typedef uint8_t command_t;
// TODO: Change based upon expected concurrent commands
typedef uint8_t handle_t;
// TODO: Change based upon desired checksum size
typedef uint16_t checksum_t;


// TODO: Set these parameters based upon your protocol specification
#define SYNC_VALUE          0xAA55
#define MAX_DATA_LENGTH     1024
#define MAX_NUMBER_OF_CMDS  100

#define MAX_PROTOCOL_OVERHEAD     (sizeof(sync_t) + sizeof(data_length_t) + sizeof(command_t) + sizeof(handle_t) + sizeof(checksum_t) + sizeof(crc_t))

typedef enum
{
    RET_OK=0x00,
    RET_NOT_SUPPORTED=0x01,
    RET_INVALID_PARAM=0x02,
    RET_INVALID_STATE=0x03,
    RET_BUFFER_FULL=0x04,
    RET_NOT_CONNECTED=0x05,
    RET_CMD_FAILED=0x06,
    RET_AUTO_ACK=0x07
} RET_CODE_E;

typedef struct
{
    uint8_t * data;
    data_length_t length;
} payload_section_t;

typedef RET_CODE_E (* cmd_handler_t) (command_t cmd, handle_t handle, uint8_t * payload, data_length_t length, fifo_t * resp_fifo);

void packetize_data(command_t cmd, handle_t cmd_handle, payload_section_t * payloads, uint16_t num_payloads, fifo_t * p_fifo);

bool depacketize_data(fifo_t * rx_fifo, fifo_t * err_fifo);

void register_cmd_handler(command_t cmd, cmd_handler_t cmd_handler);

#endif //YASP_PACKETIZE_H
