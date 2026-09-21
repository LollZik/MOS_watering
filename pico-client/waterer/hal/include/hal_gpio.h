#pragma once
#include <stdbool.h>
#include <stdint.h>

typedef void (*hal_gpio_irq_cb_t)(uint8_t pin, uint32_t events);

void hal_gpio_init_out(uint8_t pin, bool initial_state);
void hal_gpio_set(uint8_t pin, bool state);
void hal_gpio_init_irq(uint8_t pin, hal_gpio_irq_cb_t cb);
void hal_board_set_led(bool state);