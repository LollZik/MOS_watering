#pragma once

#define TASK_DISABLED 0
#define TASK_ENABLED  1

#define MAX_TASKS 16

#include "hardware/sync.h"
#include "pico/time.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef int (*task_fn)(void);
typedef int (*task_init_fn)(void);

typedef struct task_ctx {
  absolute_time_t deadline;
  uint32_t timeout_ms;

  const char *name;

  task_fn realise;
  task_init_fn init;

  volatile uint8_t task_en;
} task_ctx_t;

extern task_ctx_t *tasks[MAX_TASKS];
extern volatile bool should_wake_up;

static inline void
disable_task(task_ctx_t *task) 
{
  if (task) {
    task->task_en = TASK_DISABLED;
  }
}

void enable_task(task_ctx_t *task);

void __run_sched(void);
int add_task(task_ctx_t *task);
uint32_t get_task_count(void);

#define REGISTER_TASK(_name, _timeout_ms, _task_fn, _init_fn, _task_en) \
  task_ctx_t _task_fn##_ctx = { \
    .timeout_ms = _timeout_ms, \
    .name = _name, \
    .realise = _task_fn, \
    .init = _init_fn, \
    .task_en = _task_en \
  }; \
  static void __attribute__((constructor)) _register_##_task_fn(void) { \
    add_task(&_task_fn##_ctx); \
  }

