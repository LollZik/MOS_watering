#include <stdio.h>
#include "sched.h"
#include "hal_gpio.h"
#include "pins.h"

extern task_ctx_t disp_task_ctx;

void gpio_callback(uint8_t gpio, uint32_t events) {
  switch (gpio) 
  {
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
  hal_gpio_init_irq(GPIO_BUTTON_IRQ_PIN, gpio_callback);
}
