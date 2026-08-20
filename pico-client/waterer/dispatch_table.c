#include <string.h>
#include <stdio.h>
#include <assert.h>

#include "helpers.h"
#include "hal_memory.h"
#include "hal_flash.h"
#include "hal_system.h"
#include "hal_network.h"

#include "dispatch_table.h"
#include "dispatcher.h"

#include "water_ctx.h"
#include "sched.h"

#define PHYSICAL_SLOT_SIZE (SLOT1_ORIGIN - SLOT0_ORIGIN)

volatile bool displayer_ip_received = false;
uint32_t displayer_ip = 0;


handle_packet dispatch_table[256] = {
  [GET_WATERING_CTX_CMD]    = get_watering_ctx,

  [READ_SW_VERSION_CMD]     = read_sw_version_handle,
  [READ_RUNNING_SLOT_CMD]   = get_running_slot_handle,

  [SET_ACTIVE_SLOT_CMD]     = set_active_slot_handle,

  [RESET_PICO_CMD]          = reset_handle,

  [SET_NAME_CMD]            = set_name_handle,
  [GET_INFO_CMD]            = get_info_handle,

  [FLASH_WRITE_CMD]         = flash_write,
  [FLASH_ERASE_CMD]         = flash_erase,

  [TRIGGER_WATER_CMD]       = trigger_water,
  [SET_WATERING_TIME]       = set_watering_time_cmd,
  [SET_WATER_THRESHOLD]     = set_water_thresh_cmd,

  [PEER_DISCOVERY_CMD]      = peer_discovery_handle,
};

uint8_t
flash_erase(packet_t *packet, packet_t *out_packet, uint16_t *out_len)
{
  const uint32_t addr = get_be24(packet->data.flash_erase.addr);

  if ( packet->header.length != sizeof(erase_flash_data_t) ) {
    return ACK_LEN_ERR;
  }

  if ( addr + FLASH_PAGE_SIZE > CUR_SLOT_ORIGIN && addr < CUR_SLOT_ORIGIN + PHYSICAL_SLOT_SIZE ) {
    return ACK_PARAM_ERR;
  }

  if ( addr >= BOOTLOADER_ORIGIN && addr < SLOT0_ORIGIN ) {
    return ACK_PARAM_ERR;
  }

  if ( addr % FLASH_PAGE_SIZE != 0) {
    return ACK_PARAM_ERR;
  }
  hal_flash_erase_page(addr);

  *out_len = 0;

  return ACK_OK;
}

uint8_t
flash_write(packet_t *packet, packet_t *out_packet, uint16_t *out_len)
{
  const uint32_t addr = get_be24(packet->data.flash_write.addr);

  if ( packet->header.length != sizeof(write_flash_data_t) ) {
    return ACK_LEN_ERR;
  }

  if ( addr + MAX_FLASH_DATA >= CUR_SLOT_ORIGIN && addr < CUR_SLOT_ORIGIN + PHYSICAL_SLOT_SIZE ) {
    return ACK_PARAM_ERR;
  }

  if ( addr >= BOOTLOADER_ORIGIN && addr < SLOT0_ORIGIN ) {
    return ACK_PARAM_ERR;
  }

  hal_flash_write_page(addr, packet->data.flash_write.data, MAX_FLASH_DATA);

  *out_len = 0;

  return ACK_OK;
}

uint8_t
get_running_slot_handle(packet_t *in_packet, packet_t *out_packet, uint16_t *out_len)
{
  if ( in_packet->header.length != 0 ) {
    return ACK_LEN_ERR;
  }

  read_running_slot_resp_t *resp = (read_running_slot_resp_t *)out_packet->data.buf;

  static_assert(CUR_SLOT_ORIGIN == SLOT0_ORIGIN ||
                CUR_SLOT_ORIGIN == SLOT1_ORIGIN ||
                CUR_SLOT_ORIGIN == SLOT2_ORIGIN);

  resp->slot_id = (CUR_SLOT_ORIGIN - SLOT0_ORIGIN)/(512*1024);
  *out_len = 1;

  return ACK_OK;
}

uint8_t
get_slot_version(packet_t *in_packet, packet_t *out_packet, uint16_t *out_len)
{
  if ( in_packet->header.length != 0 ) {
    return ACK_LEN_ERR;
  }

  read_running_slot_resp_t *resp = (read_running_slot_resp_t *)out_packet->data.buf;

  static_assert(CUR_SLOT_ORIGIN == SLOT0_ORIGIN ||
                CUR_SLOT_ORIGIN == SLOT1_ORIGIN ||
                CUR_SLOT_ORIGIN == SLOT2_ORIGIN);

  resp->slot_id = (CUR_SLOT_ORIGIN - SLOT0_ORIGIN)/(512*1024);
  *out_len = 1;

  return ACK_OK;
}

uint8_t
set_active_slot_handle(packet_t *in_packet, packet_t *out_packet, uint16_t *out_len)
{
  if ( in_packet->header.length != sizeof(set_active_slot_t) ) {
    return ACK_LEN_ERR;
  }

  set_active_slot_t *req = (set_active_slot_t *)in_packet->data.buf;

  if (hal_flash_set_active_slot(req->slot_id)) {
    return ACK_PARAM_ERR;
  }

  *out_len=0;

  return ACK_OK;
}

uint8_t
reset_handle(packet_t *in_packet, packet_t *out_packet, uint16_t *out_len)
{
  if ( in_packet->header.length != 0 ) {
    return ACK_LEN_ERR;
  }

  printf("Rebooting pico\n");

  hal_system_reset();

  return ACK_OK;
}

uint8_t
read_sw_version_handle(packet_t *in_packet, packet_t *out_packet, uint16_t *out_len)
{
  if ( in_packet->header.length != 0 ) {
    return ACK_LEN_ERR;
  }

  read_sw_version_resp_t *resp = &(out_packet->data.read_sw_version);

  resp->major = PROJECT_VERSION_MAJOR;
  resp->minor = PROJECT_VERSION_MINOR;
  resp->patch = PROJECT_VERSION_PATCH;

  *out_len = 3;

  return ACK_OK;
}

uint8_t
set_name_handle(packet_t *in_packet, packet_t *out_packet, uint16_t *out_len)
{
  hal_config_t cpy;
  memcpy(&cpy, &shared, sizeof(hal_config_t));

  if (in_packet->header.length > MAX_NAME_LEN) {
    return -1;
  }

  strncpy(cpy.name, in_packet->data.set_name.name, MAX_NAME_LEN);

  hal_flash_erase_page((uint32_t) &shared);

  uint8_t buf[256] = {0};
  memcpy(buf, &cpy, sizeof(hal_config_t));
  hal_flash_write_page((uint32_t) &shared, buf, 256);
}

uint8_t
get_info_handle(packet_t *in_packet, packet_t *out_packet, uint16_t *out_len)
{
  char *last = strncpy(out_packet->data.get_info.name, shared.name, MAX_NAME_LEN);

  if (last == NULL) {
    return ACK_PARAM_ERR;
  }

  hal_system_get_board_id((uint8_t *)&out_packet->data.get_info.uuid);

  out_packet->data.get_info.role = ROLE_WATERER;
  out_packet->data.get_info.ip = hal_network_get_ip_v4();

  *out_len = sizeof(get_info_t);

  return ACK_OK;
}

uint8_t
get_watering_ctx(packet_t *in_packet, packet_t *out_packet, uint16_t *out_len)
{
  out_packet->data.get_ctx.water_lvl = 0xA5;
  out_packet->data.get_ctx.battery_lvl = 0xA5;
  out_packet->data.get_ctx.moisture_lvl = get_moist_lvl();
  out_packet->data.get_ctx.temp_lvl = get_temp_lvl();
  out_packet->data.get_ctx.uptime = (uint32_t)(hal_system_get_time_us() / 1000ULL);

  *out_len = sizeof(get_watering_ctx_t);

  return ACK_OK;
}

uint8_t
trigger_water(packet_t *in_packet, packet_t *out_packet, uint16_t *out_len)
{
  uint32_t time_ms = in_packet->data.trigger_water.time_ms;

  if (time_ms == -1)
  {
    return ACK_PARAM_ERR;
  }

  set_watering_time(time_ms);
  start_watering();

  *out_len = 0;
  return ACK_OK;
}

uint8_t
set_watering_time_cmd(packet_t *in_packet, packet_t *out_packet, uint16_t *out_len)
{
  uint32_t time_ms = in_packet->data.set_water.time_ms;

  if (time_ms == -1)
  {
    return ACK_PARAM_ERR;
  }

  set_watering_time(time_ms);

  *out_len = 0;

  return ACK_OK;
}

uint8_t
set_water_thresh_cmd(packet_t *in_packet, packet_t *out_packet, uint16_t *out_len)
{
  uint32_t time_ms = in_packet->data.set_water.time_ms;

  if (time_ms == -1)
  {
    return ACK_PARAM_ERR;
  }

  set_moist_thresh(time_ms);

  *out_len = 0;

  return ACK_OK;
}

uint8_t
peer_discovery_handle(packet_t *in_packet, packet_t *out_packet, uint16_t *out_len)
{
  if (in_packet->header.length != sizeof(peer_discovery_t)) {
    return ACK_LEN_ERR;
  }

  *out_len = 0;

  if (displayer_ip_received) {
        return ACK_OK; 
    }

  peer_discovery_t *resp = &(in_packet->data.peer_discovery);
  
  if (resp->role == ROLE_DISPLAYER) {
    displayer_ip = resp->ip;
    displayer_ip_received = true;
  }
  return ACK_OK;
}
