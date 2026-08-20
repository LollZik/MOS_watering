#include <stdio.h>
#include "hal_memory.h"
#include "water_ctx.h"
#include "sched.h"
#include "hal_adc.h"
#include "hal_system.h"
#include "pins.h"

#define MOIST_TASK_TIMEOUT_MS  2000
#define AFTER_WATERING_TASK_TIMEOUT_MS  20000

static uint16_t moist_lvl;
static uint64_t deadline = 0;

uint16_t
get_moist_lvl(void)
{
  return moist_lvl; 
}

void
set_moist_thresh(uint16_t new_moist_thresh)
{
  ram_shared.moist_thresh = new_moist_thresh;
  hal_memory_save_config(&shared);
}

int
moist_init(void)
{
  hal_adc_init();
  hal_adc_setup_pin(MOISTURE_ADC_PIN);

  return 0;
}

int
moist_task(void)
{
  moist_lvl = hal_adc_read_channel(MOISTURE_ADC_PIN - ADC_PINS_OFFSET);
  printf("Moist lvl: %u, thresh: %u\n", moist_lvl, ram_shared.moist_thresh);

  if (moist_lvl <= ram_shared.moist_thresh)
  {
    const uint64_t cur_time = hal_system_get_time_us();
    if (deadline <= cur_time)
    {
      printf("Watering the plants\n");
      start_watering();
      deadline = cur_time + (AFTER_WATERING_TASK_TIMEOUT_MS * 1000ULL);
    }
  }

  return MOIST_TASK_TIMEOUT_MS;  
}

REGISTER_TASK("Moisture task", MOIST_TASK_TIMEOUT_MS, moist_task, moist_init, true);

