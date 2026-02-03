#include "one_wire.h"
#include "delay_us.h"
#include "main.h"

HAL_StatusTypeDef one_wire_reset(void) {
  int pin_state;

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

void one_wire_write_bit(int value) {
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

int one_wire_read_bit(void) {
  int pin_state;

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

void one_wire_write_byte(uint8_t byte) {
  // Sending from LSB to MSB
  for (int i = 0; i < 8; i++) {
    // e.g. byte & 0x01 -> 1010 0101 & 0000 0001 = 0000 0001 = 1
    one_wire_write_bit(byte & 0x01);
    byte >>= 1;
  }
}

uint8_t one_wire_read_byte(void) {
  // Reading from LSB to MSB
  uint8_t value = 0;

  for (int i = 0; i < 8; i++) {
    value >>= 1;
    if (one_wire_read_bit())
      value |= 0x80;
  }

  return value;
}
