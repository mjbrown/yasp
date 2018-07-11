#include "fifo.h"
#include "string.h"

uint32_t fifo_bytes_used( fifo_t *fifo )
{
    if ( fifo->start > fifo->end ) {
        return ( fifo->size - fifo->start ) + fifo->end;
    } else {
        return ( fifo->end - fifo->start );
    }
}

uint32_t fifo_bytes_free( fifo_t *fifo )
{
    return fifo->size - fifo_bytes_used( fifo ) - 1;
}

void fifo_get( fifo_t *fifo, uint8_t *dest, uint32_t size )
{
    // C Syntax: BOOLEAN ? if TRUE : if FALSE
    uint32_t max_copy1_size = ( fifo->start > fifo->end ) ? ( fifo->size - fifo->start ) : ( fifo->end - fifo->start );
    uint32_t copy1_size = ( max_copy1_size > size ) ? size : max_copy1_size;
    memcpy( dest, fifo->data + fifo->start, copy1_size );
    fifo->start = ( fifo->start + copy1_size ) % fifo->size;

    // It is OK to do a memcpy of ZERO bytes
    uint32_t copy2_size = ( size - copy1_size );
    memcpy( dest + copy1_size, fifo->data + fifo->start, copy2_size );
    fifo->start = ( fifo->start + copy2_size ) % fifo->size;
}

void fifo_put( fifo_t *fifo, uint8_t *src, uint32_t size )
{
    // C Syntax: BOOLEAN ? if TRUE : if FALSE
    uint32_t max_copy1_size = ( fifo->start > fifo->end ) ? ( fifo->start - fifo->end ) : ( fifo->size - fifo->end );
    uint32_t copy1_size = ( max_copy1_size > size ) ? size : max_copy1_size;
    memcpy( fifo->data + fifo->end, src, copy1_size );
    fifo->end = ( fifo->end + copy1_size ) % fifo->size;

    // It is OK to do a memcpy of ZERO bytes
    uint32_t copy2_size = ( size - copy1_size );
    memcpy( fifo->data + fifo->end, src + copy1_size, copy2_size );
    fifo->end = ( fifo->end + copy2_size ) % fifo->size;
}

void fifo_peek( fifo_t *fifo, uint8_t *dest, uint32_t offset, uint32_t size )
{
    // C Syntax: BOOLEAN ? if TRUE : if FALSE
    uint32_t temp_start = ( fifo->start + offset ) % fifo->size;
    uint32_t max_copy1_size = ( temp_start > fifo->end ) ? ( fifo->size - temp_start ) : ( fifo->end - temp_start );
    uint32_t copy1_size = ( max_copy1_size > size ) ? size : max_copy1_size;
    memcpy( dest, fifo->data + temp_start, copy1_size );
    uint32_t next_start = ( temp_start + copy1_size ) % fifo->size;

    // It is OK to do a memcpy of ZERO bytes
    uint32_t copy2_size = ( size - copy1_size );
    memcpy( dest + copy1_size, fifo->data + next_start, copy2_size );
}

void fifo_destroy( fifo_t *fifo, uint32_t size )
{
    fifo->start = ( fifo->start + size ) % fifo->size;
}
/*
void print_fifo_info(fifo_t * fifo) {
    printf("FIFO Size: %d, Start: %d, End: %d, Used: %d\nContents: ", fifo->size, fifo->start, fifo->end, fifo_bytes_used(fifo));
    for (uint32_t i = 0; i < fifo_bytes_used(fifo); i++) {
        uint8_t data;
        fifo_peek(fifo, &data, i, 1);
        printf("%02X", data);
    }
    printf("\n");
}
*/
