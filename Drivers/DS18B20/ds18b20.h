#pragma once
#ifndef DS18B20_H
#define DS18B20_H

#include <stdint.h>

/* DS18B20 ROM Commands */
#define DS18B20_MATCH_ROM_CMD 0x55
#define DS18B20_SKIP_ROM_CMD 0xCC

/* DS18B20 Function Commands */
#define DS18B20_CONVERT_T_FN_CMD 0x44
#define DS18B20_READ_SCRATCHPAD_FN_CMD 0xBE
#define DS18B20_WRITE_SCRATCHPAD_FN_CMD 0x4E
#define DS18B20_COPY_SCRATCHPAD_FN_CMD 0x48
#define DS18B20_RECALL_E_FN_CMD 0xB8

/* DS18B20 Configuration Register Resolutions */
#define DS18B20_CFG_REG_9_BIT_RES 0x1F
#define DS18B20_CFG_REG_10_BIT_RES 0x3F
#define DS18B20_CFG_REG_11_BIT_RES 0x5F
#define DS18B20_CFG_REG_12_BIT_RES 0x7F

/* DS18B20 Memmmory Sizes */
#define DS18B20_ROM_BYTES 8
#define DS18B20_SCRATCHPAD_BYTES 9

int8_t ds18b20_start_conversion(uint8_t *rom_code);
int8_t ds18b20_start_conversion_all(void);
int8_t ds18b20_read_temperature(uint8_t *rom_code, float *temperature);
int8_t ds18b20_read_scratchpad(uint8_t *rom_code, uint8_t *scratchpad);
int8_t ds18b20_write_scratchpad(uint8_t *rom_code, int8_t th, int8_t tl,
                                uint8_t config);
int8_t ds18b20_copy_scratchpad(uint8_t *rom_code);

#endif
