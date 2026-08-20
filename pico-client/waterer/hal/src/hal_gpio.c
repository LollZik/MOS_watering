#include "hal_gpio.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "pico/cyw43_arch.h"

static hal_gpio_irq_cb_t app_cb = NULL;

static void
gpio_callback(uint gpio, uint32_t events)
{
  if (app_cb != NULL) {
    app_cb((uint8_t)gpio, events);
  }
}

void
hal_gpio_init_out(uint8_t pin, bool initial_state)
{
  gpio_init(pin);
  gpio_set_dir(pin, GPIO_OUT);
  gpio_put(pin, initial_state ? 1 : 0);
}

void
hal_gpio_set(uint8_t pin, bool state)
{
  gpio_put(pin, state ? 1 : 0);
}

void
hal_gpio_init_irq(uint8_t pin, hal_gpio_irq_cb_t cb)
{
  app_cb = cb;
  gpio_init(pin);
  gpio_set_irq_enabled_with_callback(pin, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
}

void
hal_board_set_led(bool state)
{
  cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, state ? 1 : 0);
}