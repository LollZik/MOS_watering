#include <stdio.h>
#include "hal_wlan.h"
#include "hal_network.h"
#include "hal_server.h"
#include "hal_system.h"
#include "sched.h"
#include "proto.h"
#include "dispatcher.h"

hal_net_conn_t server_conn = NULL;

int
main()
{
  hal_system_init();
  if (!hal_wlan_init()) {
    return 1;
  }

  while (!hal_wlan_connect_timeout(WIFI_SSID, WIFI_PASS, 30000)) {
    printf("Connection failed. Retrying...\n");
  }

  if (!hal_network_init_and_connect(TEST_TCP_SERVER_IP, &server_conn)) {
    printf("Error: TCP Server init failed\n");
  }

  tcp_server_init();
  should_wake_up = true;
  __run_sched();

  return 0;
}