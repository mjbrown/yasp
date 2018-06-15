#include <stdio.h>
#include <stdlib.h>
#include "packetize.h"
#include "util.h"

uint8_t tx_buffer[MAX_PROTOCOL_OVERHEAD + MAX_DATA_LENGTH + 1];
static fifo_t serial_tx = {0, 0, tx_buffer, sizeof(tx_buffer)};

#define LOOPBACK_CMD    1

RET_CODE_E cmd_loopback_handler(command_t cmd, handle_t handle, uint8_t * payload, data_length_t length) {
    //printf("Loopback (Command %d) (Handle %d) RX: ", cmd, handle);
    //for (int i = 0; i < length; i++) {
    //    printf("%02X", payload[i]);
    //}
    //printf("\n");
}

void loopback(uint8_t * payload, data_length_t length) {
    packetize_data(LOOPBACK_CMD, 0, payload, length, &serial_tx);
}

int main() {
    crc_init();
    uint8_t fake_data[MAX_DATA_LENGTH];
    for (uint32_t i = 0; i < sizeof(fake_data); i++) {
        fake_data[i] = (uint8_t) i;
    }
    register_cmd_handler(LOOPBACK_CMD, cmd_loopback_handler);
    for (uint16_t i = 0; i < MAX_DATA_LENGTH; i++) {
        printf("Packet Length: %d\n", i);
        for (uint32_t j = 0; j < 50000; j++) {
            loopback(fake_data, i);
            if (!depacketize_data(&serial_tx)) {
                printf("Error %d, %d!\n", i, j);
                exit(1);
            }
        }
    }
    return 0;
}
