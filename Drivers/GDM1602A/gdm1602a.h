#ifndef GDM1602A_H
#define GDM1602A_H

#include <stdbool.h>
#include <stdint.h>

/* Instructions */
#define GDM1602A_INS_CLEAR 0x01
#define GDM1602A_INS_HOME 0x02
#define GDM1602A_INS_ENTRY_MODE 0x04
#define GDM1602A_INS_DISPLAY_CTRL 0x08
#define GDM1602A_INS_CURSOR_SHIFT 0x10
#define GDM1602A_INS_FUNCTION_SET 0x20
#define GDM1602A_INS_SET_CGRAM_ADDR 0x40
#define GDM1602A_INS_SET_DDRAM_ADDR 0x80

/* Entry Mode Instruction Flag Bit Positions */
#define GDM1602A_EM_SH_BIT 0
#define GDM1602A_EM_ID_BIT 1

/* Entry Mode Instruction Flags */
#define GDM1602A_EM_SHIFT_ON (1 << GDM1602A_EM_SH_BIT)
#define GDM1602A_EM_SHIFT_OFF (0 << GDM1602A_EM_SH_BIT)
#define GDM1602A_EM_INCREMENT (1 << GDM1602A_EM_ID_BIT)
#define GDM1602A_EM_DECREMENT (0 << GDM1602A_EM_ID_BIT)

/* Display Control Instruction Flag Bit Positions */
#define GDM1602A_DC_B_BIT 0
#define GDM1602A_DC_C_BIT 1
#define GDM1602A_DC_D_BIT 2

/* Display Control Instruction Flags */
#define GDM1602A_DC_BLINK_ON (1 << GDM1602A_DC_B_BIT)
#define GDM1602A_DC_BLINK_OFF (0 << GDM1602A_DC_B_BIT)
#define GDM1602A_DC_CURSOR_ON (1 << GDM1602A_DC_C_BIT)
#define GDM1602A_DC_CURSOR_OFF (0 << GDM1602A_DC_C_BIT)
#define GDM1602A_DC_DISPLAY_ON (1 << GDM1602A_DC_D_BIT)
#define GDM1602A_DC_DISPLAY_OFF (0 << GDM1602A_DC_D_BIT)

/* Cursor Shift Instruction Flag Bit Positions */
#define GDM1602A_CS_RL_BIT 2
#define GDM1602A_CS_SC_BIT 3

/* Cursor Shift Instruction Flags */
#define GDM1602A_CS_CURSOR_L                                                   \
  ((0 << GDM1602A_CS_SC_BIT) | (0 << GDM1602A_CS_RL_BIT))
#define GDM1602A_CS_CURSOR_R                                                   \
  ((0 << GDM1602A_CS_SC_BIT) | (1 << GDM1602A_CS_RL_BIT))
#define GDM1602A_CS_DISPLAY_L                                                  \
  ((1 << GDM1602A_CS_SC_BIT) | (0 << GDM1602A_CS_RL_BIT))
#define GDM1602A_CS_DISPLAY_R                                                  \
  ((1 << GDM1602A_CS_SC_BIT) | (1 << GDM1602A_CS_RL_BIT))

/* Function Set Instruction Flag Bit Positions */
#define GDM1602A_FS_F_BIT 2
#define GDM1602A_FS_N_BIT 3
#define GDM1602A_FS_DL_BIT 4

/* Function Set Instruction Flags */
#define GDM1602A_FS_4BIT (0 << GDM1602A_FS_DL_BIT)
#define GDM1602A_FS_8BIT (1 << GDM1602A_FS_DL_BIT)
#define GDM1602A_FS_1LINE (0 << GDM1602A_FS_N_BIT)
#define GDM1602A_FS_2LINE (1 << GDM1602A_FS_N_BIT)
#define GDM1602A_FS_5x8FONT (0 << GDM1602A_FS_F_BIT)
#define GDM1602A_FS_5x11FONT (1 << GDM1602A_FS_F_BIT)

/* DDRAM Address Locations for 16x2 Display */
#define GDM1602A_LINE1_START 0x00 // First line starts at 0x00
#define GDM1602A_LINE2_START 0x40 // Second line starts at 0x40

/* Display Dimensions */
#define GDM1602A_COLS 16
#define GDM1602A_ROWS 2

void gdm1602a_init(void);
void gdm1602a_clear(void);
void gdm1602a_home(void);
void gdm1602a_set_cursor(uint8_t row, uint8_t col);
void gdm1602a_putchar(char c);
void gdm1602a_puts(const char *str);
void gdm1602a_printf(uint8_t row, uint8_t col, const char *format, ...);
void gdm1602a_display_on(void);
void gdm1602a_display_off(void);
void gdm1602a_cursor_on(void);
void gdm1602a_cursor_off(void);
void gdm1602a_blink_on(void);
void gdm1602a_blink_off(void);
void gdm1602a_shift_cursor_left(void);
void gdm1602a_shift_cursor_right(void);
void gdm1602a_shift_display_left(void);
void gdm1602a_shift_display_right(void);
void gdm1602a_create_char(uint8_t location, uint8_t charmap[8]);

#endif
