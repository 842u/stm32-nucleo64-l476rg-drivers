#include "gdm1602a_test.h"
#include "gdm1602a.h"
#include "main.h"
#include <string.h>

/* Test names for reference */
static const char *test_names[] = {
    "Basic Output",    "Cursor Control", "Display Control",
    "Cursor Movement", "Display Shift",  "Custom Chars",
    "Printf Function", "Multiline Text", "Scrolling Text"};

#define NUM_TESTS (sizeof(test_names) / sizeof(test_names[0]))

/**
 * @brief Get the number of available tests
 * @return Number of tests
 */
uint8_t gdm1602a_test_get_count(void) { return NUM_TESTS; }

/**
 * @brief Get the name of a specific test
 * @param test_number Test index (0-based)
 * @return Test name string
 */
const char *gdm1602a_test_get_name(uint8_t test_number) {
  if (test_number >= NUM_TESTS) {
    return "Invalid";
  }
  return test_names[test_number];
}

/**
 * @brief Run a single test by number
 * @param test_number Test index (0-based)
 */
void gdm1602a_test_run_single(uint8_t test_number) {
  switch (test_number) {
  case 0:
    gdm1602a_test_basic_output();
    break;
  case 1:
    gdm1602a_test_cursor_control();
    break;
  case 2:
    gdm1602a_test_display_control();
    break;
  case 3:
    gdm1602a_test_cursor_movement();
    break;
  case 4:
    gdm1602a_test_display_shift();
    break;
  case 5:
    gdm1602a_test_custom_characters();
    break;
  case 6:
    gdm1602a_test_printf_function();
    break;
  case 7:
    gdm1602a_test_multiline_text();
    break;
  case 8:
    gdm1602a_test_scrolling_text();
    break;
  default:
    gdm1602a_clear();
    gdm1602a_printf(0, 0, "Invalid test #%d", test_number);
    HAL_Delay(TEST_DELAY_MEDIUM);
    break;
  }
}

/**
 * @brief Run all tests in sequence
 */
void gdm1602a_test_all(void) {
  // Welcome message
  gdm1602a_clear();
  gdm1602a_printf(0, 0, "GDM1602A Tests");
  gdm1602a_printf(1, 0, "Starting...");
  HAL_Delay(TEST_DELAY_MEDIUM);

  // Run all tests
  for (uint8_t i = 0; i < NUM_TESTS; i++) {
    gdm1602a_test_run_single(i);
    HAL_Delay(TEST_DELAY_LONG);
  }

  // All tests complete
  gdm1602a_clear();
  gdm1602a_printf(0, 0, "All tests");
  gdm1602a_printf(1, 0, "completed!");
  HAL_Delay(TEST_DELAY_LONG);
}

/**
 * @brief Test 1: Basic text output
 */
void gdm1602a_test_basic_output(void) {
  gdm1602a_clear();
  gdm1602a_puts("Test 1: Output");
  HAL_Delay(TEST_DELAY_SHORT);

  gdm1602a_set_cursor(1, 0);
  gdm1602a_puts("Hello, World!");
  HAL_Delay(TEST_DELAY_MEDIUM);

  // Test character by character
  gdm1602a_clear();
  gdm1602a_set_cursor(0, 0);
  const char *msg = "Char by char...";
  for (int i = 0; msg[i] != '\0'; i++) {
    gdm1602a_putchar(msg[i]);
    HAL_Delay(100);
  }
  HAL_Delay(TEST_DELAY_SHORT);
}

/**
 * @brief Test 2: Cursor control (on/off/blink)
 */
void gdm1602a_test_cursor_control(void) {
  gdm1602a_clear();
  gdm1602a_printf(0, 0, "Test 2: Cursor");
  HAL_Delay(TEST_DELAY_SHORT);

  // Cursor ON
  gdm1602a_clear();
  gdm1602a_set_cursor(0, 0);
  gdm1602a_puts("Cursor ON");
  gdm1602a_set_cursor(1, 0);
  gdm1602a_cursor_on();
  HAL_Delay(TEST_DELAY_MEDIUM);

  // Cursor OFF
  gdm1602a_clear();
  gdm1602a_puts("Cursor OFF");
  gdm1602a_cursor_off();
  HAL_Delay(TEST_DELAY_MEDIUM);

  // Blinking cursor
  gdm1602a_clear();
  gdm1602a_set_cursor(0, 0);
  gdm1602a_puts("Blink ON");
  gdm1602a_set_cursor(1, 0);
  gdm1602a_cursor_on();
  gdm1602a_blink_on();
  HAL_Delay(TEST_DELAY_LONG);

  // Blink OFF
  gdm1602a_clear();
  gdm1602a_puts("Blink OFF");
  gdm1602a_blink_off();
  HAL_Delay(TEST_DELAY_SHORT);
  gdm1602a_cursor_off();
  HAL_Delay(TEST_DELAY_MEDIUM);
}

/**
 * @brief Test 3: Display on/off
 */
void gdm1602a_test_display_control(void) {
  gdm1602a_clear();
  gdm1602a_printf(0, 0, "Test 3: Display");
  gdm1602a_printf(1, 0, "Control");
  HAL_Delay(TEST_DELAY_MEDIUM);

  // Blink display on/off
  for (int i = 0; i < 3; i++) {
    gdm1602a_display_off();
    HAL_Delay(500);
    gdm1602a_display_on();
    HAL_Delay(500);
  }

  HAL_Delay(TEST_DELAY_SHORT);
}

/**
 * @brief Test 4: Cursor movement
 */
void gdm1602a_test_cursor_movement(void) {
  gdm1602a_clear();
  gdm1602a_printf(0, 0, "Test 4: Move");
  HAL_Delay(TEST_DELAY_SHORT);

  // Show cursor
  gdm1602a_cursor_on();
  gdm1602a_set_cursor(1, 0);
  gdm1602a_puts("*             *");

  // Move cursor right
  gdm1602a_set_cursor(1, 1);
  for (int i = 0; i < 13; i++) {
    gdm1602a_shift_cursor_right();
    HAL_Delay(200);
  }

  // Move cursor left
  for (int i = 0; i < 13; i++) {
    gdm1602a_shift_cursor_left();
    HAL_Delay(200);
  }

  gdm1602a_cursor_off();
  HAL_Delay(TEST_DELAY_SHORT);
}

/**
 * @brief Test 5: Display shift (scrolling)
 */
void gdm1602a_test_display_shift(void) {
  gdm1602a_clear();
  gdm1602a_printf(0, 0, "Test 5: Shift");
  HAL_Delay(TEST_DELAY_SHORT);

  gdm1602a_clear();
  gdm1602a_printf(0, 0, "Scroll Left>>>>");
  gdm1602a_printf(1, 0, "1234567890ABCDEF");
  HAL_Delay(TEST_DELAY_SHORT);

  // Shift display left
  for (int i = 0; i < 10; i++) {
    gdm1602a_shift_display_left();
    HAL_Delay(300);
  }

  HAL_Delay(500);

  // Shift display right
  for (int i = 0; i < 10; i++) {
    gdm1602a_shift_display_right();
    HAL_Delay(300);
  }

  HAL_Delay(TEST_DELAY_SHORT);
}

/**
 * @brief Test 6: Custom characters
 */
void gdm1602a_test_custom_characters(void) {
  gdm1602a_clear();
  gdm1602a_printf(0, 0, "Test 6: Custom");
  HAL_Delay(TEST_DELAY_SHORT);

  // Define custom characters
  uint8_t heart[8] = {0b00000, 0b01010, 0b11111, 0b11111,
                      0b11111, 0b01110, 0b00100, 0b00000};

  uint8_t smiley[8] = {0b00000, 0b01010, 0b01010, 0b00000,
                       0b10001, 0b01110, 0b00000, 0b00000};

  uint8_t bell[8] = {0b00100, 0b01110, 0b01110, 0b01110,
                     0b11111, 0b00000, 0b00100, 0b00000};

  uint8_t arrow_up[8] = {0b00100, 0b01110, 0b11111, 0b00100,
                         0b00100, 0b00100, 0b00100, 0b00000};

  uint8_t arrow_down[8] = {0b00100, 0b00100, 0b00100, 0b00100,
                           0b11111, 0b01110, 0b00100, 0b00000};

  uint8_t battery[8] = {0b01110, 0b11111, 0b11111, 0b11111,
                        0b11111, 0b11111, 0b11111, 0b11111};

  // Create custom characters
  gdm1602a_create_char(0, heart);
  gdm1602a_create_char(1, smiley);
  gdm1602a_create_char(2, bell);
  gdm1602a_create_char(3, arrow_up);
  gdm1602a_create_char(4, arrow_down);
  gdm1602a_create_char(5, battery);

  // Display custom characters
  gdm1602a_clear();
  gdm1602a_set_cursor(0, 0);
  gdm1602a_putchar(0); // Heart
  gdm1602a_putchar(' ');
  gdm1602a_putchar(1); // Smiley
  gdm1602a_putchar(' ');
  gdm1602a_putchar(2); // Bell
  gdm1602a_putchar(' ');
  gdm1602a_puts("Custom!");

  gdm1602a_set_cursor(1, 0);
  gdm1602a_putchar(3); // Arrow up
  gdm1602a_putchar(' ');
  gdm1602a_putchar(4); // Arrow down
  gdm1602a_putchar(' ');
  gdm1602a_putchar(5); // Battery
  gdm1602a_putchar(' ');
  gdm1602a_puts("Chars");

  HAL_Delay(TEST_DELAY_LONG);
}

/**
 * @brief Test 7: Printf functionality
 */
void gdm1602a_test_printf_function(void) {
  gdm1602a_clear();
  gdm1602a_printf(0, 0, "Test 7: Printf");
  HAL_Delay(TEST_DELAY_SHORT);

  // Test integers
  gdm1602a_clear();
  for (int i = 0; i <= 10; i++) {
    gdm1602a_printf(0, 0, "Counter: %d", i);
    gdm1602a_printf(1, 0, "Hex: 0x%02X", i);
    HAL_Delay(300);
  }

  HAL_Delay(TEST_DELAY_SHORT);

  // Test strings
  gdm1602a_clear();
  gdm1602a_printf(0, 0, "Name: %s", "GDM1602A");
  gdm1602a_printf(1, 0, "MCU: %s", "STM32");
  HAL_Delay(TEST_DELAY_MEDIUM);

  // Test mixed types
  gdm1602a_clear();
  gdm1602a_printf(0, 0, "Temp: %d C", 25);
  gdm1602a_printf(1, 0, "Humid: %d%%", 65);
  HAL_Delay(TEST_DELAY_MEDIUM);
}

/**
 * @brief Test 8: Multiline text positioning
 */
void gdm1602a_test_multiline_text(void) {
  gdm1602a_clear();
  gdm1602a_printf(0, 0, "Test 8: Lines");
  HAL_Delay(TEST_DELAY_SHORT);

  // Test different column positions
  gdm1602a_clear();
  gdm1602a_set_cursor(0, 0);
  gdm1602a_puts("Left");
  gdm1602a_set_cursor(0, 11);
  gdm1602a_puts("Right");

  gdm1602a_set_cursor(1, 6);
  gdm1602a_puts("Center");
  HAL_Delay(TEST_DELAY_MEDIUM);

  // Test home function
  gdm1602a_clear();
  gdm1602a_set_cursor(1, 10);
  gdm1602a_puts("Bottom");
  HAL_Delay(TEST_DELAY_SHORT);

  gdm1602a_home(); // Should go to (0,0)
  gdm1602a_puts("Top (home)");
  HAL_Delay(TEST_DELAY_MEDIUM);
}

/**
 * @brief Test 9: Scrolling text animation
 */
void gdm1602a_test_scrolling_text(void) {
  gdm1602a_clear();
  gdm1602a_printf(0, 0, "Test 9: Scroll");
  HAL_Delay(TEST_DELAY_SHORT);

  const char *long_text = "This is a very long scrolling message!";
  int text_len = strlen(long_text);

  gdm1602a_clear();
  gdm1602a_set_cursor(0, 0);
  gdm1602a_puts("Scrolling:");

  // Scroll text across line 2
  for (int pos = 16; pos > -text_len; pos--) {
    gdm1602a_set_cursor(1, 0);
    gdm1602a_puts("                "); // Clear line

    gdm1602a_set_cursor(1, 0);

    // Display portion of text that fits on screen
    for (int i = 0; i < 16; i++) {
      int char_index = i - pos;
      if (char_index >= 0 && char_index < text_len) {
        gdm1602a_putchar(long_text[char_index]);
      } else {
        gdm1602a_putchar(' ');
      }
    }

    HAL_Delay(200);
  }

  HAL_Delay(TEST_DELAY_SHORT);
}
