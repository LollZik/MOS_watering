#include <stdio.h>
#include <time.h>

#include "pico/cyw43_arch.h"

#include "sched.h"

int
uart_task(void)
{
  //bool gpio_val = !cyw43_arch_gpio_get(CYW43_WL_GPIO_LED_PIN);
  // Instead of spewing jibberish on serial console, just blink LED
  //cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, gpio_val);

  //if (gpio_val) {
  //  return 100;
  //}

  return 2900;
}

REGISTER_TASK("Uart task", 1000, uart_task, NULL, true);
