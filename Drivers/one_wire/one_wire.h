#ifndef ONE_WIRE_H
#define ONE_WIRE_H

#include "delay_us.h"
#include "stm32l4xx_hal.h"

HAL_StatusTypeDef one_wire_reset(void);
void one_wire_write_bit(int value);
int one_wire_read_bit(void);
void one_wire_write_byte(uint8_t byte);
uint8_t one_wire_read_byte(void);

#endif
