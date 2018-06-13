#ifndef YASP_PACKETIZE_H
#define YASP_PACKETIZE_H

#include "fifo.h"
#include <stdint.h>
#include <stdbool.h>

// TODO: Change based upon desired synchronization sequence
typedef uint16_t sync_t;
// TODO: Change based upon expected message size
typedef uint16_t data_length_t;
// TODO: Change based upon expected command count
typedef uint8_t command_t;
// TODO: Change based upon expected concurrent commands
typedef uint8_t handle_t;
// TODO: Change based upon desired checksum size
typedef uint16_t checksum_t;
// TODO: Change based upon desired CRC size
typedef uint32_t crc_t;


// TODO: Set these parameters based upon your protocol specification
#define SYNC_VALUE          0xAA55
#define MAX_DATA_LENGTH     1024


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

void packetize_data(command_t cmd, handle_t cmd_handle, uint8_t * data, data_length_t length, fifo_t * p_fifo);

bool unpacketize_data(fifo_t * p_fifo, uint8_t * data, uint16_t length);

void packetize_init(void);

#endif //YASP_PACKETIZE_H
