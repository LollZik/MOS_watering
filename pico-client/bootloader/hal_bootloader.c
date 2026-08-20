#include <stdint.h>

#include "hardware/resets.h"
#include "hardware/regs/m0plus.h"
#include "RP2040.h"

#include "hal_bootloader.h"

void
hal_crc_init(void)
{
    unreset_block_wait(RESETS_RESET_DMA_BITS);
}

void
hal_crc_deinit(void)
{
    reset_block(RESETS_RESET_DMA_BITS);
}

void
hal_bootloader_jump_to_app(const void *app_start_address)
{
    uint32_t start_addr = (uint32_t)app_start_address + 0x100;

    asm volatile (
        "mov r0, %[start]\n"
        "ldr r1, =%[vtable]\n"
        "str r0, [r1]\n"
        "ldmia r0, {r0, r1}\n"
        "msr msp, r0\n"
        "bx r1\n"
        :
        : [start] "r" (start_addr), [vtable] "X" (PPB_BASE + M0PLUS_VTOR_OFFSET)
        :
    );
}

void
hal_bootloader_tight_loop(void)
{
    __asm volatile("wfe");
}