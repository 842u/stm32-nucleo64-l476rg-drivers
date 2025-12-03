#include "gdm1602a.h"
#include "delay_us.h"
#include "gdm1602a_config.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static uint8_t display_control = 0;

static void lcd_write_nibble(uint8_t nibble);
static void lcd_write_byte(uint8_t data, uint8_t rs);
static void lcd_instruction(uint8_t cmd);
static void lcd_write_data(uint8_t data);
static void lcd_enable_pulse(void);
#if LCD_USE_BUSY_FLAG
static uint8_t lcd_read_byte(uint8_t rs);
static bool lcd_is_busy(void);
static void lcd_wait_while_busy(void);
#endif

static void lcd_enable_pulse(void) {
  HAL_GPIO_WritePin(LCD_E_PORT, LCD_E_PIN, GPIO_PIN_SET);
  delay_us(LCD_DELAY_ENABLE);
  HAL_GPIO_WritePin(LCD_E_PORT, LCD_E_PIN, GPIO_PIN_RESET);
  delay_us(LCD_DELAY_ENABLE);
}

#if LCD_USE_8BIT_MODE
static void lcd_write_byte_direct(uint8_t data) {
  HAL_GPIO_WritePin(LCD_DB0_PORT, LCD_DB0_PIN,
                    (data & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LCD_DB1_PORT, LCD_DB1_PIN,
                    (data & 0x02) ? GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LCD_DB2_PORT, LCD_DB2_PIN,
                    (data & 0x04) ? GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LCD_DB3_PORT, LCD_DB3_PIN,
                    (data & 0x08) ? GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LCD_DB4_PORT, LCD_DB4_PIN,
                    (data & 0x10) ? GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LCD_DB5_PORT, LCD_DB5_PIN,
                    (data & 0x20) ? GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LCD_DB6_PORT, LCD_DB6_PIN,
                    (data & 0x40) ? GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LCD_DB7_PORT, LCD_DB7_PIN,
                    (data & 0x80) ? GPIO_PIN_SET : GPIO_PIN_RESET);
  lcd_enable_pulse();
}
#else
static void lcd_write_nibble(uint8_t nibble) {
  HAL_GPIO_WritePin(LCD_DB4_PORT, LCD_DB4_PIN,
                    (nibble & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LCD_DB5_PORT, LCD_DB5_PIN,
                    (nibble & 0x02) ? GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LCD_DB6_PORT, LCD_DB6_PIN,
                    (nibble & 0x04) ? GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LCD_DB7_PORT, LCD_DB7_PIN,
                    (nibble & 0x08) ? GPIO_PIN_SET : GPIO_PIN_RESET);
  lcd_enable_pulse();
}
#endif

static void lcd_write_byte(uint8_t data, uint8_t rs) {
  HAL_GPIO_WritePin(LCD_RS_PORT, LCD_RS_PIN,
                    rs ? GPIO_PIN_SET : GPIO_PIN_RESET);
#if LCD_USE_RW_PIN
  HAL_GPIO_WritePin(LCD_RW_PORT, LCD_RW_PIN, GPIO_PIN_RESET);
#endif

#if LCD_USE_8BIT_MODE
  lcd_write_byte_direct(data);
#else
  lcd_write_nibble(data >> 4);
  lcd_write_nibble(data & 0x0F);
#endif
}

static void lcd_instruction(uint8_t instruction) {
#if LCD_USE_BUSY_FLAG
  lcd_wait_while_busy();
#endif
  lcd_write_byte(instruction, 0);

  if (instruction == GDM1602A_INS_CLEAR || instruction == GDM1602A_INS_HOME) {
    delay_us(LCD_DELAY_INS_CLEAR_HOME);
  } else {
#if !LCD_USE_BUSY_FLAG
    delay_us(LCD_DELAY_INS);
#endif
  }
}

static void lcd_write_data(uint8_t data) {
#if LCD_USE_BUSY_FLAG
  lcd_wait_while_busy();
#endif
  lcd_write_byte(data, 1);
#if !LCD_USE_BUSY_FLAG
  delay_us(LCD_DELAY_INS);
#endif
}

#if LCD_USE_BUSY_FLAG
/* Read byte from LCD (requires R/W pin) */
/**
 * ! WORK FOR 8 BIT ??
 */
static uint8_t lcd_read_byte(uint8_t rs) {
  uint8_t data = 0;

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

  GPIO_InitStruct.Pin = LCD_DB4_PIN;
  HAL_GPIO_Init(LCD_DB4_PORT, &GPIO_InitStruct);
  GPIO_InitStruct.Pin = LCD_DB5_PIN;
  HAL_GPIO_Init(LCD_DB5_PORT, &GPIO_InitStruct);
  GPIO_InitStruct.Pin = LCD_DB6_PIN;
  HAL_GPIO_Init(LCD_DB6_PORT, &GPIO_InitStruct);
  GPIO_InitStruct.Pin = LCD_DB7_PIN;
  HAL_GPIO_Init(LCD_DB7_PORT, &GPIO_InitStruct);

  // Set RS and R/W
  HAL_GPIO_WritePin(LCD_RS_PORT, LCD_RS_PIN,
                    rs ? GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LCD_RW_PORT, LCD_RW_PIN, GPIO_PIN_SET);

  // Read high nibble
  HAL_GPIO_WritePin(LCD_E_PORT, LCD_E_PIN, GPIO_PIN_SET);
  delay_us(LCD_DELAY_ENABLE);
  data = (HAL_GPIO_ReadPin(LCD_DB7_PORT, LCD_DB7_PIN) << 3) |
         (HAL_GPIO_ReadPin(LCD_DB6_PORT, LCD_DB6_PIN) << 2) |
         (HAL_GPIO_ReadPin(LCD_DB5_PORT, LCD_DB5_PIN) << 1) |
         (HAL_GPIO_ReadPin(LCD_DB4_PORT, LCD_DB4_PIN));
  HAL_GPIO_WritePin(LCD_E_PORT, LCD_E_PIN, GPIO_PIN_RESET);
  delay_us(LCD_DELAY_ENABLE);

  data <<= 4;

  // Read low nibble
  HAL_GPIO_WritePin(LCD_E_PORT, LCD_E_PIN, GPIO_PIN_SET);
  delay_us(LCD_DELAY_ENABLE);
  data |= (HAL_GPIO_ReadPin(LCD_DB7_PORT, LCD_DB7_PIN) << 3) |
          (HAL_GPIO_ReadPin(LCD_DB6_PORT, LCD_DB6_PIN) << 2) |
          (HAL_GPIO_ReadPin(LCD_DB5_PORT, LCD_DB5_PIN) << 1) |
          (HAL_GPIO_ReadPin(LCD_DB4_PORT, LCD_DB4_PIN));
  HAL_GPIO_WritePin(LCD_E_PORT, LCD_E_PIN, GPIO_PIN_RESET);
  delay_us(LCD_DELAY_ENABLE);

  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

  GPIO_InitStruct.Pin = LCD_DB4_PIN;
  HAL_GPIO_Init(LCD_DB4_PORT, &GPIO_InitStruct);
  GPIO_InitStruct.Pin = LCD_DB5_PIN;
  HAL_GPIO_Init(LCD_DB5_PORT, &GPIO_InitStruct);
  GPIO_InitStruct.Pin = LCD_DB6_PIN;
  HAL_GPIO_Init(LCD_DB6_PORT, &GPIO_InitStruct);
  GPIO_InitStruct.Pin = LCD_DB7_PIN;
  HAL_GPIO_Init(LCD_DB7_PORT, &GPIO_InitStruct);

  return data;
}

static bool lcd_is_busy(void) {
  uint8_t status = lcd_read_byte(0);
  return (status & GDM1602A_BUSY_FLAG) != 0;
}

static void lcd_wait_while_busy(void) {
  uint32_t timeout = 10000;
  while (lcd_is_busy() && timeout--) {
    delay_us(1);
  }
}
#endif

void gdm1602a_init(void) {
  // Initialization sequence from hd44780u datasheet

  delay_us(LCD_DELAY_INIT_1);

#if LCD_USE_8BIT_MODE
  lcd_write_byte_direct(0x30);
  delay_us(LCD_DELAY_INIT_2);

  lcd_write_byte_direct(0x30);
  delay_us(LCD_DELAY_INIT_3);

  lcd_write_byte_direct(0x30);
  delay_us(LCD_DELAY_INIT_3);

  // Function set: 8-bit mode, 2 lines, 5x8 font
  lcd_instruction(GDM1602A_INS_FUNCTION_SET | GDM1602A_FS_8BIT |
                  GDM1602A_FS_2LINE | GDM1602A_FS_5x8FONT);
#else
  lcd_write_nibble(0x03);
  delay_us(LCD_DELAY_INIT_2);

  lcd_write_nibble(0x03);
  delay_us(LCD_DELAY_INIT_3);

  lcd_write_nibble(0x03);
  delay_us(LCD_DELAY_INIT_3);

  lcd_write_nibble(0x02);
  delay_us(LCD_DELAY_INIT_3);

  // Function set: 4-bit mode, 2 lines, 5x8 font
  lcd_instruction(GDM1602A_INS_FUNCTION_SET | GDM1602A_FS_4BIT |
                  GDM1602A_FS_2LINE | GDM1602A_FS_5x8FONT);
#endif

  // Display control: display off, cursor off, blink off
  lcd_instruction(GDM1602A_INS_DISPLAY_CTRL | GDM1602A_DC_DISPLAY_OFF |
                  GDM1602A_DC_CURSOR_OFF | GDM1602A_DC_BLINK_OFF);

  gdm1602a_clear();

  // Entry mode: increment cursor, no shift
  lcd_instruction(GDM1602A_INS_ENTRY_MODE | GDM1602A_EM_INCREMENT |
                  GDM1602A_EM_SHIFT_OFF);

  display_control =
      GDM1602A_DC_DISPLAY_ON | GDM1602A_DC_CURSOR_OFF | GDM1602A_DC_BLINK_OFF;

  // Display control: display on, cursor off, blink off
  lcd_instruction(GDM1602A_INS_DISPLAY_CTRL | GDM1602A_DC_DISPLAY_ON |
                  GDM1602A_DC_CURSOR_OFF | GDM1602A_DC_BLINK_OFF);
}

void gdm1602a_clear(void) { lcd_instruction(GDM1602A_INS_CLEAR); }

void gdm1602a_home(void) { lcd_instruction(GDM1602A_INS_HOME); }

void gdm1602a_set_cursor(uint8_t row, uint8_t col) {
  uint8_t address;

  if (row >= GDM1602A_ROWS) {
    row = GDM1602A_ROWS - 1;
  }

  if (col >= GDM1602A_COLS) {
    col = GDM1602A_COLS - 1;
  }

  address =
      (row == 0) ? (GDM1602A_LINE1_START + col) : (GDM1602A_LINE2_START + col);

  lcd_instruction(GDM1602A_INS_SET_DDRAM_ADDR | address);
}

void gdm1602a_putchar(char c) { lcd_write_data((uint8_t)c); }

void gdm1602a_puts(const char *str) {
  while (*str) {
    gdm1602a_putchar(*str++);
  }
}

/* Printf style function */
void gdm1602a_printf(uint8_t row, uint8_t col, const char *format, ...) {
  char buffer[GDM1602A_COLS + 1];
  va_list args;

  va_start(args, format);
  vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);

  gdm1602a_set_cursor(row, col);
  gdm1602a_puts(buffer);
}

/* Display control functions */
void gdm1602a_display_on(void) {
  /**
   * Scenario:
   *
   * Display, cursor, and blink are all ON
   * display_control = 0b00000111
   *
   * If we use display_control &= GDM1602A_DC_DISPLAY_OFF
   * display_control &= 0b00000000
   *
   * Will result in all bits cleared
   * display_control = 0b00000000
   *
   * So if we use display_control &= ~GDM1602A_DC_DISPLAY_ON
   * ~GDM1602A_DC_DISPLAY_ON = 0b11111011
   *
   * Will result in only bit 2 cleared
   * display_control = 0b00000000
   */
  display_control |= GDM1602A_DC_DISPLAY_ON;
  lcd_instruction(GDM1602A_INS_DISPLAY_CTRL | display_control);
}

void gdm1602a_display_off(void) {
  display_control &= ~GDM1602A_DC_DISPLAY_ON;
  lcd_instruction(GDM1602A_INS_DISPLAY_CTRL | display_control);
}

void gdm1602a_cursor_on(void) {
  display_control |= GDM1602A_DC_CURSOR_ON;
  lcd_instruction(GDM1602A_INS_DISPLAY_CTRL | display_control);
}

void gdm1602a_cursor_off(void) {
  display_control &= ~GDM1602A_DC_CURSOR_ON;
  lcd_instruction(GDM1602A_INS_DISPLAY_CTRL | display_control);
}

void gdm1602a_blink_on(void) {
  display_control |= GDM1602A_DC_BLINK_ON;
  lcd_instruction(GDM1602A_INS_DISPLAY_CTRL | display_control);
}

void gdm1602a_blink_off(void) {
  display_control &= ~GDM1602A_DC_BLINK_ON;
  lcd_instruction(GDM1602A_INS_DISPLAY_CTRL | display_control);
}

/* Shift functions */
void gdm1602a_shift_cursor_left(void) {
  lcd_instruction(GDM1602A_INS_CURSOR_SHIFT | GDM1602A_CS_CURSOR_L);
}

void gdm1602a_shift_cursor_right(void) {
  lcd_instruction(GDM1602A_INS_CURSOR_SHIFT | GDM1602A_CS_CURSOR_R);
}

void gdm1602a_shift_display_left(void) {
  lcd_instruction(GDM1602A_INS_CURSOR_SHIFT | GDM1602A_CS_DISPLAY_L);
}

void gdm1602a_shift_display_right(void) {
  lcd_instruction(GDM1602A_INS_CURSOR_SHIFT | GDM1602A_CS_DISPLAY_R);
}

/* Create custom character (8 characters limit) */
void gdm1602a_create_char(uint8_t location, uint8_t charmap[8]) {
  location &= 0x07;
  lcd_instruction(GDM1602A_INS_SET_CGRAM_ADDR | (location << 3));

  for (uint8_t i = 0; i < 8; i++) {
    lcd_write_data(charmap[i]);
  }

  // Return to DDRAM
  lcd_instruction(GDM1602A_INS_SET_DDRAM_ADDR);
}
