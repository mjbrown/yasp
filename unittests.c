#include <string.h>
#include <stdlib.h>

#include "unity_fixture.h"
#include "yasp.h"
#include "serial_HAL.h"

#define CMD_TEST_CALLBACK   1
#define CMD_TEST_CALLBACK2  2
#define CMD_TEST_NOT_REGISTERED 3

#define SERIAL_BUFFER_SIZE  0xFFFF

/*------------------------------ Test Helpers -------------------------------*/
static serial_rx_callback rx_callback = 0;

static uint16_t number_of_mock_cb = 0;
static uint8_t last_mock_cb_payload[256];
static uint16_t last_mock_cb_payload_length;

static uint16_t number_of_mock_cb2 = 0;
static uint8_t last_mock_cb2_payload[256];
static uint16_t last_mock_cb2_payload_length;

static uint8_t last_serial_tx[256];
static uint16_t tx_buffer_len;

void mock_callback(uint8_t * payload, uint16_t length)
{
    number_of_mock_cb++;
    memcpy(last_mock_cb_payload, payload, length);
    last_mock_cb_payload_length += length;
}

void mock_callback2(uint8_t * payload, uint16_t length)
{
    number_of_mock_cb2++;
    memcpy(last_mock_cb2_payload, payload, length);
    last_mock_cb2_payload_length += length;
}

void serial_tx(uint8_t * buffer, uint16_t length) {
    memcpy(last_serial_tx+tx_buffer_len, buffer, length);
    tx_buffer_len += length;
}

void setSerialRxHandler(serial_rx_callback callback)
{
    rx_callback = callback;
}

uint16_t get_serial_buffer_size()
{
    return SERIAL_BUFFER_SIZE; // Max
}

// Define our main test group name
TEST_GROUP(YASP);

// This is run BEFORE each test
TEST_SETUP(YASP)
{
    yasp_init();
    number_of_mock_cb = 0;
    last_mock_cb_payload_length = 0;
    number_of_mock_cb2 = 0;
    last_mock_cb2_payload_length = 0;
    tx_buffer_len = 0;
}

// This is run AFTER each test
TEST_TEAR_DOWN(YASP)
{
}

TEST(YASP, TestOneCommandRegister)
{
    uint16_t handled = 0xFFFF;
    uint8_t command_buffer[] = { 0xFF, 0xFF, 0x00, 0x04, CMD_TEST_CALLBACK, 0xAA, 0x04+ CMD_TEST_CALLBACK+0xAA};
    register_yasp_command((void (*)(uint8_t *, uint16_t)) mock_callback, CMD_TEST_CALLBACK);
    handled = rx_callback(command_buffer, (uint16_t)(sizeof(command_buffer)));
    TEST_ASSERT_EQUAL_HEX16((sizeof(command_buffer)), handled);
    TEST_ASSERT_EQUAL_HEX16(1, number_of_mock_cb);
}

TEST(YASP, TestAnotherCommandRegister)
{
    uint16_t handled = 0xFFFF;
    uint8_t command_buffer[] = { 0xFF, 0xFF, 0x00, 0x04, CMD_TEST_CALLBACK, 0xAA, 0x04+ CMD_TEST_CALLBACK+0xAA};
    register_yasp_command((void (*)(uint8_t *, uint16_t)) mock_callback2, CMD_TEST_CALLBACK2);
    handled = rx_callback(command_buffer, (uint16_t)(sizeof(command_buffer)));
    TEST_ASSERT_EQUAL_HEX16(1, number_of_mock_cb);
    TEST_ASSERT_EQUAL_HEX16(sizeof(command_buffer), handled);
    command_buffer[4] = CMD_TEST_CALLBACK2;
    command_buffer[6] = 0x04+ CMD_TEST_CALLBACK2+0xAA;
    handled = rx_callback(command_buffer, (uint32_t)(sizeof(command_buffer)));
    TEST_ASSERT_EQUAL_HEX16((sizeof(command_buffer)), handled);
    TEST_ASSERT_EQUAL_HEX16(number_of_mock_cb2, 1);
}

TEST(YASP, TestNoSynch)
{
    uint16_t handled = 0xFFFF;
    uint8_t command_buffer[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09 };
    handled = rx_callback(command_buffer, (uint16_t)(sizeof(command_buffer)));
    TEST_ASSERT_EQUAL_HEX16(sizeof(command_buffer), handled);
    command_buffer[3] = 0xFF;
    command_buffer[4] = 0xFF;
    handled = rx_callback(command_buffer, (uint16_t)(sizeof(command_buffer)));
    TEST_ASSERT_EQUAL_HEX16(3, handled);
}

TEST(YASP, TestCorrupt)
{
    uint16_t handled = 0xFFFF;
    uint8_t command_buffer[] = { 0xFF, 0xFF, 0x00, 0x04, CMD_TEST_CALLBACK, 0xAA, 0x11};
    handled = rx_callback(command_buffer, (uint16_t)(sizeof(command_buffer)));
    TEST_ASSERT_EQUAL_HEX16(sizeof(command_buffer), handled);
    TEST_ASSERT_EQUAL_STRING("Corrupt!", last_serial_tx+5);
}

TEST(YASP, TestNoRegistered)
{
    uint16_t handled = 0xFFFF;
    uint8_t command_buffer[] = { 0xFF, 0xFF, 0x00, 0x04, CMD_TEST_NOT_REGISTERED, 0xAA, 0x04+ CMD_TEST_NOT_REGISTERED+0xAA};
    handled = rx_callback(command_buffer, (uint16_t)(sizeof(command_buffer)));
    TEST_ASSERT_EQUAL_HEX16(sizeof(command_buffer), handled);
    TEST_ASSERT_EQUAL_STRING("NotRegistered!", last_serial_tx+5);
}

TEST(YASP, TestDoubleCommand)
{
    uint16_t handled = 0xFFFF;
    uint8_t command_buffer[] = { 0xFF, 0xFF, 0x00, 0x04, CMD_TEST_CALLBACK, 0x33, 0x04+ CMD_TEST_CALLBACK+0x33, \
                                 0xFF, 0xFF, 0x00, 0x04, CMD_TEST_CALLBACK2, 0x44, 0x04+ CMD_TEST_CALLBACK2+0x44};
    handled = rx_callback(command_buffer, (uint16_t)(sizeof(command_buffer)));
    TEST_ASSERT_EQUAL_HEX16(1, number_of_mock_cb);
    TEST_ASSERT_EQUAL(0x33, last_mock_cb_payload[0]);
    TEST_ASSERT_EQUAL_HEX16(1, last_mock_cb_payload_length);
    TEST_ASSERT_EQUAL_HEX16(1, number_of_mock_cb2);
    TEST_ASSERT_EQUAL_HEX16(handled, sizeof(command_buffer));
}

TEST(YASP, TestInfoCommand)
{
    uint16_t handled = 0xFFFF;
    uint8_t command_buffer[] = { 0xFF, 0xFF, 0x00, 0x03, CMD_YASP_INFO, 0x03+ CMD_YASP_INFO };
    handled = rx_callback(command_buffer, (uint16_t)(sizeof(command_buffer)));
    TEST_ASSERT_EQUAL(REGISTRY_LENGTH, last_serial_tx[5]);
    TEST_ASSERT_EQUAL(SERIAL_BUFFER_SIZE>>8, last_serial_tx[6]);
    TEST_ASSERT_EQUAL(SERIAL_BUFFER_SIZE&0xFF, last_serial_tx[7]);
}

// Add all test cases to a test group
TEST_GROUP_RUNNER(YASP)
{
    RUN_TEST_CASE(YASP, TestOneCommandRegister);
    RUN_TEST_CASE(YASP, TestAnotherCommandRegister);
    RUN_TEST_CASE(YASP, TestNoSynch);
    RUN_TEST_CASE(YASP, TestCorrupt);
    RUN_TEST_CASE(YASP, TestNoRegistered);
    RUN_TEST_CASE(YASP, TestDoubleCommand);
    RUN_TEST_CASE(YASP, TestInfoCommand);
}

// Run all test groups (only 1 for now)
static void
run_all_tests(void)
{
    RUN_TEST_GROUP(YASP);
}

// Main program passes execution to Unity
int main(int argc, char *argv[])
{
    return UnityMain(argc, argv, run_all_tests);
}
