#include "hw390.h"
#include "adc.h"
#include "stm32l4xx_hal.h"
#include "stm32l4xx_hal_adc.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

void hw390_init(HW390_HandleTypeDef *hhw390, ADC_HandleTypeDef *hadc,
                uint32_t sensor_id, uint32_t calibration_flash_address) {
  hhw390->hadc = *hadc;
  hhw390->calibration.sensor_id = sensor_id;
  hhw390->calibration.magic = HW390_CALIBRATION_STRUCT_MAGIC;
  hhw390->calibration_flash_address = calibration_flash_address;
  hhw390->calibration.dry_value = 0;
  hhw390->calibration.wet_value = 0;
}

uint32_t hw390_read_data(HW390_HandleTypeDef *hhw390, uint16_t timeout_ms) {
  uint32_t data = 0;

  HAL_ADC_Start(&hhw390->hadc);
  if (HAL_ADC_PollForConversion(&hhw390->hadc, timeout_ms) == HAL_OK) {
    data = HAL_ADC_GetValue(&hhw390->hadc);
  }
  HAL_ADC_Stop(&hhw390->hadc);

  return data;
}

uint32_t hw390_read_average_data(HW390_HandleTypeDef *hhw390, uint16_t samples,
                                 uint16_t delay_ms) {
  uint64_t sum = 0;

  for (uint16_t i = 0; i < samples; i++) {
    sum += hw390_read_data(hhw390, 100);
    HAL_Delay(delay_ms);
  }

  return (uint32_t)(sum / samples);
}

void hw390_calibrate(HW390_HandleTypeDef *hhw390, bool is_dry) {
  uint32_t value = hw390_read_average_data(hhw390, 100, 100);

  if (is_dry) {
    hhw390->calibration.dry_value = value;
  } else {
    hhw390->calibration.wet_value = value;
  }
}

void hw390_save_calibration(HW390_HandleTypeDef *hhw390) {
  HAL_FLASH_Unlock();

  /**
   * STM32L476RG Flash Layout
   * 1 MB flash = 512 pages × 2 KB each
   * Bank 1: 0x08000000 - 0x0807FFFF (Pages 0-255)
   * Bank 2: 0x08080000 - 0x080FFFFF (Pages 256-511)
   * 72-bit wide data read/write
   */

  /**
   * Example:
   * calibration_flash_address = 0x080FF000
   * FLASH_BASE = 0x08000000
   * address_offset = 0x080FF000 - 0x08000000 = 0xFF000 = 1,044,480 bytes
   */
  uint32_t address_offset = hhw390->calibration_flash_address - FLASH_BASE;

  /**
   * Example:
   * address_offset = 1,044,480 bytes
   * FLASH_PAGE_SIZE = 2048 bytes
   * page = 1,044,480 / 2048 = 510
   */
  uint32_t absolute_page = address_offset / FLASH_PAGE_SIZE;

  uint32_t bank = (absolute_page < 256) ? FLASH_BANK_1 : FLASH_BANK_2;

  uint32_t bank_page =
      (absolute_page < 256) ? absolute_page : (absolute_page - 256);

  // Erase page
  FLASH_EraseInitTypeDef eraseInit;
  uint32_t pageError;
  eraseInit.TypeErase = FLASH_TYPEERASE_PAGES;
  eraseInit.Banks = bank;
  eraseInit.Page = bank_page;
  eraseInit.NbPages = 1;

  printf("Erasing bank %u, page %u...\r\n", bank, bank_page);
  HAL_FLASHEx_Erase(&eraseInit, &pageError);

  uint32_t calibration_address = hhw390->calibration_flash_address;
  uint64_t *data = (uint64_t *)(&hhw390->calibration);

  size_t writes_number = sizeof(HW390_CalibrationTypeDef) / 8;
  printf("Writing %u x 64-bit values...\r\n", writes_number);

  for (size_t i = 0; i < writes_number; i++) {
    HAL_StatusTypeDef status = HAL_FLASH_Program(
        FLASH_TYPEPROGRAM_DOUBLEWORD, calibration_address + i * 8, data[i]);
    if (status != HAL_OK) {
      printf("Write failed at offset %u! Error: 0x%08X\r\n", i * 8,
             HAL_FLASH_GetError());
    }
  }

  HAL_FLASH_Lock();
  printf("Flash write complete\r\n");
}

bool hw390_load_calibration(HW390_HandleTypeDef *hhw390) {
  HW390_CalibrationTypeDef *calibration = &hhw390->calibration;
  HW390_CalibrationTypeDef *flash_calibration =
      (HW390_CalibrationTypeDef *)hhw390->calibration_flash_address;

  uint32_t sensor_id = hhw390->calibration.sensor_id;

  if (flash_calibration->magic != HW390_CALIBRATION_STRUCT_MAGIC) {
    printf("Invalid magic number!\r\n");
    return false;
  }

  if (flash_calibration->sensor_id == sensor_id) {
    *calibration = *flash_calibration;
    return true;
  }

  return false;
}

void hw390_erase_calibration(HW390_HandleTypeDef *hhw390) {
  HAL_FLASH_Unlock();

  uint32_t address_offset = hhw390->calibration_flash_address - FLASH_BASE;
  uint32_t absolute_page = address_offset / FLASH_PAGE_SIZE;
  uint32_t bank = (absolute_page < 256) ? FLASH_BANK_1 : FLASH_BANK_2;
  uint32_t bank_page =
      (absolute_page < 256) ? absolute_page : (absolute_page - 256);

  printf("Address: 0x%08X\r\n", hhw390->calibration_flash_address);
  printf("Offset: %u bytes\r\n", address_offset);
  printf("Absolute page: %u\r\n", absolute_page);
  printf("Bank: %u, Page in bank: %u\r\n", bank, bank_page);

  FLASH_EraseInitTypeDef eraseInit;
  uint32_t pageError = 0;
  eraseInit.TypeErase = FLASH_TYPEERASE_PAGES;
  eraseInit.Banks = bank;
  eraseInit.Page = bank_page;
  eraseInit.NbPages = 1;

  HAL_StatusTypeDef status = HAL_FLASHEx_Erase(&eraseInit, &pageError);

  if (status != HAL_OK) {
    printf("Erase FAILED! Status: %d, PageError: %u\r\n", status, pageError);
    printf("Flash Error Code: 0x%08X\r\n", HAL_FLASH_GetError());
  } else {
    printf("Erase SUCCESS!\r\n");
  }

  HAL_FLASH_Lock();
}

void hw390_debug_flash(HW390_HandleTypeDef *hhw390) {
  HW390_CalibrationTypeDef *flash_calibration =
      (HW390_CalibrationTypeDef *)hhw390->calibration_flash_address;

  printf("=== Flash Debug ===\r\n");
  printf("Address: 0x%08X\r\n", hhw390->calibration_flash_address);
  printf("Flash ID: 0x%08X\r\n", flash_calibration->sensor_id);
  printf("Flash Dry: %u\r\n", flash_calibration->dry_value);
  printf("Flash Wet: %u\r\n", flash_calibration->wet_value);
  printf("Expected ID: 0x%08X\r\n", hhw390->calibration.sensor_id);
  printf("==================\r\n");
}

uint8_t hw390_get_moisture_percent(HW390_HandleTypeDef *hhw390,
                                   uint32_t adc_value) {
  uint32_t dry = hhw390->calibration.dry_value;
  uint32_t wet = hhw390->calibration.wet_value;

  if (dry == 0 || wet == 0 || dry == wet) {
    return 0;
  }

  if (adc_value >= dry)
    return 0;
  if (adc_value <= wet)
    return 100;

  // Linear interpolation
  return ((dry - adc_value) * 100) / (dry - wet);
}
