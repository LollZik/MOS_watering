/** * Copyright (c) 2022 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "pico/cyw43_arch.h"
#include "pico/stdio.h"
#include "hardware/watchdog.h"

#include "dispatcher.h"
#include "dispatch_table.h"
#include "sched.h"
#include "proto.h"
#include "shared_mem.h"
#include "tcp_client.h"
#include "reset.h"


#if !defined(PROJECT_VERSION_MAJOR) || !defined(PROJECT_VERSION_MINOR)|| !defined(PROJECT_VERSION_PATCH)
  #error "PROJECT_VERSION cannot be read"
#endif
#define DEBUG_printf printf

extern void init_gpio(void);

struct tcp_pcb *server_tcp = NULL;
struct tcp_pcb *display_tcp = NULL;

void
__main(void)
{
  if (!tcp_client_init_and_connect(TEST_TCP_SERVER_IP, &server_tcp)) {
      printf("Error: TCP Server init failed\n");
      return; 
  }

  // Wait until server sends a peer_discovery packet wiith displayer's IP
  while (!displayer_ip_received) {
      sleep_ms(100);
  }
  char displayer_ip_str[16]; 
  strncpy(displayer_ip_str, ipaddr_ntoa(&displayer_ip), sizeof(displayer_ip_str));
  displayer_ip_str[sizeof(displayer_ip_str) - 1] = '\0';

  printf("Displayer IP received: %s - Connecting...\n", displayer_ip_str);


  if (!tcp_client_init_and_connect(displayer_ip_str, &display_tcp)) {
      printf("Error: Displayer TCP init failed for IP: %s\n", displayer_ip_str);
      return; 
  }

  __run_sched();
}

void __attribute__((__noreturn__)) 
main()
{
  stdio_init_all();
  sleep_ms(5000);

  if (cyw43_arch_init()) {
    printf("CYW43 initialisation failed\n");
    exit(-1);
  }

  printf("Enabling STA mode\n");
  cyw43_arch_enable_sta_mode();

  printf("Connecting to %s Wi-Fi...\n", WIFI_SSID);
  while (1) {
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASS, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        printf("Wi-Fi connection failed, retrying.\n");
    }
    else {
      printf("Connected.\n");
      cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
      break;
    }
  }

  printf("Welcome in waterer software!\n");
  printf("My name is : %s\n", get_name());
  printf("Running at %08X\n", (uint32_t)CUR_SLOT_ORIGIN);
  printf("Running slot  in flash: %02X\n", get_running_slot_id());
  printf("Running firmware ver: %02X.%02X.%02X\n",
         PROJECT_VERSION_MAJOR, PROJECT_VERSION_MINOR, PROJECT_VERSION_PATCH);

  init_gpio();
  load_config_from_flash();

  __main();
  
  reset_pico();
  cyw43_arch_deinit();
  exit(0);
}