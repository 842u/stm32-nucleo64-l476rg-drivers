#include "ds18b20.h"
#include "one_wire.h"
#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Start temperature conversion
 *
 * Initiates a temperature conversion on the specified DS18B20 sensor.
 * The conversion takes approximately 750ms for 12-bit resolution.
 *
 * @param rom_code Pointer to 8-byte ROM code of sensor
 * @retval 0 on success, -1 on error
 *
 * @note Must wait at least 750ms before calling
 * ds18b20_read_temperature()
 * @note For 9-bit: 94ms, 10-bit: 188ms, 11-bit: 375ms, 12-bit: 750ms
 */
int8_t ds18b20_start_conversion(uint8_t *rom_code) {
  if (one_wire_reset() != HAL_OK) {
    return -1;
  }

  one_wire_write_byte(DS18B20_MATCH_ROM_CMD);
  for (int i = 0; i < DS18B20_ROM_BYTES; i++) {
    one_wire_write_byte(rom_code[i]);
  }

  one_wire_write_byte(DS18B20_CONVERT_T_FN_CMD);

  return 0;
}

/**
 * @brief Start conversion on all sensors (broadcast)
 *
 * Starts temperature conversion on ALL DS18B20 sensors on the bus
 * simultaneously. This is more efficient than starting each sensor
 * individually.
 *
 * @retval 0 on success, -1 on error
 *
 * @note All sensors will convert in parallel (still takes 750ms total)
 * @note Use ds18b20_read_temperature() with specific ROM codes to read each
 * sensor
 */
int8_t ds18b20_start_conversion_all(void) {
  if (one_wire_reset() != HAL_OK) {
    return -1;
  }

  one_wire_write_byte(DS18B20_SKIP_ROM_CMD);
  one_wire_write_byte(DS18B20_CONVERT_T_FN_CMD);

  return 0;
}

/**
 * @brief Read temperature from sensor
 *
 * Reads the temperature value from the sensor's scratchpad memory.
 * Must be called after ds18b20_start_conversion() and sufficient wait time.
 *
 * @param rom_code Pointer to 8-byte ROM code of sensor
 * @param temperature Pointer to store temperature value in °C
 * @retval 0 on success, -1 on error
 *
 * @note Temperature resolution: 0.0625°C (12-bit mode)
 * @note Valid range: -55°C to +125°C
 */
int8_t ds18b20_read_temperature(uint8_t *rom_code, float *temperature) {
  uint8_t scratchpad[DS18B20_SCRATCHPAD_BYTES];
  int16_t raw_temp;

  if (one_wire_reset() != HAL_OK) {
    return -1;
  }

  one_wire_write_byte(DS18B20_MATCH_ROM_CMD);
  for (int i = 0; i < DS18B20_ROM_BYTES; i++) {
    one_wire_write_byte(rom_code[i]);
  }

  one_wire_write_byte(DS18B20_READ_SCRATCHPAD_FN_CMD);

  for (int i = 0; i < DS18B20_SCRATCHPAD_BYTES; i++) {
    scratchpad[i] = one_wire_read_byte();
  }

  uint8_t crc = one_wire_calculate_crc8(scratchpad, 8);
  if (crc != scratchpad[8]) {
    return -1;
  }

  raw_temp = (scratchpad[1] << 8) | scratchpad[0];
  *temperature = (float)raw_temp / 16.0f;

  return 0;
}

/**
 * @brief Read scratchpad memory from DS18B20 sensor
 *
 * Reads all 9 bytes from the sensor's scratchpad memory:
 * - Byte 0-1: Temperature (LSB, MSB)
 * - Byte 2: TH register (high alarm trigger)
 * - Byte 3: TL register (low alarm trigger)
 * - Byte 4: Configuration register
 * - Byte 5-7: Reserved
 * - Byte 8: CRC
 *
 * @param rom_code Pointer to 8-byte ROM code of sensor
 * @param scratchpad Pointer to 9-byte buffer to store scratchpad data
 * @retval 0 on success, -1 on error
 */
int8_t ds18b20_read_scratchpad(uint8_t *rom_code, uint8_t *scratchpad) {
  if (one_wire_reset() != HAL_OK) {
    return -1;
  }

  one_wire_write_byte(DS18B20_MATCH_ROM_CMD);
  for (int i = 0; i < DS18B20_ROM_BYTES; i++) {
    one_wire_write_byte(rom_code[i]);
  }

  one_wire_write_byte(DS18B20_READ_SCRATCHPAD_FN_CMD);

  for (int i = 0; i < DS18B20_SCRATCHPAD_BYTES; i++) {
    scratchpad[i] = one_wire_read_byte();
  }

  uint8_t crc = one_wire_calculate_crc8(scratchpad, 8);
  if (crc != scratchpad[8]) {
    return -1;
  }

  return 0;
}

/**
 * @brief Write scratchpad memory to DS18B20 sensor
 *
 * Writes 3 bytes to the sensor's scratchpad memory:
 * - Byte 0: TH register (high alarm trigger) in °C
 * - Byte 1: TL register (low alarm trigger) in °C
 * - Byte 2: Configuration register (resolution settings)
 *
 * Configuration register format:
 * - Bit 7: Reserved (0)
 * - Bit 6-5: Resolution (00=9-bit, 01=10-bit, 10=11-bit, 11=12-bit)
 * - Bit 4-0: Reserved (1)
 *
 * Common configuration values:
 * - 0x1F: 9-bit resolution (93.75ms conversion)
 * - 0x3F: 10-bit resolution (187.5ms conversion)
 * - 0x5F: 11-bit resolution (375ms conversion)
 * - 0x7F: 12-bit resolution (750ms conversion, default)
 *
 * @param rom_code Pointer to 8-byte ROM code of sensor
 * @param th High temperature alarm trigger (-55 to +125°C)
 * @param tl Low temperature alarm trigger (-55 to +125°C)
 * @param config Configuration register value
 * @retval 0 on success, -1 on error
 *
 * @note Changes are written to scratchpad RAM (volatile)
 * @note Use ds18b20_copy_scratchpad() to save to EEPROM for permanent storage
 * @note After power cycle, values revert to EEPROM contents unless saved
 */
int8_t ds18b20_write_scratchpad(uint8_t *rom_code, int8_t th, int8_t tl,
                                uint8_t config) {
  if (one_wire_reset() != HAL_OK) {
    return -1;
  }

  one_wire_write_byte(DS18B20_MATCH_ROM_CMD);
  for (int i = 0; i < DS18B20_ROM_BYTES; i++) {
    one_wire_write_byte(rom_code[i]);
  }

  one_wire_write_byte(DS18B20_WRITE_SCRATCHPAD_FN_CMD);

  one_wire_write_byte((uint8_t)th);
  one_wire_write_byte((uint8_t)tl);
  one_wire_write_byte(config);

  return 0;
}

/**
 * @brief Copy scratchpad to EEPROM (permanent storage)
 *
 * Copies the scratchpad contents (TH, TL, Config) to non-volatile EEPROM.
 * This makes the settings permanent - they will be restored after power cycle.
 *
 * @param rom_code Pointer to 8-byte ROM code of sensor
 * @retval 0 on success, -1 on error
 *
 * @note Copy operation takes up to 10ms
 * @note Always call this after ds18b20_write_scratchpad() to make changes
 * permanent
 */
int8_t ds18b20_copy_scratchpad(uint8_t *rom_code) {
  if (one_wire_reset() != HAL_OK) {
    return -1;
  }

  one_wire_write_byte(DS18B20_MATCH_ROM_CMD);
  for (int i = 0; i < DS18B20_ROM_BYTES; i++) {
    one_wire_write_byte(rom_code[i]);
  }

  one_wire_write_byte(DS18B20_COPY_SCRATCHPAD_FN_CMD);

  return 0;
}

/**
 * @brief Recall scratchpad from EEPROM
 *
 * Restores scratchpad contents (TH, TL, Config) from EEPROM.
 * Useful to reload saved settings after they've been modified in RAM.
 *
 * @param rom_code Pointer to 8-byte ROM code of sensor
 * @retval 0 on success, -1 on error
 *
 * @note Recall happens automatically at power-up
 * @note This function is mainly useful if scratchpad was modified but not saved
 */
int8_t ds18b20_recall_eeprom(uint8_t *rom_code) {
  if (one_wire_reset() != HAL_OK) {
    return -1;
  }

  one_wire_write_byte(DS18B20_MATCH_ROM_CMD);
  for (int i = 0; i < DS18B20_ROM_BYTES; i++) {
    one_wire_write_byte(rom_code[i]);
  }

  one_wire_write_byte(DS18B20_RECALL_E_FN_CMD);

  return 0;
}
