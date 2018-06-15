#ifndef YASP_UTIL_H
#define YASP_UTIL_H

#include <stdint.h>

// TODO: Change based upon desired CRC size
typedef uint16_t crc_t;

void toUintLEArray( uint32_t value, uint8_t *target, uint8_t length );

uint32_t LEtoUint( uint8_t *value, uint8_t bytes );

void crc_init(void);

crc_t crc16(uint8_t const message[], uint32_t nBytes, crc_t initial);

#endif //YASP_UTIL_H
