#include <string.h>

#include "pico/time.h"
#include "pico/unique_id.h"
#include "pico/cyw43_arch.h"

#include "hardware/flash.h"
#include "hardware/sync.h"
#include "shared_mem.h"

#include "dispatcher.h"
#include "reset.h"


uint8_t tx_buf[sizeof(packet_t)];

extern handle_packet dispatch_table[256];

void
dispatch(packet_t *in_packet, uint16_t len, struct tcp_pcb *tpcb)
{
  const uint8_t cmd = in_packet->header.cmd_ack;
  const uint16_t msg_id = in_packet->header.msg_id;

  if (dispatch_table[cmd] == NULL) {
    send_error_response(tpcb, ACK_CMD_ERR, msg_id);
    return;
  }

  if (len != sizeof(header_t) + in_packet->header.length) {
    send_error_response(tpcb, ACK_LEN_ERR, msg_id);
    return;
  }

  uint16_t resp_len = 0;
  uint8_t ack = dispatch_table[cmd](in_packet, (packet_t *)tx_buf, &resp_len);

  if (ack == ACK_OK) {
    ack = cmd;
  }

  send_ok_response(tpcb, ack, msg_id, resp_len);
}

void
send_response(struct tcp_pcb *tpcb, packet_t *packet)
{

  uint8_t *data = (uint8_t *)packet;

  printf("Sending packet of size :%u, header: %02X\n", sizeof(header_t) + packet->header.length, packet->header.cmd_ack);

  tcp_write(tpcb, packet, (sizeof(header_t) + packet->header.length), TCP_WRITE_FLAG_MORE);
}

void
send_error_response(struct tcp_pcb *tpcb, uint8_t ack, uint16_t msg_id)
{
  packet_t *packet = (packet_t *)tx_buf;

  packet->header.cmd_ack = ack;
  packet->header.msg_id = msg_id;
  packet->header.length = 0;

  send_response(tpcb, packet);
}

void
send_ok_response(struct tcp_pcb *tpcb, uint8_t ack, uint16_t msg_id, uint16_t length)
{
  packet_t *packet = (packet_t *)tx_buf;

  packet->header.cmd_ack = ack;
  packet->header.msg_id = msg_id;
  packet->header.length = length;

  send_response(tpcb, packet);
}

uint8_t 
ignore_cmd_handle(packet_t *in_packet, packet_t *out_packet, uint16_t *out_len) 
{
    *out_len = 0;
    return ACK_OK;
}