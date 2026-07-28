#include <string.h>
#include "pico/unique_id.h"
#include "pico/cyw43_arch.h"
#include "dispatch_table.h"
#include "dispatcher.h"

handle_packet dispatch_table[256] = {
    [GET_INFO_CMD] = get_info_handle,
    [GET_WATERING_CTX_CMD] = ignore_cmd_handle,
    [PEER_DISCOVERY_CMD]   = ignore_cmd_handle,
    [TRIGGER_WATER_CMD]    = ignore_cmd_handle
};

uint8_t
get_info_handle(packet_t *in_packet, packet_t *out_packet, uint16_t *out_len)
{
  strncpy((char*)out_packet->data.get_info.name, "Displayer", MAX_NAME_LEN);
  pico_get_unique_board_id((pico_unique_board_id_t *) &out_packet->data.get_info.uuid);

  out_packet->data.get_info.role = ROLE_DISPLAYER;

  struct netif *n = &cyw43_state.netif[CYW43_ITF_STA];
  out_packet->data.get_info.ip = netif_ip4_addr(n)->addr;
  
  *out_len = sizeof(get_info_t);

  return ACK_OK;
}