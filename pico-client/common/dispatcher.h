#pragma once

#include <time.h>
#include <stdint.h>
#include "proto.h"
#include "hal_network.h"

typedef uint8_t (*handle_packet)(packet_t *in_packet, packet_t *out_packet, uint16_t *out_len);

void dispatch(packet_t *packet, uint16_t len, hal_net_conn_t conn);

uint8_t ignore_cmd_handle(packet_t *in_packet, packet_t *out_packet, uint16_t *out_len);