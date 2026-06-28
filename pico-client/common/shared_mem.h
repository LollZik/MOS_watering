#pragma once

#include "helpers.h"
#include "proto.h"

// Keep in mind, that for backwards compability, it is NECESSARY
// to append new fields to the end of this struct

#pragma pack(push,1)

typedef struct shared_data {
  uint8_t running_slot_id;
  uint16_t moist_thresh;
  uint16_t watering_time;
  uint8_t name[MAX_NAME_LEN];
} shared_mem_t;

#pragma pack(pop)

extern shared_mem_t __attribute__((section(".shared")))shared;
extern shared_mem_t ram_shared;

uint8_t get_running_slot_id(void);
uint8_t *get_name(void);

void load_config_from_flash(void);
void save_config_to_flash(void);
