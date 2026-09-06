#pragma once
#include <stdint.h>

int hal_flash_set_active_slot(uint8_t slot_id);
void hal_flash_erase_page(uint32_t addr);
void hal_flash_write_page(uint32_t addr, const uint8_t *data, uint32_t len);