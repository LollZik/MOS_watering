#include <string.h>
#include <stdio.h>

#include "dispatcher.h"
#include "hal_network.h"


uint8_t tx_buf[sizeof(packet_t)];

extern handle_packet dispatch_table[256];

void
send_response(hal_net_conn_t conn, packet_t *packet)
{
  printf("Sending packet of size :%u, header: %02X\n", sizeof(header_t) + packet->header.length, packet->header.cmd_ack);

  hal_network_send(conn, packet, (sizeof(header_t) + packet->header.length));
}

void
send_error_response(hal_net_conn_t conn, uint8_t ack, uint16_t msg_id)
{
  packet_t *packet = (packet_t *)tx_buf;

  packet->header.cmd_ack = ack;
  packet->header.msg_id = msg_id;
  packet->header.length = 0;

  send_response(conn, packet);
}

void
send_ok_response(hal_net_conn_t conn, uint8_t ack, uint16_t msg_id, uint16_t length)
{
  packet_t *packet = (packet_t *)tx_buf;

  packet->header.cmd_ack = ack;
  packet->header.msg_id = msg_id;
  packet->header.length = length;

  send_response(conn, packet);
}

void
dispatch(packet_t *in_packet, uint16_t len, hal_net_conn_t conn)
{
  const uint8_t cmd = in_packet->header.cmd_ack;
  const uint16_t msg_id = in_packet->header.msg_id;

  if (dispatch_table[cmd] == NULL) {
    send_error_response(conn, ACK_CMD_ERR, msg_id);
    return;
  }

  if (len != sizeof(header_t) + in_packet->header.length) {
    send_error_response(conn, ACK_LEN_ERR, msg_id);
    return;
  }

  uint16_t resp_len = 0;
  uint8_t ack = dispatch_table[cmd](in_packet, (packet_t *)tx_buf, &resp_len);

  if (ack == ACK_OK) {
    ack = cmd;
  }

  send_ok_response(conn, ack, msg_id, resp_len);
}

uint8_t 
ignore_cmd_handle(packet_t *in_packet, packet_t *out_packet, uint16_t *out_len) 
{
    *out_len = 0;
    return ACK_OK;
}