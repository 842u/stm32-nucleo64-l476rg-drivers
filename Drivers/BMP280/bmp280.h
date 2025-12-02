#ifndef BMP280_H
#define BMP280_H

#include "stm32l4xx_hal.h"
#include <stdint.h>

/* BMP280 I2C Address */
#define BMP280_ADDRESS_0 (0x76 << 1)
#define BMP280_ADDRESS_1 (0x77 << 1)

/* BMP280 Registers */
#define BMP280_REG_CHIP_ID 0xD0
#define BMP280_REG_RESET 0xE0
#define BMP280_REG_STATUS 0xF3
#define BMP280_REG_CTRL_MEAS 0xF4
#define BMP280_REG_CONFIG 0xF5
#define BMP280_REG_PRESS_MSB 0xF7
#define BMP280_REG_TEMP_MSB 0xFA
#define BMP280_REG_TRIMM_PARAM 0x88

/* BMP280 Trimming Parameters Registers Count */
#define BMP280_TRIMM_PARAM_REGISTERS_COUNT 24

/* BMP280 Chip ID */
#define BMP280_CHIP_ID 0x58

/* Standby Time */
#define BMP280_STANDBY_0_5MS 0x00
#define BMP280_STANDBY_62_5MS 0x01
#define BMP280_STANDBY_125MS 0x02
#define BMP280_STANDBY_250MS 0x03
#define BMP280_STANDBY_500MS 0x04
#define BMP280_STANDBY_1000MS 0x05
#define BMP280_STANDBY_2000MS 0x06
#define BMP280_STANDBY_4000MS 0x07

/* IIR Filter Coefficient */
#define BMP280_FILTER_OFF 0x00
#define BMP280_FILTER_2 0x01
#define BMP280_FILTER_4 0x02
#define BMP280_FILTER_8 0x03
#define BMP280_FILTER_16 0x04

/* Oversampling */
#define BMP280_OVERSAMPLING_SKIP 0x00
#define BMP280_OVERSAMPLING_1X 0x01
#define BMP280_OVERSAMPLING_2X 0x02
#define BMP280_OVERSAMPLING_4X 0x03
#define BMP280_OVERSAMPLING_8X 0x04
#define BMP280_OVERSAMPLING_16X 0x05

/* Power Modes */
#define BMP280_MODE_SLEEP 0x00
#define BMP280_MODE_FORCED 0x01
#define BMP280_MODE_NORMAL 0x03

/* BMP280 Handle Structure */
typedef struct {
  I2C_HandleTypeDef *hi2c;
  uint16_t address;

  /* Calibration data */
  uint16_t dig_T1;
  int16_t dig_T2;
  int16_t dig_T3;
  uint16_t dig_P1;
  int16_t dig_P2;
  int16_t dig_P3;
  int16_t dig_P4;
  int16_t dig_P5;
  int16_t dig_P6;
  int16_t dig_P7;
  int16_t dig_P8;
  int16_t dig_P9;

  int32_t t_fine; // Used in pressure calculation
} BMP280_HandleTypeDef;

/* Function Prototypes */
HAL_StatusTypeDef BMP280_Init(BMP280_HandleTypeDef *hbmp280,
                              I2C_HandleTypeDef *hi2c, uint16_t address);
HAL_StatusTypeDef BMP280_Configure(BMP280_HandleTypeDef *hbmp280,
                                   uint8_t standby_time, uint8_t filter,
                                   uint8_t temp_oversampling,
                                   uint8_t press_oversampling, uint8_t mode);
HAL_StatusTypeDef BMP280_ReadTemperature(BMP280_HandleTypeDef *hbmp280,
                                         float *temperature);
HAL_StatusTypeDef BMP280_ReadPressure(BMP280_HandleTypeDef *hbmp280,
                                      float *pressure);
HAL_StatusTypeDef BMP280_ReadAll(BMP280_HandleTypeDef *hbmp280,
                                 float *temperature, float *pressure);

#endif
