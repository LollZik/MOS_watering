#include <stdio.h>

#include "hardware/adc.h"
#include "sched.h"

#define TEMP_TASK_TIMEOUT_MS  2000
#define TEMP_INPUT            4

#include "pins.h"

static uint16_t temp_lvl;

uint16_t
get_temp_lvl(void)
{
  return temp_lvl;
}

int
temp_init(void)
{
  adc_init();
  
  return 0;
}

int
temp_task(void)
{
  adc_select_input(TEMP_INPUT);
  temp_lvl = adc_read();

  return TEMP_TASK_TIMEOUT_MS;  
}

REGISTER_TASK("Temperature task", TEMP_TASK_TIMEOUT_MS, temp_task, temp_init, true);
