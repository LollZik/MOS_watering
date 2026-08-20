#include "hal_wlan.h"
#include "pico/cyw43_arch.h"

uint8_t
hal_wlan_init(void)
{
  if (cyw43_arch_init_with_country(CYW43_COUNTRY_POLAND)) {
    return false;
  }
  cyw43_arch_enable_sta_mode();
  return true;
}

void
hal_wlan_deinit(void)
{
  cyw43_arch_deinit();
}

uint8_t
hal_wlan_connect_timeout(const char *ssid, const char *pass, uint32_t timeout_ms)
{
  int err = cyw43_arch_wifi_connect_timeout_ms(ssid, pass, CYW43_AUTH_WPA2_AES_PSK, timeout_ms);
  return (err == 0);
}
