#pragma once

void start_watering(void);
int set_watering_time(uint32_t ms);
void set_moist_thresh(const uint16_t thresh_moist);
uint16_t get_moist_lvl(void);
uint16_t get_temp_lvl(void);
