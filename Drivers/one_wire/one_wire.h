#ifndef ONE_WIRE_H
#define ONE_WIRE_H

#include "delay_us.h"
#include "main.h"
#include "stm32l4xx_hal.h"
#include <string.h>

/* Persistent Search ROM State */
static uint8_t LastDiscrepancy = 0;
static uint8_t LastFamilyDiscrepancy = 0;
static uint8_t LastDeviceFlag = 0;
static uint8_t ROM_NO[8] = {0};

HAL_StatusTypeDef one_wire_reset(void);
void one_wire_write_bit(uint8_t value);
uint8_t one_wire_read_bit(void);
void one_wire_write_byte(uint8_t byte);
uint8_t one_wire_read_byte(void);
uint8_t one_wire_calculate_crc8(uint8_t *data, uint8_t length);
void one_wire_search_rom_reset(void);
int8_t one_wire_search_rom(uint8_t *rom_code);
int8_t one_wire_get_device_count(void);

#endif
