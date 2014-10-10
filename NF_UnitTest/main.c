#include <stdint.h>
#include <stdio.h>
#include "../serial_HAL.h"
#include "../yasp.h"

static serial_rx_callback rx_callback = 0;

void setSerialRxHandler(serial_rx_callback callback) {
    rx_callback = callback;
}

// Transmit over serial, buffered
void serial_tx(uint8_t * buffer, uint16_t length) {
    //printf("%s", buffer);
}

#define CMD_TEST_CALLBACK   1
#define CMD_TEST_CALLBACK2  2

void mock_callback(uint8_t * payload, uint16_t length)
{
    printf("Got called!\n");
}

void mock_callback2(uint8_t * payload, uint16_t length)
{
    printf("Got called too!\n");
}

void test_mock_callback()
{
    uint16_t handled = 0xFFFF;
    uint8_t buffer[] = { 0xFF, 0xFF, 0x00, 0x04, CMD_TEST_CALLBACK, 0xAA, 0x04+ CMD_TEST_CALLBACK+0xAA};
    printf("Size: %08x\n", (uint32_t)(sizeof(buffer)));
    register_yasp_command((void(*)(uint8_t *, uint16_t))mock_callback, CMD_TEST_CALLBACK);
    handled = rx_callback(buffer, (uint32_t)(sizeof(buffer)));
    printf("Handled: %d\n", handled);

    register_yasp_command((void(*)(uint8_t *, uint16_t))mock_callback2, CMD_TEST_CALLBACK2);
    printf("Again!\n");
    handled = rx_callback(buffer, (uint32_t)(sizeof(buffer)));
    buffer[4] = CMD_TEST_CALLBACK2;
    buffer[6] = 0x04+ CMD_TEST_CALLBACK2+0xAA;
    handled = rx_callback(buffer, (uint32_t)(sizeof(buffer)));
    printf("Handled: %d\n", handled);
}


int main(int argc, char * argv[]) {
    yasp_init();
    test_mock_callback();
    return 0;
}