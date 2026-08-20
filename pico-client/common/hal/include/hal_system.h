#pragma once
#include <stdint.h>
#include <stdbool.h>

void hal_system_reset(void);
void hal_system_watchdog_init(uint32_t timeout_ms);
void hal_system_watchdog_feed(void);

void hal_system_irq_init(void);
uint64_t hal_system_get_time_us(void);
void hal_system_sleep_until_us(uint64_t deadline_us);