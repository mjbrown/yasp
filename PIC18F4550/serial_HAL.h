/* 
 * File:   serial_HAL.h
 * Author: Michael
 *
 * Created on September 6, 2014, 11:27 PM
 */

#ifndef SERIAL_HAL_H
#define	SERIAL_HAL_H

#ifdef	__cplusplus
extern "C" {
#endif

    uint16_t serial_rx(uint8_t *);
    void serial_tx(uint8_t *, uint16_t);

#ifdef	__cplusplus
}
#endif

#endif	/* SERIAL_HAL_H */

