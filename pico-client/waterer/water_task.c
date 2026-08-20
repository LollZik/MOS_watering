#include <assert.h>
#include "hal_memory.h"
#include "sched.h"
#include "hal_gpio.h"
#include "pins.h"

#define WAIT 0
#define WATER 1
#define END_WATER 2

static uint8_t cur_state = WAIT;

extern task_ctx_t water_task_ctx;

void
start_watering(void)
{
  cur_state = WATER;
  enable_task(&water_task_ctx);
}

int
set_watering_time(uint32_t ms)
{
  ram_shared.watering_time = ms;
}

int
water_init(void)
{
  hal_gpio_init_out(GPIO_WATERING_PIN, false);
  return 0;
}

int
water_task(void)
{
  switch(cur_state)
  {
    case WAIT:
      {
        cur_state = WATER;  
        return 10;
      }
    case WATER:
      {
        hal_gpio_set(GPIO_WATERING_PIN, true);
        cur_state = END_WATER;
        return ram_shared.watering_time;
      }
    case END_WATER:
      {
        hal_gpio_set(GPIO_WATERING_PIN, false);
        cur_state = WAIT;
        return -1;
      }
    default:
      {
        assert(1 == 0); //We should never be here
      }
  }

  return 2900;
}

REGISTER_TASK("Watering task", -1, water_task, water_init, false);
