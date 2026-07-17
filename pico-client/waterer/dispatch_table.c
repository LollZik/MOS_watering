#include <string.h>

#include "pico/time.h"
#include "pico/unique_id.h"
#include "pico/cyw43_arch.h"

#include "hardware/flash.h"
#include "hardware/sync.h"
#include "shared_mem.h"

#include "dispatch_table.h"
#include "dispatcher.h"

#include "water_ctx.h"

#include "sched.h"
#include "reset.h"

#define SLOT_SIZE (SLOT1_ORIGIN - SLOT0_ORIGIN)

extern void w_get_watering_stats(get_watering_ctx_t *ctx);

volatile bool displayer_ip_received = false;
ip_addr_t displayer_ip = {0};


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

static inline int
set_running_slot(uint8_t slot_id)
{
  shared_mem_t cpy;
  memcpy(&cpy, &shared, sizeof(shared_mem_t));

  if (slot_id > 1) {
    return -1;
  }

  uint32_t ints = save_and_disable_interrupts();

  flash_range_erase((uint32_t)&shared, FLASH_PAGE_SIZE);

  cpy.running_slot_id = slot_id;
  memcpy(&shared, &cpy, sizeof(shared_mem_t));

  flash_range_program((uint32_t) &shared, (const uint8_t *)&cpy, FLASH_PAGE_SIZE);

  restore_interrupts(ints);

  return 0;
}

uint8_t
flash_erase(packet_t *packet, packet_t *out_packet, uint16_t *out_len)
{
  const uint32_t addr = get_be24(packet->data.flash_erase.addr);

  if ( packet->header.length != sizeof(erase_flash_data_t) ) {
    return ACK_LEN_ERR;
  }

  if ( addr + FLASH_PAGE_SIZE > CUR_SLOT_ORIGIN && addr < CUR_SLOT_ORIGIN + SLOT_SIZE ) {
    return ACK_PARAM_ERR;
  }

  if ( addr >= BOOTLOADER_ORIGIN && addr < SLOT0_ORIGIN ) {
    return ACK_PARAM_ERR;
  }

  if ( addr % FLASH_PAGE_SIZE != 0) {
    return ACK_PARAM_ERR;
  }
  uint32_t ints = save_and_disable_interrupts();

  flash_range_erase(addr, FLASH_PAGE_SIZE);
  
  restore_interrupts(ints);

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

  if ( addr + MAX_FLASH_DATA >= CUR_SLOT_ORIGIN && addr < CUR_SLOT_ORIGIN + SLOT_SIZE ) {
    return ACK_PARAM_ERR;
  }

  if ( addr >= BOOTLOADER_ORIGIN && addr < SLOT0_ORIGIN ) {
    return ACK_PARAM_ERR;
  }

  uint32_t ints = save_and_disable_interrupts();

  flash_range_program(addr, packet->data.flash_write.data, MAX_FLASH_DATA);
  
  restore_interrupts(ints);

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

  if (set_running_slot(req->slot_id)) {
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

  reset_pico();

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
  shared_mem_t cpy;
  memcpy(&cpy, &shared, sizeof(shared_mem_t));

  if (in_packet->header.length > MAX_NAME_LEN) {
    return -1;
  }

  strncpy(cpy.name, in_packet->data.set_name.name, MAX_NAME_LEN);

  uint32_t ints = save_and_disable_interrupts();

  flash_range_erase((uint32_t) &shared, FLASH_PAGE_SIZE);
  flash_range_program((uint32_t) &shared, (const uint8_t *)&cpy, FLASH_PAGE_SIZE);

  restore_interrupts(ints);
  
  return ACK_OK;
}

uint8_t
get_info_handle(packet_t *in_packet, packet_t *out_packet, uint16_t *out_len)
{
  char *last = strncpy(out_packet->data.get_info.name, shared.name, MAX_NAME_LEN);

  if (last == NULL) {
    return ACK_PARAM_ERR;
  }

  pico_get_unique_board_id((pico_unique_board_id_t *) &out_packet->data.get_info.uuid);
  
  out_packet->data.get_info.role = ROLE_WATERER;

  struct netif *n = &cyw43_state.netif[CYW43_ITF_STA];
  out_packet->data.get_info.ip = netif_ip4_addr(n)->addr;
  
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
  out_packet->data.get_ctx.uptime = to_ms_since_boot(get_absolute_time());

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
    ip_addr_set_ip4_u32(&displayer_ip, resp->ip);
    
    displayer_ip_received = true;
  }
  return ACK_OK;
}
