#include <stdbool.h>
#include "util.h"

void toUintLEArray( uint32_t value, uint8_t *target, uint8_t bytes )
{
    uint8_t i=0;
    for ( i = 0; i < bytes; i++ ) {
        target[i] = ( value >> ( 8 * i ) ) & 0xFF;
    }
}

uint32_t LEtoUint( uint8_t *src, uint8_t len )
{
    uint32_t value = 0;
    uint8_t i=0;

    for ( i = 0; i < len; i++ ) {
        value += ( ( ( uint32_t ) src[i] ) << ( 8 * i ) );
    }
    return value;
}

#define POLYNOMIAL  0x8005
#define WIDTH       (8 * sizeof(crc_t))
#define TOPBIT      (1 << (WIDTH - 1))

crc_t  crcTable[256];

void crc_init(void) {
    crc_t  remainder;
    for (int dividend = 0; dividend < 256; ++dividend) {
        remainder = dividend << (WIDTH - 8);
        for (uint8_t bit = 8; bit > 0; --bit) {
            if (remainder & TOPBIT) {
                remainder = (remainder << 1) ^ POLYNOMIAL;
            } else {
                remainder = (remainder << 1);
            }
        }
        crcTable[dividend] = remainder;
    }
}

crc_t crc16(uint8_t const message[], uint32_t nBytes, crc_t initial) {
    uint8_t data;
    crc_t remainder = initial;
    for (int byte = 0; byte < nBytes; ++byte) {
        data = message[byte] ^ (remainder >> (WIDTH - 8));
        remainder = crcTable[data] ^ (remainder << 8);
    }
    return remainder;
}
