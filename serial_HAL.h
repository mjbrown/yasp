// These functions must be created in the board support package
#ifndef _SERIAL_H_
#define _SERIAL_H_

// Sets the callback for when serial data is received
void setSerialRxHandler(void(*)(uint8_t *, uint16_t, uint16_t*));

// Transmit over serial, buffered
void serial_tx(uint8_t *, uint16_t);

// Polls for serial data, transmits buffer
void serialService();

#endif
