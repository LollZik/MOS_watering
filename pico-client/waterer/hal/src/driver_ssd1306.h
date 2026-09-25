#pragma once
#include <stdint.h>
#include <stdbool.h>

#define SSD1306_HEIGHT              32
#define SSD1306_WIDTH               128

#define SSD1306_PAGE_HEIGHT         8
#define SSD1306_NUM_PAGES           (SSD1306_HEIGHT / SSD1306_PAGE_HEIGHT)
#define SSD1306_BUF_LEN             (SSD1306_NUM_PAGES * SSD1306_WIDTH)

void driver_ssd1306_init(void);
void driver_ssd1306_disable(void);
void driver_ssd1306_render(uint8_t *buf, uint8_t x_start, uint8_t x_end, uint8_t y_start, uint8_t y_end);
void driver_ssd1306_scroll(bool on);