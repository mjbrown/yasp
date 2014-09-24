/*  Code for processing command messages
*/
#include <stdint.h>
#include <stdio.h>

#define REG_LENGTH  32

static void(*commandCallbacks)(uint8_t *, uint16_t);
static uint8_t commands[REG_LENGTH];
static uint8_t registeredCommands = 0;

void registerYaspCommand(void(*commandCallback)(uint8_t *, uint16_t), uint8_t command) {
    commands[registeredCommands] = command;
    commandCallbacks = commandCallback;
    registeredCommands += 1;
}

#define SYNCH_BYTE  0xFF

/* Packet Structure */
#define SYNCH1_POS      0
#define SYNCH2_POS      1
#define START_CHECKSUM  2
#define LENGTH_POS      2  // Synch is not included in command length or checksum
#define COMMAND_POS     4

/* Return Codes */
#define RET_CMD_INCOMPLETE  -1
#define RET_CMD_NOSYNCH     -2
#define RET_CMD_CORRUPT     -3
#define RET_CMD_FAILED      -4

int16_t processCommand(uint8_t * buffer, uint16_t length)
{
    uint16_t cmd_len;
    uint16_t i;
    uint8_t checksum = 0;
    if (length < 6)
        return RET_CMD_INCOMPLETE;
    if ((buffer[0] & buffer[1] & SYNCH_BYTE) != SYNCH_BYTE)
        return RET_CMD_NOSYNCH;
    cmd_len = (((uint16_t) buffer[LENGTH_POS]) << 8) + ((uint16_t)buffer[LENGTH_POS+1]);
    if (cmd_len > (length - START_CHECKSUM)) // Synch not included in command length
    {
        return RET_CMD_INCOMPLETE;
    }
    for (i = START_CHECKSUM; i < cmd_len + START_CHECKSUM; i++)
    {
        checksum += buffer[i];
    }
    //printf("Actual: %02x, Checksum: %02x\n", buffer[cmd_len + START_CHECKSUM], checksum);
    if (checksum != buffer[cmd_len + START_CHECKSUM])
    {
        return RET_CMD_CORRUPT;
    }
    for (i = 0; i < registeredCommands; i++)
    {
        if (commands[i] == buffer[COMMAND_POS]) {
            commandCallbacks(buffer + COMMAND_POS + 1, cmd_len);
            break;
        }
    }
    if (i == registeredCommands) {
        return RET_CMD_FAILED;
    }
    return (START_CHECKSUM + cmd_len + 1);
}

uint8_t ack_buffer[16];
uint16_t ack_len;

void yaspRx(uint8_t * cmd_buffer, uint16_t buffer_len, uint16_t * handled)
{
    uint16_t i = 1;
    int16_t return_code = 0;
    return_code = processCommand(cmd_buffer, buffer_len);
    //sprintf(ack_buffer, "RX(%d,%d):%02x%02x%02x%02x%02x", return_code, buffer_len, cmd_buffer[3], cmd_buffer[4], cmd_buffer[5], cmd_buffer[6], cmd_buffer[7]);
    //serial_tx(ack_buffer, 30);
    if (return_code > 0)
    {
        sprintf(ack_buffer, "Success!");
        serial_tx(ack_buffer, 8);
        (*handled) = return_code;
    }
    else if (return_code == RET_CMD_NOSYNCH)
    {
        sprintf(ack_buffer, "Resynch!");
        serial_tx(ack_buffer, 8);
        while ((cmd_buffer[i++] != SYNCH_BYTE) && (i < buffer_len));
        (*handled) = i;
    }
    else if (return_code == RET_CMD_CORRUPT)
    {
        sprintf(ack_buffer, "Corrupt!");
        serial_tx(ack_buffer, 8);
        *handled = buffer_len;
    }
    else if (return_code == RET_CMD_FAILED)
    {
        sprintf(ack_buffer, "Failed!");
        serial_tx(ack_buffer, 8);
        *handled = buffer_len;
    }
}

void yasp_Init()
{
    setSerialRxHandler(yaspRx);
}
