#include <stdbool.h>

#include "hal_memory.h"
#include "hal_crc.h"

bool
validate_crc(watering_slot_t *slot)
{
  const uint32_t exp_crc = hal_crc32(slot->data, SLOT_SIZE);

  return (slot->crc == exp_crc);
}

int main();

//****************************************************************************
// This is normally provided as part of pico_stdlib so we have to provide it
// here if not we're not using it.
void exit(int ret)
{
  (void)ret;
  while (true){
    hal_bootloader_tight_loop();
  }
  main();
}

//****************************************************************************
// Replace the standard 'atexit' with an empty version to avoid pulling in
// additional code that we don't need anyway.
int atexit(void *a, void (*f)(void*), void *d)
{
  (void)a;
  (void)f;
  (void)d;
  return 0;
}

int main(void)
{
  hal_crc_init();

  const uint8_t slot_id = hal_memory_get_running_slot_id();

  if (validate_crc(slots[slot_id])) {
    hal_crc_deinit();
    hal_bootloader_jump_to_app(slots[slot_id]);
  }

  return 0;
}
