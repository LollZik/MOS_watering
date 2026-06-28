#include <stdio.h>

#include "shared_mem.h"
#include "water_ctx.h"
#include "hardware/adc.h"
#include "sched.h"

#define MOIST_TASK_TIMEOUT_MS  2000
#define AFTER_WATERING_TASK_TIMEOUT  20000

#include "pins.h"

static uint16_t moist_lvl;
static uint16_t thresh_moist_lvl = 0x8000;

static absolute_time_t deadline = 0;

uint16_t
get_moist_lvl(void)
{
  return moist_lvl; 
}

void
set_moist_thresh(uint16_t new_moist_thresh)
{
  ram_shared.moist_thresh = new_moist_thresh;
  save_config_to_flash();
}

int
moist_init(void)
{
  adc_init();
  adc_gpio_init(MOISTURE_ADC_PIN);
  adc_select_input(MOISTURE_ADC_PIN - ADC_PINS_OFFSET);

  return 0;
}

int
moist_task(void)
{
  adc_select_input(MOISTURE_ADC_PIN - ADC_PINS_OFFSET);
  moist_lvl = adc_read();
  printf("Moist lvl: %u, thresh: %u\n", moist_lvl, ram_shared.moist_thresh);

  if (moist_lvl <= ram_shared.moist_thresh)
  {
    const absolute_time_t cur_time = get_absolute_time();
    if (deadline <= cur_time)
    {
      start_watering();
      deadline = make_timeout_time_ms(AFTER_WATERING_TASK_TIMEOUT);
    }
  }

  return MOIST_TASK_TIMEOUT_MS;  
}
