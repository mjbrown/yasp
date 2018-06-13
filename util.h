#ifndef YASP_UTIL_H
#define YASP_UTIL_H

#include <stdint.h>

void toUintLEArray( uint32_t value, uint8_t *target, uint8_t length );

uint32_t LEtoUint( uint8_t *value, uint8_t bytes );

uint32_t crc32(uint8_t const message[], int nBytes, uint32_t start_value);

#endif //YASP_UTIL_H
