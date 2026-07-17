#pragma once

#include <time.h>

#include "proto.h"
#include "lwip/tcp.h"

#include "dispatcher.h"

extern volatile bool displayer_ip_received;
extern ip_addr_t displayer_ip;

// TCP commands
uint8_t get_watering_ctx(packet_t *in_packet, packet_t *out_packet, uint16_t *out_len);

uint8_t flash_write(packet_t *in_packet, packet_t *out_packet, uint16_t *out_len);
uint8_t flash_erase(packet_t *in_packet, packet_t *out_packet, uint16_t *out_len);

uint8_t reset_handle(packet_t *in_packet, packet_t *out_packet, uint16_t *out_len);

uint8_t get_running_slot_handle(packet_t *in_packet, packet_t *out_packet, uint16_t *out_len);
uint8_t set_active_slot_handle(packet_t *in_packet, packet_t *out_packet, uint16_t *out_len);

uint8_t read_sw_version_handle(packet_t *in_packet, packet_t *out_packet, uint16_t *out_len);

uint8_t set_name_handle(packet_t *in_packet, packet_t *out_packet, uint16_t *out_len);
uint8_t get_info_handle(packet_t *in_packet, packet_t *out_packet, uint16_t *out_len);

uint8_t trigger_water(packet_t *in_packet, packet_t *out_packet, uint16_t *out_len);
uint8_t set_watering_time_cmd(packet_t *in_packet, packet_t *out_packet, uint16_t *out_len);
uint8_t set_water_thresh_cmd(packet_t *in_packet, packet_t *out_packet, uint16_t *out_len);

uint8_t peer_discovery_handle(packet_t *in_packet, packet_t *out_packet, uint16_t *out_len);

extern handle_packet dispatch_table[256];
