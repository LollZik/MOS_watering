/** * Copyright (c) 2022 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "dispatcher.h"
#include "dispatch_table.h"
#include "sched.h"
#include "proto.h"

#include "hal_system.h"
#include "hal_wlan.h"
#include "hal_network.h"
#include "hal_memory.h"
#include "hal_gpio.h"

#if !defined(PROJECT_VERSION_MAJOR) || !defined(PROJECT_VERSION_MINOR)|| !defined(PROJECT_VERSION_PATCH)
  #error "PROJECT_VERSION cannot be read"
#endif
#define DEBUG_printf printf

extern void init_gpio(void);

hal_net_conn_t server_conn = NULL;
hal_net_conn_t display_conn = NULL;

void
__main(void)
{
  if (!hal_network_init_and_connect(TEST_TCP_SERVER_IP, &server_conn)) {
      printf("Error: TCP Server init failed\n");
      return; 
  }

  // Wait until server sends a peer_discovery packet wiith displayer's IP
  while (!displayer_ip_received) {
      hal_system_sleep_until_us(hal_system_get_time_us() + 100000);
  }

  char displayer_ip_str[16]; 
  uint8_t *bytes = (uint8_t *)&displayer_ip;
  snprintf(displayer_ip_str, sizeof(displayer_ip_str), "%u.%u.%u.%u", bytes[0], bytes[1], bytes[2], bytes[3]);

  printf("Displayer IP received: %s - Connecting...\n", displayer_ip_str);


  if (!hal_network_init_and_connect(displayer_ip_str, &display_conn)) {
      printf("Error: Displayer TCP init failed for IP: %s\n", displayer_ip_str);
      return; 
  }

  __run_sched();
}

void __attribute__((__noreturn__)) 
main()
{
  hal_system_init();
  hal_system_sleep_until_us(hal_system_get_time_us() + 5000000ULL);

  if (!hal_wlan_init()) {
    printf("CYW43 initialisation failed\n");
    exit(-1);
  }


  printf("Connecting to %s Wi-Fi...\n", WIFI_SSID);
  while (1) {
    if (!hal_wlan_connect_timeout(WIFI_SSID, WIFI_PASS, 30000)) {
        printf("Wi-Fi connection failed, retrying.\n");
    }
    else {
      printf("Connected.\n");
      hal_board_set_led(true);
      break;
    }
  }

  printf("Welcome in waterer software!\n");
  printf("My name is : %s\n", hal_memory_get_name());
  printf("Running at %08X\n", (uint32_t)CUR_SLOT_ORIGIN);
  printf("Running slot  in flash: %02X\n", hal_memory_get_running_slot_id());
  printf("Running firmware ver: %02X.%02X.%02X\n",
         PROJECT_VERSION_MAJOR, PROJECT_VERSION_MINOR, PROJECT_VERSION_PATCH);

  init_gpio();

  hal_config_t cfg;
  hal_memory_load_config(&cfg);

  __main();
  
  hal_system_reset();
  hal_wlan_deinit();
  exit(0);
}