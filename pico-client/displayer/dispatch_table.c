#include <string.h>

#include "dispatch_table.h"
#include "dispatcher.h"
#include "hal_system.h"
#include "hal_network.h"

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
  hal_system_get_board_id((uint8_t *)&out_packet->data.get_info.uuid);
  out_packet->data.get_info.role = ROLE_DISPLAYER;
  out_packet->data.get_info.ip = hal_network_get_ip_v4();
  
  *out_len = sizeof(get_info_t);

  return ACK_OK;
}