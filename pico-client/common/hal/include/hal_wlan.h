#pragma once

#include <stdbool.h>
#include <stdint.h>

bool hal_wlan_init(void);
void hal_wlan_deinit(void);
bool hal_wlan_connect_timeout(const char *ssid, const char *pass, uint32_t timeout_ms);