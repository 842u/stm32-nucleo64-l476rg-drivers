#ifndef HW390_H
#define HW390_H

#include "stm32l4xx_hal.h"
#include <stdbool.h>
#include <stdint.h>

#define HW390_CALIBRATION_STRUCT_MAGIC 0xDEADBEEF // CAFE BABE :)

typedef struct {
  uint32_t magic; // To identify if it is a correct structure
  uint32_t sensor_id;
  uint32_t dry_value;
  uint32_t wet_value;
} HW390_CalibrationTypeDef;

typedef struct {
  ADC_HandleTypeDef hadc;
  HW390_CalibrationTypeDef calibration;
  uint32_t calibration_flash_address;
} HW390_HandleTypeDef;

void hw390_init(HW390_HandleTypeDef *hhw390, ADC_HandleTypeDef *hadc,
                uint32_t sensor_id, uint32_t calibration_flash_address);

uint32_t hw390_read_data(HW390_HandleTypeDef *hhw390, uint16_t timeout_ms);
uint32_t hw390_read_average_data(HW390_HandleTypeDef *hhw390, uint16_t samples,
                                 uint16_t delay_ms);

void hw390_calibrate(HW390_HandleTypeDef *hhw390, bool is_dry);
void hw390_save_calibration(HW390_HandleTypeDef *hhw390);
bool hw390_load_calibration(HW390_HandleTypeDef *hhw390);
void hw390_erase_calibration(HW390_HandleTypeDef *hhw390);

void hw390_debug_flash(HW390_HandleTypeDef *hhw390);

uint8_t hw390_get_moisture_percent(HW390_HandleTypeDef *hhw390,
                                   uint32_t adc_value);

#endif
