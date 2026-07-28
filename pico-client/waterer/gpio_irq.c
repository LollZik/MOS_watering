#include "hardware/irq.h"
#include "hardware/gpio.h"

#include <stdio.h>

#include "sched.h"
#include "pins.h"

extern task_ctx_t disp_task_ctx;

void gpio_callback(uint gpio, uint32_t events) {
  switch (gpio) {
    case GPIO_BUTTON_IRQ_PIN: 
      {
        enable_task(&disp_task_ctx);
        break;
      }
    default:
      break;
  }
}

void
init_gpio(void)
{
  gpio_init(GPIO_BUTTON_IRQ_PIN);

  gpio_set_irq_enabled_with_callback(GPIO_BUTTON_IRQ_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
}
