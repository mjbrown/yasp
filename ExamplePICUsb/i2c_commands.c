#include <stdint.h>
#include <i2c.h>

#include "../yasp.h"

uint16_t BigEndianArrayToUint16(uint8_t * array)
{
    return (((uint16_t) array[0]) << 8) + ((uint16_t)array[1]);
}

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

#define I2CWR_LEN_POS       0
#define I2CWR_CTRLBYTE_POS  2
#define I2CWR_DATA_POS      3
void i2c_write_command(uint8_t * payload, uint8_t payload_len) {
    uint16_t length;
    uint8_t controlByte;
    length = BigEndianArrayToUint16(payload + I2CWR_LEN_POS);
    controlByte = payload[I2CWR_CTRLBYTE_POS];
    I2C_Write(controlByte, payload+I2CWR_DATA_POS, length);
}

#define CMD_I2C_WRITE   1
#define CMD_I2C_READ    2

void init_i2c_commands() {
    TRISBbits.TRISB0 = 0;
    TRISBbits.TRISB1 = 0;
    ADCON1bits.PCFG = 0xF;
    SSPADD = 0x3F;
    OpenI2C(MASTER, SLEW_OFF);
    register_yasp_command((command_callback) i2c_write_command, CMD_I2C_WRITE);
    //register_yasp_command(i2c_read_command, CMD_I2C_READ);
}

/*
#define I2CRD_CMDLEN_POS    0
#define I2CRD_CTRLBYTE_POS  2
#define I2CRD_LEN_POS       3
#define I2CRD_CMDDATA_POS   5
void i2c_read_command(uint8_t * payload, uint8_t payload_len) {
    cmd_length = BigEndianArrayToUint16(payload + I2CRD_CMDLEN_POS);
    controlByte = payload[I2CRD_CTRLBYTE_POS];
    (*ack_length) = BigEndianArrayToUint16(payload + I2CRD_LEN_POS);
    I2C_Read(controlByte, payload + I2CRD_CMDDATA_POS, cmd_length, ack_msg, *ack_length);
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
*/