#pragma once

#include "proto.h"
#include "lwip/tcp.h"
#include "dispatcher.h"

uint8_t get_info_handle(packet_t *in_packet, packet_t *out_packet, uint16_t *out_len);