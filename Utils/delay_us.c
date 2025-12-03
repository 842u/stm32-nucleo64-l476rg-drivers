#include "delay_us.h"
#include "stm32l4xx_hal.h"
#include <stdbool.h>

static bool dwt_initialized = false;

/**
 * @brief Initialize DWT cycle counter for microsecond delays
 */
void delay_us_init(void) {
  // Unlock access to DWT registers
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;

  // Small delay to ensure DWT is ready
  for (volatile int i = 0; i < 100; i++)
    ;

  // Reset cycle counter
  DWT->CYCCNT = 0;

  // Enable cycle counter
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

  // Verify it's actually counting
  uint32_t start = DWT->CYCCNT;
  for (volatile int i = 0; i < 1000; i++)
    ;
  uint32_t end = DWT->CYCCNT;

  if (end > start) {
    dwt_initialized = true;
  } else {
    // DWT failed to initialize - fallback to busy wait
    dwt_initialized = false;
  }
}

/**
 * @brief Fallback busy-wait delay (less accurate)
 */
static void delay_us_fallback(uint32_t us) {
  // Approximate cycles per microsecond
  uint32_t cycles = (SystemCoreClock / 1000000) * us / 3;

  while (cycles--) {
    __NOP();
  }
}

/**
 * @brief Precise microsecond delay using DWT cycle counter
 * @param us Microseconds to delay
 */
void delay_us(uint32_t us) {
  // Use fallback if DWT not initialized
  if (!dwt_initialized) {
    delay_us_fallback(us);
    return;
  }

  uint32_t start_ticks = DWT->CYCCNT;
  uint32_t ticks = us * (SystemCoreClock / 1000000);

  // Prevent infinite loop for very small delays
  if (ticks == 0) {
    ticks = 1;
  }

  // Wait with timeout protection (max ~53 seconds at 80MHz)
  uint32_t timeout = 0xFFFFFFFF;
  while (((DWT->CYCCNT - start_ticks) < ticks) && (timeout-- > 0))
    ;
}

/**
 * @brief Check if DWT is initialized properly
 */
bool delay_us_is_initialized(void) { return dwt_initialized; }
