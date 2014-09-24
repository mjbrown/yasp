#ifndef _YASP_H_
#define _YASP_H_

// Set the callback when a specific yasp command is received
void registerYaspCommand(void(*)(uint8_t *, uint16_t), uint8_t);

// Attach yasp to the serial port
void yasp_Init();

#endif // _YASP_H_