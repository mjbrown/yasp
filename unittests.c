#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "unity_fixture.h"
#include "yasp.h"
#include "serial_HAL.h"
#include "crc32/crc32.h"

#define CMD_TEST_CALLBACK       1
#define CMD_TEST_CALLBACK2      2
#define CMD_TEST_NOT_REGISTERED 3
#define COMMAND_POS             4

#define SERIAL_BUFFER_SIZE      0xFFFF
#define CRC32_SIZE              4
#define MSG_WITH_1_BYTE_PAYLOAD_SIZE 10

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

TEST(YASP, TestCommandLength)
{
    uint8_t payload_buffer[] = { 0xAA, 0xBB };
    uint8_t command = CMD_TEST_CALLBACK;
    uint16_t command_size_calc = 0;
    uint16_t command_size_buff = 0;

    send_yasp_command(command, payload_buffer, sizeof(payload_buffer), false);
    command_size_calc = sizeof(command) + sizeof(command_size_calc) + sizeof(payload_buffer);
    command_size_buff = (last_serial_tx[2] << 8) | last_serial_tx[3];

    TEST_ASSERT_EQUAL_HEX16(command_size_calc, command_size_buff);
}

TEST(YASP, TestOneCommandRegister)
{
    uint16_t handled = 0xFFFF;
    uint32_t crc_32 = 0;
    uint8_t payload_buffer[] = { 0xAA, 0xBB };
    uint16_t bytes_sent;

    register_yasp_command((void (*)(uint8_t *, uint16_t)) mock_callback, CMD_TEST_CALLBACK);
    send_yasp_command(CMD_TEST_CALLBACK, payload_buffer, sizeof(payload_buffer), false);
    bytes_sent = tx_buffer_len;
    handled = rx_callback(last_serial_tx, tx_buffer_len);
    
    TEST_ASSERT_EQUAL_HEX16(bytes_sent, handled);
    TEST_ASSERT_EQUAL_HEX16(1, number_of_mock_cb);
}

TEST(YASP, TestAnotherCommandRegister)
{
    uint16_t handled = 0xFFFF;
    uint8_t payload_buffer[] = { 0xAA, 0xBB, 0xCC };
    uint8_t payload_buffer2[] = { 0xDD, 0xEE, 0xFF };
    
    register_yasp_command((void (*)(uint8_t *, uint16_t)) mock_callback, CMD_TEST_CALLBACK);
    send_yasp_command(CMD_TEST_CALLBACK, payload_buffer, sizeof(payload_buffer), false);
    handled = rx_callback(last_serial_tx, tx_buffer_len);
    TEST_ASSERT_EQUAL_HEX16(1, number_of_mock_cb);
    TEST_ASSERT_EQUAL_HEX16(tx_buffer_len, handled);
    
    register_yasp_command((void (*)(uint8_t *, uint16_t)) mock_callback2, CMD_TEST_CALLBACK2);
    send_yasp_command(CMD_TEST_CALLBACK2, payload_buffer2, sizeof(payload_buffer2), false);
    handled = rx_callback(last_serial_tx, tx_buffer_len);
    TEST_ASSERT_EQUAL_HEX16(tx_buffer_len, handled);
    TEST_ASSERT_EQUAL_HEX16(number_of_mock_cb2, 1);
}

TEST(YASP, TestNoSynch)
{
    uint16_t handled = 0xFFFF;
    uint8_t command_buffer[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09 };
    register_yasp_command((void (*)(uint8_t *, uint16_t)) mock_callback2, CMD_TEST_CALLBACK2);
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
    uint8_t payload_buffer[] = { 0xAA };
    uint8_t error_string[] = "Corrupt!";
    uint16_t error_string_idx = 0;
    uint16_t command_size = 0;
    
    register_yasp_command((void (*)(uint8_t *, uint16_t)) mock_callback, CMD_TEST_CALLBACK);
    send_yasp_command(CMD_TEST_CALLBACK, payload_buffer, sizeof(payload_buffer), false);
    handled = rx_callback(last_serial_tx, tx_buffer_len);
    
    send_yasp_command(CMD_TEST_CALLBACK, payload_buffer, sizeof(payload_buffer), false);
    // Corrupt last byte of crc
    last_serial_tx[tx_buffer_len - 1] ^= 0xFF;
    
    command_size = tx_buffer_len;
    handled = rx_callback(last_serial_tx, tx_buffer_len);
    
    TEST_ASSERT_EQUAL_HEX16(command_size, handled);
    /* "Corrupt!" payload */
    error_string_idx = tx_buffer_len - sizeof(error_string) - sizeof(uint32_t);
    TEST_ASSERT_EQUAL_STRING(error_string, &last_serial_tx[error_string_idx]);
}

TEST(YASP, TestNoRegistered)
{
    uint16_t handled = 0xFFFF;
    uint8_t payload_buffer[] = { 0xAA };
    uint8_t error_string[] = "NotRegistered!";
    uint16_t error_string_idx = 0;
    uint16_t command_size = 0;
    
    send_yasp_command(CMD_TEST_NOT_REGISTERED, payload_buffer, tx_buffer_len, false);
    command_size = tx_buffer_len;
    handled = rx_callback(last_serial_tx, tx_buffer_len);
    TEST_ASSERT_EQUAL_HEX16(command_size, handled);
    error_string_idx = tx_buffer_len - sizeof(error_string) - sizeof(uint32_t);
    TEST_ASSERT_EQUAL_STRING("NotRegistered!", &last_serial_tx[error_string_idx]);
}

TEST(YASP, TestDoubleCommand)
{
    uint16_t i=0;
    uint16_t handled = 0xFFFF;
    uint8_t payload_buffer_1[] = { 0x33  };
    uint8_t payload_buffer_2[] = { 0x44 };
    uint16_t command_size;

    register_yasp_command((void (*)(uint8_t *, uint16_t)) mock_callback, CMD_TEST_CALLBACK);
    register_yasp_command((void (*)(uint8_t *, uint16_t)) mock_callback2, CMD_TEST_CALLBACK2);
    send_yasp_command(CMD_TEST_CALLBACK, payload_buffer_1, sizeof(payload_buffer_1), false);
    send_yasp_command(CMD_TEST_CALLBACK2, payload_buffer_2, sizeof(payload_buffer_2), false);

    command_size = tx_buffer_len;
    handled = rx_callback(last_serial_tx, command_size);
    TEST_ASSERT_EQUAL_HEX16(1, number_of_mock_cb);
    TEST_ASSERT_EQUAL(0x33, last_mock_cb_payload[0]);
    TEST_ASSERT_EQUAL_HEX16(1, last_mock_cb_payload_length);
    TEST_ASSERT_EQUAL_HEX16(1, number_of_mock_cb2);
    TEST_ASSERT_EQUAL_HEX16(handled, command_size);
}

TEST(YASP, TestInfoCommand)
{
    uint16_t handled = 0xFFFF;
    uint8_t registry_length_idx;
    send_yasp_command(CMD_YASP_INFO, 0, 0, false);
    handled = rx_callback(last_serial_tx, tx_buffer_len);
    registry_length_idx = tx_buffer_len - sizeof(uint32_t) - 3;
    TEST_ASSERT_EQUAL(REGISTRY_LENGTH, last_serial_tx[registry_length_idx]);
    TEST_ASSERT_EQUAL(SERIAL_BUFFER_SIZE>>8, last_serial_tx[registry_length_idx + 1]);
    TEST_ASSERT_EQUAL(SERIAL_BUFFER_SIZE&0xFF, last_serial_tx[registry_length_idx + 2]);
}


// Add all test cases to a test group
TEST_GROUP_RUNNER(YASP)
{
    RUN_TEST_CASE(YASP, TestCommandLength);
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
