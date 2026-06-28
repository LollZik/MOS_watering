#include <string.h>
#include <stdio.h>

#include "hardware/sync.h"
#include "hardware/flash.h"
#include "shared_mem.h"

shared_mem_t __attribute__((section(".shared")))shared = {
  0,
  0x8000,
  0x0200,
  "None",
};

shared_mem_t ram_shared = {
  0,
  0x8000,
  0x0200,
  "None",
};

uint8_t
get_running_slot_id(void)
{
  return (shared.running_slot_id);
}

uint8_t *
get_name(void)
{
  return (shared.name);
}

void load_config_from_flash(void) {
    const uint8_t *flash_ptr = (const uint8_t *)(&shared);
    //memcpy(&ram_shared, flash_ptr, sizeof(shared_mem_t));
}

void save_config_to_flash(void) {
    printf("Saving to flash not working - to be fixed\n");
    //printf("Disabling irq's\n");
    //uint32_t ints = save_and_disable_interrupts();
    //printf("Disabled irq's\n");

    //flash_range_erase((uint32_t) &shared, FLASH_SECTOR_SIZE);
    //flash_range_program((uint32_t) &shared, (const uint8_t *)&ram_shared, sizeof(shared_mem_t));

    //restore_interrupts(ints);
}
