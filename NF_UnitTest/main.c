#include <stdint.h>
#include <stdio.h>
#include "../serial_HAL.h"
#include "../yasp.h"

static void(*rx_callback)(uint8_t *, uint16_t, uint16_t *) = 0;

void setSerialRxHandler(void(*callback)(uint8_t *, uint16_t, uint16_t *)) {
    rx_callback = callback;
}

// Transmit over serial, buffered
void serial_tx(uint8_t * buffer, uint16_t length) {
    printf("%s", buffer);
}

// Polls for serial data, transmits buffer
void serialService() {

}

#define CMD_TEST_CALLBACK   1

void mock_callback(uint8_t * payload, uint16_t length)
{
    printf("Got called!\n");
}

void test_mock_callback()
{
    uint16_t handled = 0xFFFF;
    uint8_t buffer[] = { 0xFF, 0xFF, 0x00, 0x04, CMD_TEST_CALLBACK, 0xAA, 0x04+ CMD_TEST_CALLBACK+0xAA};
    printf("Size: %08x\n", (uint32_t)(sizeof(buffer)));
    registerYaspCommand((void(*)(uint8_t *, uint16_t))mock_callback, CMD_TEST_CALLBACK);
    rx_callback(buffer, (uint32_t)(sizeof(buffer)), &handled);
    printf("Handled: %d\n", handled);
}


int main(int argc, char * argv[]) {
    yasp_Init();
    test_mock_callback();
    return 0;
}