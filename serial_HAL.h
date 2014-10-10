// These functions must be created in the board support package
#ifndef _SERIAL_H_
#define _SERIAL_H_

typedef uint16_t(*serial_rx_callback)(uint8_t *, uint16_t);

// Sets the callback for when serial data is received
//
// This must be implemented specifically for the target platform
void setSerialRxHandler(serial_rx_callback);

// Transmit over serial
//
// This must be implemented specifically for the target platform
void serial_tx(uint8_t *, uint16_t);

#endif
