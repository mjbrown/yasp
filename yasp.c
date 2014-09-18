/*  Code for processing command messages
*/
#include <stdint.h>
#include <stdio.h>

#include <usb/usb.h>
#include <usb/usb_device_cdc.h>
#include <i2c.h>

#define SYNCH_BYTE  0xFF

uint16_t BigEndianArrayToUint16(uint8_t * array)
{
    return (((uint16_t) array[0]) << 8) + ((uint16_t)array[1]);
}

#define CMD_I2C_WRITE   1
#define I2CWR_LEN_POS       0
#define I2CWR_CTRLBYTE_POS  2
#define I2CWR_DATA_POS      3

#define CMD_I2C_READ    2
#define I2CRD_CMDLEN_POS    0
#define I2CRD_CTRLBYTE_POS  2
#define I2CRD_LEN_POS       3
#define I2CRD_CMDDATA_POS   5

uint8_t I2C_Write(uint8_t controlByte, uint8_t * data, uint16_t length)
{
    uint16_t i;
    IdleI2C();
    StartI2C();
    while ( SSPCON2bits.SEN );
    WriteI2C(controlByte);
    IdleI2C();
    for (i = 0; i < length; i++)
    {
        WriteI2C(data[i]);
        IdleI2C();
    }
    StopI2C();
    while ( SSPCON2bits.PEN );
    return 0;
}

uint8_t I2C_Read(uint8_t controlByte, uint8_t * cmd, uint16_t cmd_len,
        uint8_t * data, uint16_t data_len)
{
    uint16_t i;
    IdleI2C();
    StartI2C();
    while (SSPCON2bits.SEN);
    WriteI2C(controlByte);
    IdleI2C();
    for (i = 0; i < cmd_len; i++)
    {
        WriteI2C(cmd[i]);
        IdleI2C();
    }
    RestartI2C();
    while (SSPCON2bits.RSEN);
    WriteI2C(controlByte | 0x01);
    IdleI2C();
    getsI2C(data, data_len);
    NotAckI2C();
    while (SSPCON2bits.ACKEN);
    StopI2C();
    while (SSPCON2bits.PEN);
    return 0;
}

int16_t executeCommand(uint8_t cmd, uint8_t * payload, uint8_t * ack_msg, uint16_t * ack_length)
{
    uint16_t length, cmd_length;
    uint8_t controlByte;
    switch (cmd)
    {
        case CMD_I2C_WRITE:
            length = BigEndianArrayToUint16(payload + I2CWR_LEN_POS);
            controlByte = payload[I2CWR_CTRLBYTE_POS];
            I2C_Write(controlByte, payload+I2CWR_DATA_POS, length);
            break;
        case CMD_I2C_READ:
            cmd_length = BigEndianArrayToUint16(payload + I2CRD_CMDLEN_POS);
            controlByte = payload[I2CRD_CTRLBYTE_POS];
            (*ack_length) = BigEndianArrayToUint16(payload + I2CRD_LEN_POS);
            I2C_Read(controlByte, payload + I2CRD_CMDDATA_POS, cmd_length, ack_msg, *ack_length);
            break;
    }
    return 0;
}

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

int16_t processCommand(uint8_t * buffer, uint16_t length, uint8_t * ack_msg, uint16_t * ack_length)
{
    uint16_t cmd_len;
    uint16_t i;
    uint8_t checksum = 0;
    if (length < 6)
        return RET_CMD_INCOMPLETE;
    if ((buffer[0] & buffer[1] & SYNCH_BYTE) != SYNCH_BYTE)
        return RET_CMD_NOSYNCH;
    cmd_len = BigEndianArrayToUint16(buffer + LENGTH_POS);
    if (cmd_len > (length - START_CHECKSUM)) // Synch not included in command length
    {
        return RET_CMD_INCOMPLETE;
    }
    for (i = START_CHECKSUM; i < cmd_len + START_CHECKSUM; i++)
    {
        checksum += buffer[i];
    }
    if (checksum != buffer[cmd_len + START_CHECKSUM])
    {
        return RET_CMD_CORRUPT;
    }
    if (executeCommand(buffer[COMMAND_POS], buffer+COMMAND_POS+1, ack_msg, ack_length) < 0)
    {
        return RET_CMD_FAILED;
    }
    return (START_CHECKSUM + cmd_len + 1);
}

uint8_t cmd_buffer[64];
uint8_t ack_buffer[64];
uint16_t buffer_len;
uint16_t ack_len;

void yasp_Init()
{
    for (buffer_len = 0; buffer_len < 64; buffer_len++)
        cmd_buffer[buffer_len] = 0;
    buffer_len = 0;
    // I2C Initialization
    TRISBbits.TRISB0 = 0;
    TRISBbits.TRISB1 = 0;
    ADCON1bits.PCFG = 0xF;
    SSPADD = 0x3F;
    OpenI2C(MASTER, SLEW_OFF);

}

void serial_tx(uint8_t * data, uint16_t length)
{
    if(mUSBUSARTIsTxTrfReady() == true)
    {
        putUSBUSART(data, length);
        return;
    }
    return;
}

void _resynch(uint16_t offset)
{
    uint16_t i;
    for (i = 0; i < buffer_len - offset; i++)
    {
        cmd_buffer[i] = cmd_buffer[i+offset];
    }
    buffer_len = buffer_len - offset;
}

void yaspService()
{
    uint8_t cmd[5];
    cmd[0] = 0x00;
    cmd[1] = 0x00;
    //I2C_Write(0x50, cmd, 2);
    //return;
    uint16_t i = 1;
    int16_t return_code = 0;
    if( USBUSARTIsTxTrfReady() == true)
    {
        buffer_len += getsUSBUSART(cmd_buffer + buffer_len, 64);
    }
    if (buffer_len > 0)
    {
        return_code = processCommand(cmd_buffer, buffer_len, ack_buffer, &ack_len);
        //sprintf(ack_buffer, "RX(%d,%d):%02x%02x%02x%02x%02x", return_code, buffer_len, cmd_buffer[3], cmd_buffer[4], cmd_buffer[5], cmd_buffer[6], cmd_buffer[7]);
        //serial_tx(ack_buffer, 30);
        if (return_code > 0)
        {
            // send ACK ok
            //serial_tx(ack_buffer, ack_len);
            sprintf(ack_buffer, "Success!");
            serial_tx(ack_buffer, 8);
            _resynch(return_code);
        }
        else if (return_code == RET_CMD_NOSYNCH)
        {
            sprintf(ack_buffer, "Resynch!");
            serial_tx(ack_buffer, 8);
            while ((cmd_buffer[i++] != SYNCH_BYTE) && (i < buffer_len));
            _resynch(i);
        }
        else if (return_code == RET_CMD_CORRUPT)
        {
            // Send error ACK
            sprintf(ack_buffer, "Corrupt!");
            serial_tx(ack_buffer, 8);
            //ack_buffer[0] = 0xFF;
            //ack_buffer[1] = 0xFE;
            //serial_tx(ack_buffer, ack_len);
            buffer_len = 0;
        }
        else if (return_code == RET_CMD_FAILED)
        {
            // Send failed ACK
            sprintf(ack_buffer, "Failed!");
            serial_tx(ack_buffer, 8);
            buffer_len = 0;
        }
    }
}
