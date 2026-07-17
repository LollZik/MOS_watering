#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H

#include "lwip/tcp.h"
#include "proto.h"
#include <stdbool.h>

bool tcp_client_init_and_connect(const char* ip_addr, struct tcp_pcb **out_ptr);

#endif