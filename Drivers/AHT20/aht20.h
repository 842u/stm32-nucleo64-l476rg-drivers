#ifndef AHT20_H
#define AHT20_H

#include "stm32l4xx_hal.h"

/* AHT20 I2C Address */
#define AHT20_ADDRESS (0x38 << 1)

/* AHT20 Registers */
#define AHT20_REG_STATUS 0x71

/* AHT20 Commands */
#define AHT20_CMD_INIT 0xBE
#define AHT20_CMD_TRIG_MEAS 0xAC
#define AHT20_CMD_SOFT_RESET 0xBB

/* AHT20 Initialization Command Parameters */
#define AHT20_INIT_PARAM_1 0x08
#define AHT20_INIT_PARAM_2 0x00

/* AHT20 Trigger Measurement Command Parameters */
#define AHT20_TRIG_MEAS_PARAM_1 0x33
#define AHT20_TRIG_MEAS_PARAM_2 0x00

/* AHT20 Status Bits */
#define AHT20_STATUS_BUSY_BIT (1 << 7)
#define AHT20_STATUS_CAL_BIT (1 << 3)

/* AHT20 Timing Constants [ms] */
#define AHT20_POWER_ON_DELAY 40
#define AHT20_INIT_DELAY 10
#define AHT20_MEASURE_DELAY 80
#define AHT20_SOFT_RESET_DELAY 20

/* AHT20 Handle Structure */
typedef struct {
  I2C_HandleTypeDef *hi2c;
  uint16_t address;
  uint8_t status;
} AHT20_HandleTypeDef;

/* Function Prototypes */
HAL_StatusTypeDef AHT20_ReadStatus(AHT20_HandleTypeDef *haht20);
uint8_t AHT20_IsCalibrated(AHT20_HandleTypeDef *haht20);
uint8_t AHT20_IsBusy(AHT20_HandleTypeDef *haht20);
HAL_StatusTypeDef AHT20_Init(AHT20_HandleTypeDef *haht20,
                             I2C_HandleTypeDef *hi2c, uint16_t address);
HAL_StatusTypeDef AHT20_SoftReset(AHT20_HandleTypeDef *haht20);
HAL_StatusTypeDef AHT20_TriggerMeasurement(AHT20_HandleTypeDef *haht20);
HAL_StatusTypeDef AHT20_ReadData(AHT20_HandleTypeDef *haht20, uint8_t *data,
                                 uint8_t size);
HAL_StatusTypeDef AHT20_ReadTemperature(AHT20_HandleTypeDef *haht20,
                                        float *temperature);
HAL_StatusTypeDef AHT20_ReadHumidity(AHT20_HandleTypeDef *haht20,
                                     float *humidity);
HAL_StatusTypeDef AHT20_ReadAll(AHT20_HandleTypeDef *haht20, float *temperature,
                                float *humidity);
HAL_StatusTypeDef AHT20_WaitUntilReady(AHT20_HandleTypeDef *haht20,
                                       uint8_t max_retries, uint16_t delay_ms);

#endif
