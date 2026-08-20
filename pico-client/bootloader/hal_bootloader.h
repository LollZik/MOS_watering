#pragma once

void hal_crc_init(void);
void hal_crc_deinit(void);

void hal_bootloader_jump_to_app(const void *app_start_address);
void hal_bootloader_tight_loop(void);