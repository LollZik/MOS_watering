#include "hal_flash.h"
#include "hal_memory.h"
#include "hardware/sync.h"
#include "hardware/flash.h"
#include <string.h>

int hal_flash_set_active_slot(uint8_t slot_id)
{
    hal_config_t cpy;
    memcpy(&cpy, &shared, sizeof(hal_config_t));
    
    if (slot_id > 1) {
        return -1;
    }
    
    uint32_t ints = save_and_disable_interrupts();
    flash_range_erase((uint32_t)&shared, FLASH_PAGE_SIZE);
    
    cpy.running_slot_id = slot_id;
    memcpy(&shared, &cpy, sizeof(hal_config_t));
    
    flash_range_program((uint32_t)&shared, (const uint8_t *)&cpy, FLASH_PAGE_SIZE);
    restore_interrupts(ints);
    
    return 0;
}

void hal_flash_erase_page(uint32_t addr)
{
    uint32_t ints = save_and_disable_interrupts();
    flash_range_erase(addr, FLASH_PAGE_SIZE);
    restore_interrupts(ints);
}

void hal_flash_write_page(uint32_t addr, const uint8_t *data, uint32_t len)
{
    uint32_t ints = save_and_disable_interrupts();
    flash_range_program(addr, data, len);
    restore_interrupts(ints);
}