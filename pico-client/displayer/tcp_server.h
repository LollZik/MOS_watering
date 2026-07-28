#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include <stdbool.h>
#include <stdint.h>

#define LISTEN_PORT 8080

extern volatile bool new_data;
extern uint8_t battery_level;
extern uint8_t water_level;
extern uint16_t temp_level;
extern uint16_t moist_level;

void tcp_server_init(void);

#endif