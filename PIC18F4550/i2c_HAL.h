/* 
 * File:   i2c_HAL.h
 * Author: Michael
 *
 * Created on September 6, 2014, 11:48 AM
 */

#include <stdint.h>

#ifndef I2C_HAL_H
#define	I2C_HAL_H

#ifdef	__cplusplus
extern "C" {
#endif

    extern uint8_t I2C_Write(uint8_t, uint8_t *, uint16_t);
    extern uint8_t I2C_Read(uint8_t, uint8_t *, uint16_t, uint8_t *, uint16_t);


#ifdef	__cplusplus
}
#endif

#endif	/* I2C_HAL_H */

