#include "util.h"
#include "packetize.h"

// TODO: Remove this include if you do not want error messages
#include "error.h"

// TODO: Set a size to zero to eliminate the field
#define SYNC_SIZE           2
#define CHECKSUM_SIZE       2
#define CRC_SIZE            2

#define PROTOCOL_OVERHEAD   (SYNC_SIZE + sizeof(data_length_t) + sizeof(handle_t) + sizeof(command_t) + CHECKSUM_SIZE + CRC_SIZE)

typedef struct {
    command_t cmd;
    cmd_handler_t cmd_handler;
} cmd_table_entry;

static cmd_table_entry cmd_table[MAX_NUMBER_OF_CMDS];
static uint32_t cmd_table_length = 0;

void register_cmd_handler(command_t cmd, cmd_handler_t cmd_handler) {
    cmd_table[cmd_table_length].cmd = cmd;
    cmd_table[cmd_table_length].cmd_handler = cmd_handler;
    cmd_table_length += 1;
}

void packetize_data(command_t cmd, handle_t cmd_handle, uint8_t * data, data_length_t length, fifo_t * p_fifo) {
    uint8_t header[PROTOCOL_OVERHEAD];
    uint8_t * p_header = header;
#if (SYNC_SIZE > 0)
    toUintLEArray( SYNC_VALUE, p_header, SYNC_SIZE);
    p_header += SYNC_SIZE;
#endif
    toUintLEArray( length, p_header, sizeof(data_length_t));
    p_header += sizeof(data_length_t);
    toUintLEArray( cmd_handle, p_header, sizeof(handle_t));
    p_header += sizeof(handle_t);
    toUintLEArray( cmd, p_header, sizeof(command_t));
    p_header += sizeof(command_t);

#if (CHECKSUM_SIZE > 0)
    checksum_t checksum = 0;
    for (uint32_t i = 0; i < (SYNC_SIZE + sizeof(data_length_t) + sizeof(command_t)); i++) {
        checksum += header[i];
    }
    for (uint32_t i = 0; i < length; i++) {
        checksum += data[i];
    }
    toUintLEArray( checksum, p_header, CHECKSUM_SIZE);
    p_header += CHECKSUM_SIZE;
#endif

#if (CRC_SIZE > 0)
    crc_t crc = crc16(header, SYNC_SIZE + sizeof(data_length_t) + sizeof(command_t) + CHECKSUM_SIZE, 0);
    toUintLEArray(crc16(data, length, crc), p_header, CRC_SIZE);
#endif

    fifo_put(p_fifo, header, PROTOCOL_OVERHEAD);
    fifo_put(p_fifo, data, length);
}


bool depacketize_data(fifo_t * rx_fifo, fifo_t * err_fifo) {
#if (SYNC_SIZE > 0)
    static bool sync_ok = true;
    while (fifo_bytes_used(rx_fifo) >= PROTOCOL_OVERHEAD) {
        uint8_t sync[SYNC_SIZE];
        fifo_peek(rx_fifo, sync, 0, 2);
        if (LEtoUint(sync, SYNC_SIZE) != SYNC_VALUE) {
            fifo_destroy(rx_fifo, 1);
            if (sync_ok) {
                error_msg((uint8_t *)"SYNC", sizeof("SYNC"), err_fifo);
            }
            sync_ok = false;
        } else {
            sync_ok = true;
            break;
        }
    }
#endif

    if (fifo_bytes_used(rx_fifo) < PROTOCOL_OVERHEAD) {
        return false;
    }
    uint8_t header[PROTOCOL_OVERHEAD];
    fifo_peek(rx_fifo, header, 0, PROTOCOL_OVERHEAD);
    data_length_t msg_length = (data_length_t )LEtoUint(header + SYNC_SIZE, sizeof(data_length_t));
    if (msg_length > MAX_DATA_LENGTH) {
        fifo_destroy(rx_fifo, SYNC_SIZE + sizeof(data_length_t));
#ifdef YASP_ERROR_H
        if (err_fifo != NULL)
            error_msg((uint8_t *)"MAX DATA LENGTH", sizeof("MAX DATA LENGTH"), err_fifo);
#endif
        return true;
    }
    if (fifo_bytes_used(rx_fifo) < (PROTOCOL_OVERHEAD + msg_length)) {
        return false;
    }
    handle_t handle = (handle_t) LEtoUint(header + SYNC_SIZE + sizeof(data_length_t), sizeof(handle_t));
    command_t cmd = (command_t) LEtoUint(header + SYNC_SIZE + sizeof(data_length_t) + sizeof(handle_t), sizeof(command_t));
    fifo_destroy(rx_fifo, PROTOCOL_OVERHEAD);
    uint8_t data_buffer[MAX_DATA_LENGTH];
    fifo_get(rx_fifo, data_buffer, msg_length);
#if (CHECKSUM_SIZE > 0)
    checksum_t checksum = 0;
    for (uint32_t i = 0; i < (SYNC_SIZE + sizeof(data_length_t) + sizeof(command_t)); i++) {
        checksum += header[i];
    }
    for (uint32_t i = 0; i < msg_length; i++) {
        checksum += data_buffer[i];
    }
    checksum_t actual = (checksum_t) LEtoUint(header + SYNC_SIZE + sizeof(data_length_t) + sizeof(command_t) +
            sizeof(handle_t), CHECKSUM_SIZE);
    if (actual != checksum) {
#ifdef YASP_ERROR_H
        if (err_fifo != NULL)
            error_msg((uint8_t *)"CHECKSUM FAIL", sizeof("CHECKSUM FAIL"), err_fifo);
#endif
        return true;
    }
#endif

#if (CRC_SIZE > 0)
    crc_t calc_crc = crc16(header, SYNC_SIZE + sizeof(data_length_t) + sizeof(command_t) + CHECKSUM_SIZE, 0);
    calc_crc = crc16(data_buffer, msg_length, calc_crc);
    crc_t actual_crc = (crc_t) LEtoUint(header + SYNC_SIZE + sizeof(data_length_t) + sizeof(command_t) + sizeof(handle_t) + CHECKSUM_SIZE, CRC_SIZE);
    if (calc_crc != actual_crc) {
#ifdef YASP_ERROR_H
        if (err_fifo != NULL)
            error_msg((uint8_t *)"CRC FAIL", sizeof("CRC FAIL"), err_fifo);
#endif
        return true;
    }
#endif
    for (uint32_t i = 0; i < cmd_table_length; i++) {
        if (cmd_table[i].cmd == cmd) {
            if (cmd_table[i].cmd_handler(cmd, handle, data_buffer, msg_length) != RET_OK) {
#ifdef YASP_ERROR_H
                if (err_fifo != NULL)
                    error_msg((uint8_t *)"COMMAND FAIL", sizeof("COMMAND FAIL"), err_fifo);
#endif
            }
            return true;
        }
    }
#ifdef YASP_ERROR_H
    if (err_fifo != NULL)
        error_msg((uint8_t *)"COMMAND NOT FOUND", sizeof("COMMAND NOT FOUND"), err_fifo);
#endif
    return true;
}
