#include "one_wire.h"

/**
 * @brief Initialize 1-Wire bus and detect presence of slave devices
 * @retval HAL_OK if device presence detected
 * @retval HAL_ERROR if no device present on the bus
 * @note Reset pulse: Pull low for 480μs, release, wait 70μs, sample presence
 * pulse
 * @note Total reset sequence takes minimum 960μs (480μs + 480μs)
 */
HAL_StatusTypeDef one_wire_reset(void) {
  uint8_t pin_state;

  HAL_GPIO_WritePin(OW_DQ_GPIO_Port, OW_DQ_Pin, GPIO_PIN_RESET);

  // Keep the low state for a required minimum 480us for the reset pulse
  delay_us(480);

  HAL_GPIO_WritePin(OW_DQ_GPIO_Port, OW_DQ_Pin, GPIO_PIN_SET);

  // Wait for a slave presence pulse
  delay_us(70);
  pin_state = HAL_GPIO_ReadPin(OW_DQ_GPIO_Port, OW_DQ_Pin);

  // Satisfy the minimum time of a master RX time slot
  delay_us(410);

  return (pin_state == 0) ? HAL_OK : HAL_ERROR;
}

/**
 * @brief Write a single bit to the 1-Wire bus
 * @param value Bit value to write
 * @note Write-1: Pull low for 1μs, release, wait 60μs total
 * @note Write-0: Pull low for 60μs, release
 * @note 1μs recovery time between time slots
 */
void one_wire_write_bit(uint8_t value) {
  if (value) {
    HAL_GPIO_WritePin(OW_DQ_GPIO_Port, OW_DQ_Pin, GPIO_PIN_RESET);

    // Keep the low state for a time between 1-15us
    delay_us(1);

    HAL_GPIO_WritePin(OW_DQ_GPIO_Port, OW_DQ_Pin, GPIO_PIN_SET);

    // Satisfy the minimum time of a write-one time slot
    delay_us(60);
  } else {
    HAL_GPIO_WritePin(OW_DQ_GPIO_Port, OW_DQ_Pin, GPIO_PIN_RESET);

    // Keep the low state for a required minimum time for the write-zero
    delay_us(60);

    HAL_GPIO_WritePin(OW_DQ_GPIO_Port, OW_DQ_Pin, GPIO_PIN_SET);
  }

  // Minimum recovery time after write time slot
  delay_us(1);
}

/**
 * @brief Read a single bit from the 1-Wire bus
 * @retval Bit value read from the bus
 * @note Read time slot: Pull low for 1μs, release, sample at ~14μs, wait 60μs
 * total
 * @note Master sampling window is 15μs from start, sampling occurs as close to
 * 15μs as possible
 */
uint8_t one_wire_read_bit(void) {
  uint32_t pin_state;

  HAL_GPIO_WritePin(OW_DQ_GPIO_Port, OW_DQ_Pin, GPIO_PIN_RESET);

  // Keep the low state for a time between 1-15us
  delay_us(1);

  HAL_GPIO_WritePin(OW_DQ_GPIO_Port, OW_DQ_Pin, GPIO_PIN_SET);

  /**
   * Master sampling windows ends in 15us and should occure as close to 15us as
    possible.
   */
  delay_us(13);

  pin_state = HAL_GPIO_ReadPin(OW_DQ_GPIO_Port, OW_DQ_Pin);

  // Satisfy the minimum time of a read-data time slot
  delay_us(60);

  return pin_state;
}

/**
 * @brief Write a byte to the 1-Wire bus
 * @param byte Byte value to write to the bus
 * @note Writes 8 bits sequentially, LSB to MSB as per 1-Wire protocol
 */
void one_wire_write_byte(uint8_t byte) {
  // Sending from LSB to MSB
  for (uint8_t i = 0; i < 8; i++) {
    // e.g. byte & 0x01 -> 1010 0101 & 0000 0001 = 0000 0001 = 1
    one_wire_write_bit(byte & 0x01);
    byte >>= 1;
  }
}

/**
 * @brief Read a byte from the 1-Wire bus
 * @retval Byte value read from the bus (LSB first)
 * @note Reads 8 bits sequentially, LSB to MSB as per 1-Wire protocol
 */
uint8_t one_wire_read_byte(void) {
  // Reading from LSB to MSB
  uint8_t value = 0;

  for (uint8_t i = 0; i < 8; i++) {
    value >>= 1;
    if (one_wire_read_bit())
      value |= 0x80;
  }

  return value;
}

/**
 * @brief Calculate 8-bit CRC for 1-Wire devices using Dallas/Maxim polynomial
 * @param data Pointer to data buffer to calculate CRC over
 * @param length Number of bytes to include in CRC calculation
 * @retval Calculated 8-bit CRC value
 * @note Uses polynomial: x^8 + x^5 + x^4 + 1 (0x8C reversed)
 * @note Processing is done LSB first as per 1-Wire protocol
 */
uint8_t one_wire_calculate_crc8(uint8_t *data, uint8_t length) {
  uint8_t crc = 0x00;

  for (uint8_t i = 0; i < length; i++) {
    // XOR with current byte
    crc ^= data[i];

    for (uint8_t j = 0; j < 8; j++) {
      // If LSB is 1
      if (crc & 0x01) {
        /**
         * x^8+x^5+x^4+1 = 0x131
         * its 8 degree polynominal so x^8 is implicit
         * x^8 is dropped so its 0x31
         * 0x31 is reversed because processing from LSB so 0x8C
         */
        crc = (crc >> 1) ^ 0x8C;
      } else {
        crc >>= 1;
      }
    }
  }

  return crc;
}

/**
 * @brief Reset search ROM state - call before starting new search sequence
 */
void one_wire_search_rom_reset(void) {
  LastDiscrepancy = 0;
  LastDeviceFlag = 0;
  LastFamilyDiscrepancy = 0;
  memset(ROM_NO, 0, sizeof ROM_NO);
}

/**
 * @brief Search for next device on 1-Wire bus
 * @param rom_code Pointer to 8-byte array to store found ROM code
 * @retval 1 if device found
 * @retval 0 if no more devices (search complete)
 * @retval -1 on error (no device present, CRC error, or bus error)
 */
int8_t one_wire_search_rom(uint8_t *rom_code) {
  uint8_t id_bit_number;
  uint8_t last_zero;
  uint8_t rom_byte_number;
  uint8_t rom_byte_mask;
  uint8_t search_direction;
  uint8_t id_bit;
  uint8_t cmp_id_bit;

  if (LastDeviceFlag) {
    LastDeviceFlag = 0;
    return 0;
  }

  if (one_wire_reset() != HAL_OK) {
    return -1;
  }

  // Send search ROM command
  one_wire_write_byte(0xF0);

  id_bit_number = 1;
  last_zero = 0;
  rom_byte_number = 0;
  rom_byte_mask = 1;

  while (rom_byte_number < 8) {
    id_bit = one_wire_read_bit();
    cmp_id_bit = one_wire_read_bit();

    // Check for no devices or error
    if (id_bit && cmp_id_bit) {
      return -1;
    }

    // Determine search direction
    if (id_bit != cmp_id_bit) {
      // All devices have the same bit value
      search_direction = id_bit;
    } else {
      // Discrepancy: both 0 and 1 present on the bus
      if (id_bit_number < LastDiscrepancy) {
        // Follow previous path
        search_direction = (ROM_NO[rom_byte_number] & rom_byte_mask) ? 1 : 0;
      } else if (id_bit_number == LastDiscrepancy) {
        // At last discrepancy, go in the 1 direction
        search_direction = 1;
      } else {
        // Past last discrepancy, go in 0 direction
        search_direction = 0;
      }

      // If went in 0 direction, record this position
      if (search_direction == 0) {
        last_zero = id_bit_number;

        // Check for Last discrepancy in family code
        if (last_zero < 9) {
          LastFamilyDiscrepancy = last_zero;
        }
      }
    }

    // Save the bit in ROM_NO
    if (search_direction) {
      ROM_NO[rom_byte_number] |= rom_byte_mask;
    } else {
      ROM_NO[rom_byte_number] &= ~rom_byte_mask;
    }

    // Write the chosen bit direction
    one_wire_write_bit(search_direction);

    id_bit_number++;
    rom_byte_mask <<= 1;

    // Move to next byte if needed
    if (rom_byte_mask == 0) {
      rom_byte_number++;
      rom_byte_mask = 1;
    }
  }

  uint8_t crc = one_wire_calculate_crc8(ROM_NO, 7);
  if (crc != ROM_NO[7]) {
    return -1;
  }

  LastDiscrepancy = last_zero;

  if (LastDiscrepancy == 0) {
    LastDeviceFlag = 1;
  }

  memcpy(rom_code, ROM_NO, 8);

  return 1;
}

/**
 * @brief Get total number of devices on the bus
 * @retval Number of devices found, or -1 on error
 */
int8_t one_wire_get_device_count(void) {
  uint8_t temp_rom[8];
  int8_t count = 0;
  int8_t result;

  one_wire_search_rom_reset();

  while ((result = one_wire_search_rom(temp_rom)) == 1) {
    count++;
    // Prevent overflow
    if (count >= 127)
      break;
  }

  if (result < 0) {
    return -1;
  }

  return count;
}
