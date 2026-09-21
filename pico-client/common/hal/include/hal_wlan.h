#pragma once
#include <stdint.h>

uint8_t hal_wlan_init(void);
void hal_wlan_deinit(void);
uint8_t hal_wlan_connect_timeout(const char *ssid, const char *pass, uint32_t timeout_ms);