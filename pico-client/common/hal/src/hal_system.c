#include "hal_system.h"
#include "hardware/watchdog.h"
#include "hardware/timer.h"
#include "hardware/irq.h"
#include "pico/time.h"
#include "pico/unique_id.h"
#include "pico/sync.h"
#include "pico/stdlib.h"

extern volatile bool should_wake_up;

void
hal_system_reset(void)
{
    watchdog_reboot(0, 0, 0);

    while (true) {
      tight_loop_contents();
    }

    assert(false); // We should never be here
}

void
hal_system_watchdog_init(uint32_t timeout_ms)
{
    watchdog_enable(timeout_ms, true);
}

void
hal_system_watchdog_feed(void)
{
    watchdog_update();
}

void
hal_system_irq_init(void)
{
    const uint32_t mask = 0x00000000 | (
          (1 << 11) |               // DMA_IRQ_0    enabled
          (1 << 12) |               // DMA_IRQ_1    enabled
          (1 << 13) |               // IO_IRQ_BANK0 enabled
          (1 << 14) |               // IO_IRQ_QSPI  enabled
          (1 << 25)                 // RTC_IRQ      enabled
        );

  irq_set_mask_enabled(mask, true);
}

uint64_t
hal_system_get_time_us(void)
{
    return to_us_since_boot(get_absolute_time());
}

static void
alarm_sleep_callback(uint alarm_id)
{
    hardware_alarm_set_callback(alarm_id, NULL);
    hardware_alarm_unclaim(alarm_id);
    should_wake_up = true;
}

void
hal_system_sleep_until_us(uint64_t deadline_us)
{
    absolute_time_t deadline = from_us_since_boot(deadline_us);

    if (deadline <= get_absolute_time()) {
        return;
    }

    int alarm_num = hardware_alarm_claim_unused(true);
    if (alarm_num >= 0) {
        hardware_alarm_set_callback(alarm_num, alarm_sleep_callback);
        hardware_alarm_set_target(alarm_num, deadline);

        while(!should_wake_up)
        {
            __wfe();
        }
        should_wake_up = false;

        hardware_alarm_unclaim(alarm_num);
    } else {
        while (get_absolute_time() < deadline) {
            tight_loop_contents();
        }
    }
}

void
hal_system_tight_loop(void)
{
    tight_loop_contents();
}

void
hal_system_get_board_id(uint8_t *out_bytes)
{
    pico_get_unique_board_id((pico_unique_board_id_t *)out_bytes);
}

void hal_system_init(void) {
    stdio_init_all();
}