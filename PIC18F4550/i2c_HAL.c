#include <stdint.h>
#include <stdbool.h>
#include <i2c.h>

void I2C_Init()
{
    TRISBbits.TRISB0 = 0;
    TRISBbits.TRISB1 = 0;
    ADCON1bits.PCFG = 0xF;
    SSPADD = 0x3F;
    OpenI2C(MASTER, SLEW_OFF);

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