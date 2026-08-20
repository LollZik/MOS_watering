#pragma once
#include <stdint.h>
#include "proto.h"

#define CRC_SIZE sizeof(uint32_t)
#define SLOT_SIZE (512*1024 - CRC_SIZE)

// Keep in mind, that for backwards compability, it is NECESSARY
// to append new fields to the end of this struct

#pragma pack(push,1)
typedef struct {
  uint8_t running_slot_id;
  uint16_t moist_thresh;
  uint16_t watering_time;
  uint8_t name[MAX_NAME_LEN]; 
} hal_config_t;
#pragma pack(pop)

typedef struct watering_slot_t {
  uint8_t data[SLOT_SIZE];
  uint32_t crc;
} watering_slot_t;

extern watering_slot_t *slots[3];

extern hal_config_t shared;
extern hal_config_t ram_shared;

uint8_t hal_memory_get_running_slot_id(void);
uint8_t *hal_memory_get_name(void);

void hal_memory_load_config(hal_config_t *out_config);
void hal_memory_save_config(const hal_config_t *in_config);
