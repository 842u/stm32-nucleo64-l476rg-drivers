#ifndef DELAY_US_H
#define DELAY_US_H

#include <stdbool.h>
#include <stdint.h>

void delay_us_init(void);
void delay_us(uint32_t us);
bool delay_us_is_initialized(void);

#endif
