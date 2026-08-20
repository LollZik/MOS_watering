#include "hal_memory.h"
#include "hardware/flash.h"
#include "hardware/sync.h"
#include <stdio.h>

hal_config_t __attribute__((section(".shared"))) shared = {
  0,
  0x8000,
  0x0200,
  "None"
};

hal_config_t ram_shared = {
  0,
  0x8000,
  0x0200,
  "None"
};

watering_slot_t *slots[3] = {
  (watering_slot_t *)SLOT0_ORIGIN,
  (watering_slot_t *)SLOT1_ORIGIN,
  (watering_slot_t *)SLOT2_ORIGIN
};

uint8_t
hal_memory_get_running_slot_id(void)
{
    return shared.running_slot_id;
}

uint8_t *
hal_memory_get_name(void)
{
    return shared.name;
}

void
hal_memory_load_config(hal_config_t *out_config)
{
    const uint8_t *flash_ptr = (const uint8_t *)(&shared);
    //memcpy(&ram_shared, flash_ptr, sizeof(shared_mem_t));
}

void
hal_memory_save_config(const hal_config_t *in_config)
{
    printf("Saving to flash not working - to be fixed\n");
    //printf("Disabling irq's\n");
    //uint32_t ints = save_and_disable_interrupts();
    //printf("Disabled irq's\n");

    //flash_range_erase((uint32_t) &shared, FLASH_SECTOR_SIZE);
    //flash_range_program((uint32_t) &shared, (const uint8_t *)&ram_shared, sizeof(shared_mem_t));

    //restore_interrupts(ints);
}
