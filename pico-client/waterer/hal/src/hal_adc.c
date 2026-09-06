#include "hal_adc.h"
#include "hardware/adc.h"

void hal_adc_init(void) {
    adc_init();
}

void hal_adc_setup_pin(uint8_t pin) {
    adc_gpio_init(pin);
}

uint16_t hal_adc_read_channel(uint8_t channel) {
    adc_select_input(channel);
    return adc_read();
}