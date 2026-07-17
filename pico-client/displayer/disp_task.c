#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"

#include "ssd1680.h"
#include "text_renderer.h"
#include "sched.h"

#define WAIT_TIME 10000

extern volatile bool new_data;
extern uint8_t battery_level;
extern uint8_t water_level;
extern uint16_t temp_level;
extern uint16_t moist_level;

typedef enum {
    STATE_CHECK_DATA,
    STATE_WAIT_FOR_SCREEN
} disp_state_t;

static disp_state_t current_state = STATE_CHECK_DATA;

int disp_init(void) {
  epd_init();
  clear_buffer();
  draw_string(10, 10, "Waiting for data");
  draw_logo();
  epd_write_framebuffer(framebuf);
  epd_trigger_refresh();
  epd_wait_until_idle();

  return -1;
}

int disp_task(void) {
    switch (current_state){
        case STATE_CHECK_DATA:
            if (!new_data) {
                return WAIT_TIME;
            }
            new_data = false;
            clear_buffer();
            char buf[64];
            
            snprintf(buf, sizeof(buf), 
                     "Battery: %d%%\n"
                     "Water: %d%%\n"
                     "Temp: %u\n"
                     "CH0 Moist: %u", 
                     battery_level, water_level, temp_level, moist_level);
            
            draw_string(5, 5, buf);
            draw_logo();
            
            epd_write_framebuffer(framebuf);
            epd_trigger_refresh();

            current_state = STATE_WAIT_FOR_SCREEN;
            return 100;

        case STATE_WAIT_FOR_SCREEN:
            if (epd_is_busy()) {
                return 100;
            }
            current_state = STATE_CHECK_DATA;
            return WAIT_TIME;
    }
    return WAIT_TIME;
}

REGISTER_TASK("Display task", 500, disp_task, disp_init, true);