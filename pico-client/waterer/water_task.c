#include "shared_mem.h"
#include "sched.h"

#define WAIT 0
#define WATER 1
#define END_WATER 2

static uint8_t cur_state = WAIT;

void
start_watering(void)
{
  cur_state = WATER;
  enable_task(&(tasks[WATER_TASK_INDEX]));
}

int
set_watering_time(uint32_t ms)
{
  ram_shared.watering_time = ms;
}

int
water_init(void)
{
  gpio_init(GPIO_WATERING_PIN);
  gpio_set_dir(GPIO_WATERING_PIN, GPIO_OUT);
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
        gpio_put(GPIO_WATERING_PIN, 1U);
        cur_state = END_WATER;
        return ram_shared.watering_time;
      }
    case END_WATER:
      {
        gpio_put(GPIO_WATERING_PIN, 0U);
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
