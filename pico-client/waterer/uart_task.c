#include "sched.h"
#include "hal_gpio.h"

static bool led_state = false;

int
uart_task(void)
{
  // led_state = !led_state;
  // hal_board_set_led(led_state);

  return 2900;
}

REGISTER_TASK("Uart task", 1000, uart_task, NULL, true);
