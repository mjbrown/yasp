#include <mem.h>
#include "error.h"
#include "util.h"

void error_msg(uint8_t * payload, data_length_t length, fifo_t * fifo) {
    packetize_data(ERROR_CMD, 0, payload, length, fifo);
}

RET_CODE_E cmd_error_handler(command_t cmd, handle_t handle, uint8_t * payload, data_length_t length) {
    fprintf(stderr, "Remote error: %s\n", payload);
    return RET_OK;
}

static uint8_t loop_buffer[MAX_DATA_LENGTH];

void loopback(uint8_t * payload, data_length_t length, fifo_t * fifo) {
    memcpy(payload, loop_buffer, length);
    packetize_data(LOOPBACK_CMD, 0, payload, length, fifo);
}

RET_CODE_E cmd_loopback_handler(command_t cmd, handle_t handle, uint8_t * payload, data_length_t length) {
    if (memcmp(payload, loop_buffer, length) != 0) {
        for (int i = 0; i < length; i++) {
            if (payload[i] != loop_buffer[i]) {
                fprintf(stderr, "Error in loopback data @ index %d: %d != %d\n", i, payload[i], loop_buffer[i]);
            }
        }
        return RET_CMD_FAILED;
    }
    return RET_OK;
}
