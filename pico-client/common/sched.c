#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "sched.h"

#include "hardware/timer.h"
#include "hardware/irq.h"
#include "pico/sync.h"

#define MIN_SCHED_TIMEOUT_MS 10000

task_ctx_t *tasks[MAX_TASKS];

static uint32_t task_count = 0;
volatile bool should_wake_up = false;

static void
alarm_sleep_callback(uint alarm_id)
{
  hardware_alarm_set_callback(alarm_id, NULL);
  hardware_alarm_unclaim(alarm_id);

  should_wake_up = true;
}

static void 
sched_go_sleep_until(absolute_time_t deadline)
{
  absolute_time_t now = get_absolute_time();
  if (deadline <= now) {
    return;
  }

  int alarm_num = hardware_alarm_claim_unused(true);
  if (alarm_num >= 0) {
    hardware_alarm_set_target(alarm_num, deadline);

    while(!should_wake_up)
    {
      __wfe();
    }

    hardware_alarm_unclaim(alarm_num);
  } else {
    while (get_absolute_time() < deadline) {
      tight_loop_contents();
    }
  }
}

void
enable_task(task_ctx_t *task) 
{
  if (task) {
    task->task_en = TASK_ENABLED;
    task->deadline = get_absolute_time();
    should_wake_up = true;
  }
}

uint32_t
get_task_count(void) 
{
  return task_count;
}

int
add_task(task_ctx_t *task)
{
  if (task_count >= MAX_TASKS || task == NULL) {
    return -1;
  }

  tasks[task_count] = task;
  task_count++;

  return 0;
}

static void
init_irq(void)
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

void
__run_sched(void)
{
  init_irq();
  for (uint32_t i = 0; i < get_task_count(); i++) {
    tasks[i]->deadline = get_absolute_time();
  }

  for (uint32_t i = 0; i < task_count; i++) {
    if (tasks[i]->init != NULL) {
      tasks[i]->init();
    }
  }

  while (true) {
    absolute_time_t min_deadline = delayed_by_ms(get_absolute_time(), MIN_SCHED_TIMEOUT_MS);

    for (uint32_t i = 0; i < get_task_count(); i++) {
      task_ctx_t *t = tasks[i];

      if (t->task_en != TASK_ENABLED) {
        continue;
      }

      const absolute_time_t cur_time = get_absolute_time();

      if (cur_time >= t->deadline) {
        const int ret = t->realise();

        if (ret > 0) {
          t->timeout_ms = (uint32_t)ret;
        } else if (ret == -1) {
          disable_task(t);
        }
        t->deadline = make_timeout_time_ms(t->timeout_ms);
      }

      if (t->deadline < min_deadline) {
        min_deadline = t->deadline;
      }
    }

    sched_go_sleep_until(min_deadline);
  }
}