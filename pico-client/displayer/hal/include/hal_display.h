#pragma once

#include <stdint.h>
#include <stdbool.h>

void hal_display_init(void);
void hal_display_wait_until_idle(void);
bool hal_display_is_busy(void);
void hal_display_write_framebuffer(const uint8_t *framebuf);
void hal_display_trigger_refresh(void);