#include <mem.h>
#include "error.h"
#include "util.h"
#include "packetize.h"

void error_msg(uint8_t * payload, data_length_t length, fifo_t * fifo) {
    payload_section_t payloads = {payload, length};
    packetize_data(ERROR_CMD, 0, &payloads, 1, fifo);
}

RET_CODE_E cmd_error_handler(command_t cmd, handle_t handle, uint8_t * payload, data_length_t length, fifo_t * resp_fifo) {
    //fprintf(stderr, "Remote error: %s\n", payload);
    return RET_OK;
}

//static uint8_t loop_buffer[MAX_DATA_LENGTH];

void loopback(uint8_t * payload, data_length_t length, fifo_t * fifo) {
//    memcpy(payload, loop_buffer, length);
    payload_section_t payloads = {payload, length};
    packetize_data(LOOPBACK_CMD, 0, &payloads, 1, fifo);
}

void multi_section_loopback(uint16_t sections, uint8_t * payload, data_length_t length, fifo_t * fifo) {
//    memcpy(payload, loop_buffer, length);
    payload_section_t payloads[16];
    uint16_t section_length = length / sections;
    uint16_t remainder = length % sections;
    uint16_t full_sections = (remainder > 0) ? (sections - 1) : sections;
    for (uint16_t i = 0; i < full_sections; i++) {
        payloads[i].length = section_length;
        payloads[i].data = payload + (i * section_length);
    }
    if (sections != full_sections) {
        payloads[sections - 1].length = remainder;
        payloads[sections - 1].data = payload + (full_sections * section_length);
    }
    packetize_data(LOOPBACK_CMD, 0, &payloads[0], sections, fifo);
}

RET_CODE_E cmd_loopback_handler(command_t cmd, handle_t handle, uint8_t * payload, data_length_t length, fifo_t * resp_fifo) {
    loopback(payload, length, resp_fifo);
    return RET_OK;
}
