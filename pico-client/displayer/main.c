#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#include "sched.h"
#include "proto.h"
#include "tcp_server.h"
#include "tcp_client.h"
#include "dispatcher.h"

struct tcp_pcb *server_tcp = NULL;
struct tcp_pcb *display_tcp = NULL;

int main() {
    stdio_init_all();

    if (cyw43_arch_init_with_country(CYW43_COUNTRY_POLAND)) {
        return 1;
    }
    
    cyw43_arch_enable_sta_mode();

    while (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASS, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        printf("Connection failed. Retrying...\n");
    }


    if (!tcp_client_init_and_connect(TEST_TCP_SERVER_IP, &server_tcp)) {
        printf("Error: TCP Server init failed\n");
    }

    tcp_server_init();

    should_wake_up = true;
    __run_sched();
    return 0;
}