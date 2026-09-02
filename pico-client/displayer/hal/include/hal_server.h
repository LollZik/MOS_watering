#pragma once

#include <stdbool.h>
#include <stdint.h>

#define LISTEN_PORT 8080

typedef void* hal_server_conn_t;

extern hal_server_conn_t display_conn;

extern volatile bool new_data;
extern uint8_t battery_level;
extern uint8_t water_level;
extern uint16_t temp_level;
extern uint16_t moist_level;

void tcp_server_init(void);
