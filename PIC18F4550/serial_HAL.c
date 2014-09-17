#include <stdint.h>
#include <stdbool.h>
#include <usb/usb.h>
#include <usb/usb_device_cdc.h>

uint16_t serial_rx(uint8_t * data)
{
    if( USBUSARTIsTxTrfReady() == true)
    {
        return getsUSBUSART(data, 64);
    }
    else
    {
        return 0;
    }
}

void serial_tx(uint8_t * data, uint16_t length)
{
    if(mUSBUSARTIsTxTrfReady() == true)
    {
        putUSBUSART(data, length);
        return;
    }
    return;
}