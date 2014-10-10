#ifndef _YASP_H_
#define _YASP_H_

// Define the length for statically allocating pointers for command callbacks
#define REGISTRY_LENGTH  32

typedef void(*command_callback)(uint8_t * payload, uint16_t payload_length);

// Set the callback when a specific yasp command is received
void register_yasp_command(command_callback callback, uint8_t command);

// Attach yasp to the serial port
void yasp_init();

void send_yasp_ack(uint8_t cmd, uint8_t * payload, uint16_t payload_length);

#endif // _YASP_H_