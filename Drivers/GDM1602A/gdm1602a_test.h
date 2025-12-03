#ifndef GDM1602A_TEST_H
#define GDM1602A_TEST_H

#include <stdbool.h>
#include <stdint.h>


/* Test configuration */
#define TEST_DELAY_SHORT 1000  // 1 second
#define TEST_DELAY_MEDIUM 2000 // 2 seconds
#define TEST_DELAY_LONG 3000   // 3 seconds

/* Test function prototypes */
void gdm1602a_test_all(void);
void gdm1602a_test_basic_output(void);
void gdm1602a_test_cursor_control(void);
void gdm1602a_test_display_control(void);
void gdm1602a_test_cursor_movement(void);
void gdm1602a_test_display_shift(void);
void gdm1602a_test_custom_characters(void);
void gdm1602a_test_printf_function(void);
void gdm1602a_test_multiline_text(void);
void gdm1602a_test_scrolling_text(void);

/* Helper functions */
void gdm1602a_test_run_single(uint8_t test_number);
const char *gdm1602a_test_get_name(uint8_t test_number);
uint8_t gdm1602a_test_get_count(void);

#endif
