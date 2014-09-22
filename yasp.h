
// Set the callback when a specific yasp command is received
void registerYaspCommand(void(*)(uint8_t *, uint8_t), uint8_t);

// Attach yasp to the serial port
void yasp_Init();