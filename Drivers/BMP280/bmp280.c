#include "bmp280.h"

static HAL_StatusTypeDef BMP280_ReadCalibration(BMP280_HandleTypeDef *hbmp280) {
  uint8_t calibration_data[BMP280_TRIMM_PARAM_REGISTERS_COUNT];
  HAL_StatusTypeDef status;

  status =
      HAL_I2C_Mem_Read(hbmp280->hi2c, hbmp280->address, BMP280_REG_TRIMM_PARAM,
                       I2C_MEMADD_SIZE_8BIT, calibration_data,
                       BMP280_TRIMM_PARAM_REGISTERS_COUNT, 1000);

  if (status != HAL_OK) {
    return status;
  }

  hbmp280->dig_T1 = (calibration_data[1] << 8) | calibration_data[0];
  hbmp280->dig_T2 = (calibration_data[3] << 8) | calibration_data[2];
  hbmp280->dig_T3 = (calibration_data[5] << 8) | calibration_data[4];
  hbmp280->dig_P1 = (calibration_data[7] << 8) | calibration_data[6];
  hbmp280->dig_P2 = (calibration_data[9] << 8) | calibration_data[8];
  hbmp280->dig_P3 = (calibration_data[11] << 8) | calibration_data[10];
  hbmp280->dig_P4 = (calibration_data[13] << 8) | calibration_data[12];
  hbmp280->dig_P5 = (calibration_data[15] << 8) | calibration_data[14];
  hbmp280->dig_P6 = (calibration_data[17] << 8) | calibration_data[16];
  hbmp280->dig_P7 = (calibration_data[19] << 8) | calibration_data[18];
  hbmp280->dig_P8 = (calibration_data[21] << 8) | calibration_data[20];
  hbmp280->dig_P9 = (calibration_data[23] << 8) | calibration_data[22];

  return HAL_OK;
}

HAL_StatusTypeDef BMP280_Init(BMP280_HandleTypeDef *hbmp280,
                              I2C_HandleTypeDef *hi2c, uint16_t address) {
  uint8_t chip_id;
  HAL_StatusTypeDef status;

  hbmp280->hi2c = hi2c;
  hbmp280->address = address;

  status = HAL_I2C_Mem_Read(hbmp280->hi2c, hbmp280->address, BMP280_REG_CHIP_ID,
                            I2C_MEMADD_SIZE_8BIT, &chip_id, 1, 1000);
  if (status != HAL_OK) {
    return status;
  }

  if (chip_id != BMP280_CHIP_ID) {
    return HAL_ERROR;
  }

  status = BMP280_ReadCalibration(hbmp280);
  if (status != HAL_OK) {
    return status;
  }

  return HAL_OK;
}

HAL_StatusTypeDef BMP280_Configure(BMP280_HandleTypeDef *hbmp280,
                                   uint8_t standby_time, uint8_t filter,
                                   uint8_t temp_oversamp,
                                   uint8_t press_oversamp, uint8_t mode) {
  uint8_t config_reg_value;
  uint8_t ctrl_meas_reg_value;
  HAL_StatusTypeDef status;

  config_reg_value = (standby_time << 5) | (filter << 2);
  status = HAL_I2C_Mem_Write(hbmp280->hi2c, hbmp280->address, BMP280_REG_CONFIG,
                             I2C_MEMADD_SIZE_8BIT, &config_reg_value, 1, 1000);
  if (status != HAL_OK) {
    return status;
  }

  ctrl_meas_reg_value = (temp_oversamp << 5) | (press_oversamp << 2) | mode;
  status =
      HAL_I2C_Mem_Write(hbmp280->hi2c, hbmp280->address, BMP280_REG_CTRL_MEAS,
                        I2C_MEMADD_SIZE_8BIT, &ctrl_meas_reg_value, 1, 1000);

  return status;
}

HAL_StatusTypeDef BMP280_ReadTemperature(BMP280_HandleTypeDef *hbmp280,
                                         float *temperature) {
  uint8_t data[3];
  int32_t adc_T;
  int32_t var1, var2;
  HAL_StatusTypeDef status;

  status =
      HAL_I2C_Mem_Read(hbmp280->hi2c, hbmp280->address, BMP280_REG_TEMP_MSB,
                       I2C_MEMADD_SIZE_8BIT, data, 3, 1000);
  if (status != HAL_OK) {
    return status;
  }

  adc_T = ((int32_t)data[0] << 12) | ((int32_t)data[1] << 4) |
          ((int32_t)data[2] >> 4);

  /* Compensation formula from BMP280 datasheet */
  var1 = ((((adc_T >> 3) - ((int32_t)hbmp280->dig_T1 << 1))) *
          ((int32_t)hbmp280->dig_T2)) >>
         11;
  var2 = (((((adc_T >> 4) - ((int32_t)hbmp280->dig_T1)) *
            ((adc_T >> 4) - ((int32_t)hbmp280->dig_T1))) >>
           12) *
          ((int32_t)hbmp280->dig_T3)) >>
         14;

  hbmp280->t_fine = var1 + var2;
  *temperature = ((hbmp280->t_fine * 5 + 128) >> 8) / 100.0f;

  return HAL_OK;
}

HAL_StatusTypeDef BMP280_ReadPressure(BMP280_HandleTypeDef *hbmp280,
                                      float *pressure) {
  uint8_t data[3];
  int32_t adc_P;
  int64_t var1, var2, p;
  HAL_StatusTypeDef status;

  status =
      HAL_I2C_Mem_Read(hbmp280->hi2c, hbmp280->address, BMP280_REG_PRESS_MSB,
                       I2C_MEMADD_SIZE_8BIT, data, 3, 1000);
  if (status != HAL_OK) {
    return status;
  }

  adc_P = ((int32_t)data[0] << 12) | ((int32_t)data[1] << 4) |
          ((int32_t)data[2] >> 4);

  /* Compensation formula from BMP280 datasheet */
  var1 = ((int64_t)hbmp280->t_fine) - 128000;
  var2 = var1 * var1 * (int64_t)hbmp280->dig_P6;
  var2 = var2 + ((var1 * (int64_t)hbmp280->dig_P5) << 17);
  var2 = var2 + (((int64_t)hbmp280->dig_P4) << 35);
  var1 = ((var1 * var1 * (int64_t)hbmp280->dig_P3) >> 8) +
         ((var1 * (int64_t)hbmp280->dig_P2) << 12);
  var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)hbmp280->dig_P1) >> 33;

  if (var1 == 0) {
    return HAL_ERROR;
  }

  p = 1048576 - adc_P;
  p = (((p << 31) - var2) * 3125) / var1;
  var1 = (((int64_t)hbmp280->dig_P9) * (p >> 13) * (p >> 13)) >> 25;
  var2 = (((int64_t)hbmp280->dig_P8) * p) >> 19;
  p = ((p + var1 + var2) >> 8) + (((int64_t)hbmp280->dig_P7) << 4);

  *pressure = (float)p / 256.0f;

  return HAL_OK;
}

HAL_StatusTypeDef BMP280_ReadAll(BMP280_HandleTypeDef *hbmp280,
                                 float *temperature, float *pressure) {
  HAL_StatusTypeDef status;

  /* Read temperature first (needed for pressure compensation) */
  status = BMP280_ReadTemperature(hbmp280, temperature);
  if (status != HAL_OK) {
    return status;
  }

  status = BMP280_ReadPressure(hbmp280, pressure);

  return status;
}
