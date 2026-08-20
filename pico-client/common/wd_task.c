#include <stdio.h>

#include "sched.h"
#include "hal_system.h"

#define WATCHDOG_TIMEOUT_MS 8000
#define WATCHDOG_UPDATE_MS  4000

int
wd_task(void)
{
  hal_system_watchdog_feed();

  return WATCHDOG_UPDATE_MS;
}

int
wd_init(void)
{
  hal_system_watchdog_init(WATCHDOG_TIMEOUT_MS);

  return 0;
}

REGISTER_TASK("Watchdog task", 4000, wd_task, wd_init, true);

