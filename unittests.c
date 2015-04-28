#include <string.h>
#include <stdlib.h>

#include "unity_fixture.h"
#include "yasp.h"
#include "serial_HAL.h"
#include "crc32/crc32.h"

#define CMD_TEST_CALLBACK       1
#define CMD_TEST_CALLBACK2      2
#define CMD_TEST_NOT_REGISTERED 3

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

void add_and_cat_crc(uint8_t * command, uint8_t * message, uint16_t command_size)
{
    int i=0;
    uint32_t crc_32 = 0;
    
    memcpy(message, command, command_size);
    crc_32 = crc32(crc_32, command+2, command_size-2);
    crc32_serialize(crc_32, message+command_size);
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
    uint32_t crc_32 = 0;
    uint8_t command_buffer[] = { 0xFF, 0xFF, 0x00, 0x04, CMD_TEST_CALLBACK, 0xAA};
    uint8_t message_buffer[MSG_WITH_1_BYTE_PAYLOAD_SIZE];

    add_and_cat_crc(command_buffer, message_buffer, 6);
    register_yasp_command((void (*)(uint8_t *, uint16_t)) mock_callback, CMD_TEST_CALLBACK);
    handled = rx_callback(message_buffer, (uint16_t)(sizeof(message_buffer)));
    
    TEST_ASSERT_EQUAL_HEX16((sizeof(message_buffer)), handled);
    TEST_ASSERT_EQUAL_HEX16(1, number_of_mock_cb);
}

TEST(YASP, TestAnotherCommandRegister)
{
    uint16_t handled = 0xFFFF;
    uint8_t command_buffer[] = { 0xFF, 0xFF, 0x00, 0x04, CMD_TEST_CALLBACK, 0xAA };
    uint8_t message_buffer[MSG_WITH_1_BYTE_PAYLOAD_SIZE];
    
    add_and_cat_crc(command_buffer, message_buffer, 6);
    register_yasp_command((void (*)(uint8_t *, uint16_t)) mock_callback2, CMD_TEST_CALLBACK2);
    handled = rx_callback(message_buffer, (uint16_t)(sizeof(message_buffer)));
    TEST_ASSERT_EQUAL_HEX16(1, number_of_mock_cb);
    TEST_ASSERT_EQUAL_HEX16(sizeof(message_buffer), handled);
    
    command_buffer[4] = CMD_TEST_CALLBACK2;
    add_and_cat_crc(command_buffer, message_buffer, 6);
    handled = rx_callback(message_buffer, (uint32_t)(sizeof(message_buffer)));
    TEST_ASSERT_EQUAL_HEX16((sizeof(message_buffer)), handled);
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
    uint8_t command_buffer[] = { 0xFF, 0xFF, 0x00, 0x04, CMD_TEST_CALLBACK, 0xAA };
    uint8_t message_buffer[MSG_WITH_1_BYTE_PAYLOAD_SIZE];
    uint8_t * corrupt_byte_pt;
    add_and_cat_crc(command_buffer, message_buffer, 6);
    // Corrupt last byte of crc
    corrupt_byte_pt = &message_buffer[sizeof(message_buffer) - 1];
    *corrupt_byte_pt ^= 0xFF;
    
    register_yasp_command((void (*)(uint8_t *, uint16_t)) mock_callback, CMD_TEST_CALLBACK);
    handled = rx_callback(message_buffer, (uint16_t)(sizeof(message_buffer)));
    TEST_ASSERT_EQUAL_HEX16((sizeof(message_buffer)), handled);
    TEST_ASSERT_EQUAL_HEX16(0, number_of_mock_cb);
    TEST_ASSERT_EQUAL_STRING("Corrupt!", last_serial_tx+5);
}

TEST(YASP, TestNoRegistered)
{
    uint16_t handled = 0xFFFF;
    uint8_t command_buffer[] = { 0xFF, 0xFF, 0x00, 0x04, CMD_TEST_NOT_REGISTERED, 0xAA };
    uint8_t message_buffer[MSG_WITH_1_BYTE_PAYLOAD_SIZE];
    add_and_cat_crc(command_buffer, message_buffer, 6);
    
    handled = rx_callback(message_buffer, (uint16_t)(sizeof(message_buffer)));
    TEST_ASSERT_EQUAL_HEX16(sizeof(message_buffer), handled);
    TEST_ASSERT_EQUAL_STRING("NotRegistered!", last_serial_tx+5);
}

TEST(YASP, TestDoubleCommand)
{
    uint16_t i=0;
    uint16_t handled = 0xFFFF;
    uint8_t command_buffer_1[] = { 0xFF, 0xFF, 0x00, 0x04, CMD_TEST_CALLBACK, 0x33  };
    uint8_t command_buffer_2[] = { 0xFF, 0xFF, 0x00, 0x04, CMD_TEST_CALLBACK2, 0x44 };
                                
    uint8_t message_buffer_1[MSG_WITH_1_BYTE_PAYLOAD_SIZE];
    uint8_t message_buffer_2[MSG_WITH_1_BYTE_PAYLOAD_SIZE];
    uint8_t complete_message[(MSG_WITH_1_BYTE_PAYLOAD_SIZE)*2];
    add_and_cat_crc(command_buffer_1, message_buffer_1, 6);
    add_and_cat_crc(command_buffer_2, message_buffer_2, 6);
    memcpy(complete_message, message_buffer_1, sizeof(message_buffer_1));
    printf("\nmessage1: \t\t");
    for(i= 0; i < sizeof(message_buffer_1); i++)
    {
        printf("%02x ", message_buffer_1[i]);
    }
    printf("\r\n");
    printf("\nmessage2: \t\t");
    for(i= 0; i < sizeof(message_buffer_2); i++)
    {
        printf("%02x ", message_buffer_2[i]);
    }
    printf("\r\n");

    memcpy(&complete_message[sizeof(message_buffer_1)], message_buffer_2, sizeof(message_buffer_2));
    
    handled = rx_callback(complete_message, (uint16_t)(sizeof(complete_message)));
    TEST_ASSERT_EQUAL_HEX16(1, number_of_mock_cb);
    TEST_ASSERT_EQUAL(0x33, last_mock_cb_payload[0]);
    TEST_ASSERT_EQUAL_HEX16(1, last_mock_cb_payload_length);
    TEST_ASSERT_EQUAL_HEX16(1, number_of_mock_cb2);
    TEST_ASSERT_EQUAL_HEX16(handled, sizeof(complete_message));
}

TEST(YASP, TestInfoCommand)
{
    uint16_t handled = 0xFFFF;
    uint8_t command_buffer[] = { 0xFF, 0xFF, 0x00, 0x03, CMD_YASP_INFO };
    uint8_t message_buffer[5 + CRC32_SIZE];
    add_and_cat_crc(command_buffer, message_buffer, 5);
    handled = rx_callback(message_buffer, (uint16_t)(sizeof(message_buffer)));
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
