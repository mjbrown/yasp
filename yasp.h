#ifndef _YASP_H_
#define _YASP_H_

// Define the length for statically allocating pointers for command callbacks
#define REGISTRY_LENGTH  32

// Built in command for software to configure itself against the target
#define CMD_YASP_INFO   0x7F

typedef void(*command_callback)(uint8_t * payload, uint16_t payload_length);

// Set the callback when a specific yasp command is received
void register_yasp_command(command_callback callback, uint8_t command);

// Attach yasp to the serial port
void yasp_init();

void send_yasp_command(uint8_t cmd, uint8_t * payload, uint16_t payload_length, uint8_t ack);

#endif // _YASP_H_
