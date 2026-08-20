#pragma once
#include <stdint.h>

void hal_adc_init(void);
void hal_adc_setup_pin(uint8_t pin);
uint16_t hal_adc_read_channel(uint8_t channel);