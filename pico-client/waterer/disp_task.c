#include <stdio.h>
#include "sched.h"
#include "hal_qled.h"
#include "hal_system.h"

#define UPDATE_COUNT 5
#define SCREEN_DELTA_MS 1000
#define UPDATE_COUNT 5

#define SCREEN_DELTA_MS 1000

const char *pico_name;

static uint8_t update_count = 0;

int
disp_task(void)
{
  if (update_count == 0) {
    hal_qled_init();
  }

  if (update_count >= UPDATE_COUNT) {
    update_count = 0;
    hal_qled_disable();
    return -1;
  } else {
    update_count++;
  }

  const uint32_t time_s = (uint32_t)(hal_system_get_time_us() / 1000000ULL);

  char text[16];
  if (time_s < 60) {
    snprintf(text, 16, "0000:00:%02u", time_s);
  } else if (time_s < 3600) {
    snprintf(text, 16, "0000:%02u:%02u", time_s/60, time_s%60);
  } else {
    snprintf(text, 16, "%04u:%02u:%02u", time_s/3600, (time_s%3600), time_s%60);
  }

  uint8_t id_bytes[8];
  hal_system_get_board_id(id_bytes);
  
  char id_str[17];
  for (int i = 0; i < 8; i++) 
  {
    sprintf(&id_str[i * 2], "%02X", id_bytes[i]); 
  }

  hal_qled_set_string(id_str, 0);
  hal_qled_set_string("Since boot:      ", 1);
  hal_qled_set_string(text, 2);
  hal_qled_draw();

  return SCREEN_DELTA_MS;
}

int
disp_init(void)
{
  hal_qled_init();

  return 3000;
}

REGISTER_TASK("Display task", 500, disp_task, disp_init, false);
