#include "aht20.h"

HAL_StatusTypeDef AHT20_ReadStatus(AHT20_HandleTypeDef *haht20) {
  HAL_StatusTypeDef hal_status;

  hal_status = HAL_I2C_Mem_Read(haht20->hi2c, haht20->address, AHT20_REG_STATUS,
                                I2C_MEMADD_SIZE_8BIT, &haht20->status, 1, 1000);

  return hal_status;
}

uint8_t AHT20_IsCalibrated(AHT20_HandleTypeDef *haht20) {
  return (haht20->status & AHT20_STATUS_CAL_BIT) ? 1 : 0;
}

uint8_t AHT20_IsBusy(AHT20_HandleTypeDef *haht20) {
  return (haht20->status & AHT20_STATUS_BUSY_BIT) ? 1 : 0;
}

HAL_StatusTypeDef AHT20_SoftReset(AHT20_HandleTypeDef *haht20) {
  HAL_StatusTypeDef hal_status;
  uint8_t cmd = AHT20_CMD_SOFT_RESET;

  hal_status =
      HAL_I2C_Master_Transmit(haht20->hi2c, haht20->address, &cmd, 1, 1000);

  if (hal_status != HAL_OK) {
    return hal_status;
  }

  HAL_Delay(AHT20_SOFT_RESET_DELAY);

  return HAL_OK;
}

/**
 * @brief Wait until sensor is ready (not busy)
 * @param haht20 Pointer to AHT20 handle structure
 * @param max_retries Maximum number of retries (typically 10)
 * @param delay_ms Delay between retries in milliseconds (typically 10)
 * @return HAL_OK if sensor becomes ready, HAL_TIMEOUT if max retries exceeded,
 *         or other HAL error code if status read fails
 * @note This is a helper function to avoid code duplication in measurement
 * functions
 */
HAL_StatusTypeDef AHT20_WaitUntilReady(AHT20_HandleTypeDef *haht20,
                                       uint8_t max_retries, uint16_t delay_ms) {
  HAL_StatusTypeDef hal_status;
  uint8_t retry_count = 0;

  while (retry_count < max_retries) {
    hal_status = AHT20_ReadStatus(haht20);

    if (hal_status != HAL_OK) {
      return hal_status;
    }

    if (!AHT20_IsBusy(haht20)) {
      return HAL_OK;
    }

    HAL_Delay(delay_ms);
    retry_count++;
  }

  return HAL_TIMEOUT;
}

/**
 * @brief Initialize AHT20 sensor
 * @param haht20 Pointer to AHT20 handle structure
 * @param hi2c Pointer to I2C handle
 * @param address I2C address of the sensor (use AHT20_ADDRESS)
 * @return HAL status
 */
HAL_StatusTypeDef AHT20_Init(AHT20_HandleTypeDef *haht20,
                             I2C_HandleTypeDef *hi2c, uint16_t address) {
  HAL_StatusTypeDef hal_status;
  uint8_t init_cmd[3];

  haht20->hi2c = hi2c;
  haht20->address = address;

  HAL_Delay(AHT20_POWER_ON_DELAY);

  hal_status = AHT20_ReadStatus(haht20);
  if (hal_status != HAL_OK) {
    return hal_status;
  }

  if (!AHT20_IsCalibrated(haht20)) {
    init_cmd[0] = AHT20_CMD_INIT;
    init_cmd[1] = AHT20_INIT_PARAM_1;
    init_cmd[2] = AHT20_INIT_PARAM_2;

    hal_status = HAL_I2C_Master_Transmit(haht20->hi2c, haht20->address,
                                         init_cmd, 3, 1000);

    if (hal_status != HAL_OK) {
      return hal_status;
    }

    HAL_Delay(AHT20_INIT_DELAY);

    hal_status = AHT20_ReadStatus(haht20);
    if (hal_status != HAL_OK) {
      return hal_status;
    }

    if (!AHT20_IsCalibrated(haht20)) {
      return HAL_ERROR;
    }
  }

  return HAL_OK;
}

HAL_StatusTypeDef AHT20_TriggerMeasurement(AHT20_HandleTypeDef *haht20) {
  HAL_StatusTypeDef hal_status;
  uint8_t cmd[3];

  cmd[0] = AHT20_CMD_TRIG_MEAS;
  cmd[1] = AHT20_TRIG_MEAS_PARAM_1;
  cmd[2] = AHT20_TRIG_MEAS_PARAM_2;

  hal_status =
      HAL_I2C_Master_Transmit(haht20->hi2c, haht20->address, cmd, 3, 1000);

  return hal_status;
}

/**
 * @brief Read raw data from AHT20 sensor
 * @param haht20 Pointer to AHT20 handle structure
 * @param data Buffer to store the data
 * @param size Number of bytes to read (typically 6 or 7 with CRC)
 * @return HAL status
 */
HAL_StatusTypeDef AHT20_ReadData(AHT20_HandleTypeDef *haht20, uint8_t *data,
                                 uint8_t size) {
  HAL_StatusTypeDef hal_status;

  hal_status =
      HAL_I2C_Master_Receive(haht20->hi2c, haht20->address, data, size, 1000);

  return hal_status;
}

/**
 * @brief Read temperature from AHT20 sensor
 * @param haht20 Pointer to AHT20 handle structure
 * @param temperature Pointer to store temperature value in degrees Celsius
 * @return HAL status
 */
HAL_StatusTypeDef AHT20_ReadTemperature(AHT20_HandleTypeDef *haht20,
                                        float *temperature) {
  HAL_StatusTypeDef hal_status;
  uint8_t data[6];
  uint32_t temperature_raw;

  hal_status = AHT20_TriggerMeasurement(haht20);
  if (hal_status != HAL_OK) {
    return hal_status;
  }

  HAL_Delay(AHT20_MEASURE_DELAY);

  hal_status = AHT20_WaitUntilReady(haht20, 5, 5);
  if (hal_status != HAL_OK) {
    return hal_status;
  }

  hal_status = AHT20_ReadData(haht20, data, 6);
  if (hal_status != HAL_OK) {
    return hal_status;
  }

  // Extract 20-bit temperature value from bytes 3, 4, and 5
  // Byte 3: bits[3:0] are temperature MSB
  // Byte 4: temperature middle byte
  // Byte 5: temperature LSB
  temperature_raw = ((uint32_t)(data[3] & 0x0F) << 16) |
                    ((uint32_t)data[4] << 8) | ((uint32_t)data[5]);

  // Convert to Celsius using formula from datasheet: T = (S_T / 2^20) * 200 -
  // 50
  *temperature = ((float)temperature_raw / 1048576.0f) * 200.0f - 50.0f;

  return HAL_OK;
}

/**
 * @brief Read humidity from AHT20 sensor
 * @param haht20 Pointer to AHT20 handle structure
 * @param humidity Pointer to store humidity value in %RH
 * @return HAL status
 */
HAL_StatusTypeDef AHT20_ReadHumidity(AHT20_HandleTypeDef *haht20,
                                     float *humidity) {
  HAL_StatusTypeDef hal_status;
  uint8_t data[6];
  uint32_t humidity_raw;

  hal_status = AHT20_TriggerMeasurement(haht20);
  if (hal_status != HAL_OK) {
    return hal_status;
  }

  HAL_Delay(AHT20_MEASURE_DELAY);

  hal_status = AHT20_WaitUntilReady(haht20, 5, 5);
  if (hal_status != HAL_OK) {
    return hal_status;
  }

  hal_status = AHT20_ReadData(haht20, data, 6);
  if (hal_status != HAL_OK) {
    return hal_status;
  }

  // Extract 20-bit humidity value from bytes 1, 2, and 3
  // Byte 1: humidity MSB
  // Byte 2: humidity middle byte
  // Byte 3: bits[7:4] are humidity LSB
  humidity_raw = ((uint32_t)data[1] << 12) | ((uint32_t)data[2] << 4) |
                 ((uint32_t)data[3] >> 4);

  // Convert to %RH using formula from datasheet: RH = (S_RH / 2^20) * 100%
  *humidity = ((float)humidity_raw / 1048576.0f) * 100.0f;

  return HAL_OK;
}

/**
 * @brief Read both temperature and humidity from AHT20 sensor
 * @param haht20 Pointer to AHT20 handle structure
 * @param temperature Pointer to store temperature value in degrees Celsius
 * @param humidity Pointer to store humidity value in %RH
 * @return HAL status
 * @note This function is more efficient than calling ReadTemperature and
 * ReadHumidity separately as it only triggers one measurement for both values
 */
HAL_StatusTypeDef AHT20_ReadAll(AHT20_HandleTypeDef *haht20, float *temperature,
                                float *humidity) {
  HAL_StatusTypeDef hal_status;
  uint8_t data[6];
  uint32_t humidity_raw, temperature_raw;

  hal_status = AHT20_TriggerMeasurement(haht20);
  if (hal_status != HAL_OK) {
    return hal_status;
  }

  HAL_Delay(AHT20_MEASURE_DELAY);

  hal_status = AHT20_WaitUntilReady(haht20, 5, 5);
  if (hal_status != HAL_OK) {
    return hal_status;
  }

  hal_status = AHT20_ReadData(haht20, data, 6);
  if (hal_status != HAL_OK) {
    return hal_status;
  }

  // Extract 20-bit humidity value from bytes 1, 2, and 3
  humidity_raw = ((uint32_t)data[1] << 12) | ((uint32_t)data[2] << 4) |
                 ((uint32_t)data[3] >> 4);

  // Extract 20-bit temperature value from bytes 3, 4, and 5
  temperature_raw = ((uint32_t)(data[3] & 0x0F) << 16) |
                    ((uint32_t)data[4] << 8) | ((uint32_t)data[5]);

  // Convert to Celsius and %RH
  *humidity = ((float)humidity_raw / 1048576.0f) * 100.0f;
  *temperature = ((float)temperature_raw / 1048576.0f) * 200.0f - 50.0f;

  return HAL_OK;
}
