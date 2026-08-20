#pragma once
#include <stdbool.h>
#include <stdint.h>

typedef void* hal_net_conn_t;

bool hal_network_init_and_connect(const char* ip_addr, hal_net_conn_t *out_ptr);

void hal_network_send(hal_net_conn_t conn, const void *data, uint16_t len);
void hal_network_flush(hal_net_conn_t conn);

uint32_t hal_network_get_ip_v4(void);

void hal_network_lock(void);
void hal_network_unlock(void);