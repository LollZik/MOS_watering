#pragma once

#include <time.h>

#include "proto.h"
#include "lwip/tcp.h"

typedef uint8_t (*handle_packet)(packet_t *in_packet, packet_t *out_packet, uint16_t *out_len);

void dispatch(packet_t *packet, uint16_t len, struct tcp_pcb *tpcb);

void send_response(struct tcp_pcb *tpcb, packet_t *packet);
void send_error_response(struct tcp_pcb *tpcb, uint8_t ack, uint16_t msg_id);
void send_ok_response(struct tcp_pcb *tpcb, uint8_t ack, uint16_t msg_id, uint16_t length);

uint8_t ignore_cmd_handle(packet_t *in_packet, packet_t *out_packet, uint16_t *out_len);