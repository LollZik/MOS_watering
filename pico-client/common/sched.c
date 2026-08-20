#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "sched.h"
#include "hal_system.h"

#define MIN_SCHED_TIMEOUT_MS 10000

task_ctx_t *tasks[MAX_TASKS];

static uint32_t task_count = 0;
volatile bool should_wake_up = false;


void
enable_task(task_ctx_t *task) 
{
  if (task) {
    task->task_en = TASK_ENABLED;
    task->deadline = hal_system_get_time_us();
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

void
__run_sched(void)
{
  hal_system_irq_init();
  for (uint32_t i = 0; i < get_task_count(); i++) {
    tasks[i]->deadline = hal_system_get_time_us();
  }

  for (uint32_t i = 0; i < task_count; i++) {
    if (tasks[i]->init != NULL) {
      tasks[i]->init();
    }
  }

  while (true) {
    uint64_t min_deadline = hal_system_get_time_us() + (MIN_SCHED_TIMEOUT_MS * 1000);

    for (uint32_t i = 0; i < get_task_count(); i++) {
      task_ctx_t *t = tasks[i];

      if (t->task_en != TASK_ENABLED) {
        continue;
      }

      const uint64_t cur_time = hal_system_get_time_us();

      if (cur_time >= t->deadline) {
        const int ret = t->realise();

        if (ret > 0) {
          t->timeout_ms = (uint32_t)ret;
        } else if (ret == -1) {
          disable_task(t);
        }
        t->deadline = hal_system_get_time_us() + (t->timeout_ms * 1000);
      }

      if (t->deadline < min_deadline) {
        min_deadline = t->deadline;
      }
    }

    hal_system_sleep_until_us(min_deadline);
  }
}