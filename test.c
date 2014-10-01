#include "unity_fixture.h"
#include "yasp.h"
#include <string.h>
#include <stdio.h>
#define CMD_BUFF_LEN 8
#define CMD_TEST_CALLBACK   1
#define CMD_TEST_CALLBACK2  2

/*------------------------------ Test Helpers -------------------------------*/
static void(*rx_callback)(uint8_t *, uint16_t, uint16_t *) = 0;

void mock_callback(uint8_t * payload, uint16_t length)
{
}

void mock_callback2(uint8_t * payload, uint16_t length)
{
}

void serial_tx(uint8_t * buffer, uint16_t length) {
}

void serialService() {

}

void setSerialRxHandler(void(*callback)(uint8_t *, uint16_t, uint16_t *)) {
    rx_callback = callback;
}


// Define our main test group name
TEST_GROUP(YASP);

// This is run BEFORE each test
TEST_SETUP(YASP)
{
	yasp_Init();
}

// This is run AFTER each test
TEST_TEAR_DOWN(YASP)
{
}

// A stub test that always fails
TEST(YASP, MockCallbacks)
{
    uint16_t handled = 0xFFFF;
    uint8_t command_buffer[] = { 0xFF, 0xFF, 0x00, 0x04, CMD_TEST_CALLBACK, 0xAA, 0x04+ CMD_TEST_CALLBACK+0xAA};
    registerYaspCommand((void(*)(uint8_t *, uint16_t))mock_callback, CMD_TEST_CALLBACK);
    rx_callback(command_buffer, (uint32_t)(sizeof(command_buffer)), &handled);
    TEST_ASSERT_EQUAL_HEX16((sizeof(command_buffer)), handled);

    registerYaspCommand((void(*)(uint8_t *, uint16_t))mock_callback2, CMD_TEST_CALLBACK2);
    rx_callback(command_buffer, (uint32_t)(sizeof(command_buffer)), &handled);
    command_buffer[4] = CMD_TEST_CALLBACK2;
    command_buffer[6] = 0x04+ CMD_TEST_CALLBACK2+0xAA;
    rx_callback(command_buffer, (uint32_t)(sizeof(command_buffer)), &handled);
    TEST_ASSERT_EQUAL_HEX16((sizeof(command_buffer)), handled);
}

// Add all test cases to a test group
TEST_GROUP_RUNNER(YASP)
{
    RUN_TEST_CASE(YASP, MockCallbacks);
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
