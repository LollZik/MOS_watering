#include "sched.h"
#include "hal_adc.h"
#include "pins.h"

#define TEMP_TASK_TIMEOUT_MS  2000
#define TEMP_INPUT            4

static uint16_t temp_lvl;

uint16_t
get_temp_lvl(void)
{
  return temp_lvl;
}

int
temp_init(void)
{
  hal_adc_init();
  
  return 0;
}

int
temp_task(void)
{
  temp_lvl = hal_adc_read_channel(TEMP_INPUT);

  return TEMP_TASK_TIMEOUT_MS;  
}

REGISTER_TASK("Temperature task", TEMP_TASK_TIMEOUT_MS, temp_task, temp_init, true);
