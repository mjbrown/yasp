#ifndef YASP_FIFO_H
#define YASP_FIFO_H

#include <stdint.h>
#include <stdio.h>

// Use the accessor functions below, never touch this struct directly!
typedef struct {
    uint32_t start;
    uint32_t end;
    uint8_t * data;
    uint32_t size;
} fifo_t;

uint32_t fifo_bytes_free( fifo_t *fifo );

uint32_t fifo_bytes_used( fifo_t *fifo );

void fifo_get( fifo_t *fifo, uint8_t *dest, uint32_t size );

void fifo_put( fifo_t *fifo, uint8_t *src, uint32_t size );

void fifo_peek( fifo_t *fifo, uint8_t *dest, uint32_t offset, uint32_t size );

void fifo_destroy( fifo_t *fifo, uint32_t size );

#endif //YASP_FIFO_H
