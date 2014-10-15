/* Example of a serial HAL meant for PIC USB serial */
#include <stdint.h>
#include <usb/usb.h>
#include <usb/usb_device_cdc.h>

#include "../serial_HAL.h"

#define BUFFER_SIZE     64

static serial_rx_callback rx_callback = NULL;

static uint8_t buffer[BUFFER_SIZE];
static uint16_t buffer_len = 0;

static uint8_t txbuffer[BUFFER_SIZE];
static uint16_t tx_len = 0;

/* Callback returns number of bytes processed */
void setSerialRxHandler(serial_rx_callback callback) {
    rx_callback = callback;
}

void serial_tx(uint8_t * data, uint16_t len) {
    uint16_t i = 0;
    for (i = 0; i < len; i++) {
        txbuffer[tx_len++] = data[i];
    }
}

void serialService() {
    uint16_t rxsize = 0;
    uint16_t rxhandled = 0;
    uint16_t i;
    if (USBUSARTIsTxTrfReady() == true)
    {
        rxsize = getsUSBUSART(buffer + buffer_len, BUFFER_SIZE - buffer_len);
    }
    if (rxsize > 0) {
        buffer_len += rxsize;
        rxhandled = rx_callback(buffer, buffer_len);
        for (i = 0; i < rxhandled; i++)
        {
            buffer[i] = buffer[i+rxhandled];
        }
        buffer_len = buffer_len - rxhandled;
    }
    if ((mUSBUSARTIsTxTrfReady() == true) && (tx_len > 0))
    {
        putUSBUSART(txbuffer, tx_len);
        tx_len = 0;
    }
}

uint16_t get_serial_buffer_size()
{
    return BUFFER_SIZE;
}