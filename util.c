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

#define POLYNOMIAL  0xD8
#define WIDTH       (8 * sizeof(uint32_t))
#define TOPBIT      (1 << (WIDTH - 1))

uint32_t crc32(uint8_t const message[], int nBytes, uint32_t start_value) {
    int byte, bit;
    uint32_t  remainder = start_value;
    for (byte = 0; byte < nBytes; ++byte) {
        remainder ^= (message[byte] << (WIDTH - 8));
        for (bit = 8; bit > 0; --bit) {
            if (remainder & TOPBIT) {
                remainder = (remainder << 1) ^ POLYNOMIAL;
            } else {
                remainder = (remainder << 1);
            }
        }
    }
    return (remainder);
}
