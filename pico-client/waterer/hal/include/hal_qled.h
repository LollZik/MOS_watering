#pragma once
#include <stdint.h>

void hal_qled_init(void);
void hal_qled_set_string(const char *text, uint8_t column);
void hal_qled_draw(void);
void hal_qled_disable(void);