#include <stdio.h>
#include <stdlib.h>
#include "packetize.h"
#include "util.h"
#include "error.h"

uint8_t tx_buffer[MAX_PROTOCOL_OVERHEAD + MAX_DATA_LENGTH + 1];
static fifo_t serial_tx = {0, 0, tx_buffer, sizeof(tx_buffer)};
uint8_t rx_buffer[MAX_PROTOCOL_OVERHEAD + MAX_DATA_LENGTH + 1];
static fifo_t serial_rx = {0, 0, rx_buffer, sizeof(rx_buffer)};

void test_loopback(int repetitions) {
    uint8_t fake_data[MAX_DATA_LENGTH];
    for (uint32_t i = 0; i < sizeof(fake_data); i++) {
        fake_data[i] = (uint8_t) i;
    }
    for (uint16_t i = 0; i < MAX_DATA_LENGTH; i++) {
        printf("Packet Length: %d\n", i);
        for (uint32_t j = 0; j < repetitions; j++) {
            loopback(fake_data, i, &serial_tx);
            if (!depacketize_data(&serial_tx, &serial_rx)) {
                printf("Error %d, %d!\n", i, j);
                exit(1);
            }
        }
    }
}

void test_extra_bytes(int repetitions) {
    uint8_t fake_data[4];
    for (uint32_t i = 0; i < repetitions; i++) {
        for (uint32_t j = 0; j < sizeof(fake_data); j++) {
            fake_data[j] = (uint8_t) (rand() & 0xFF);
        }
        for (uint32_t j = 0; j < 4; j++) {
            loopback(fake_data, sizeof(fake_data), &serial_tx);
            fifo_put(&serial_tx, fake_data, sizeof(fake_data));
        }
        while (depacketize_data(&serial_tx, &serial_rx));
        while (depacketize_data(&serial_rx, NULL));
    }
}

#define TEST_REPETITIONS    5

int main() {
    crc_init();
    register_cmd_handler(LOOPBACK_CMD, cmd_loopback_handler);
    register_cmd_handler(ERROR_CMD, cmd_error_handler);
    //test_loopback(TEST_REPETITIONS);
    test_extra_bytes(TEST_REPETITIONS);
    return 0;
}
