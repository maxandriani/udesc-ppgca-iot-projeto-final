#ifndef LEDS_H
#define LEDS_H

#include <stdlib.h>
#include <responses.h>

void leds_config();
void led_up();
void led_up_and_wait(uint32_t seconds);
void led_down();
void led_down_and_wait(uint32_t seconds);
void led_start_blink();
void led_start_blink(uint8_t delay);
void led_stop_blink();
void led_start_panic();
void led_stop_panic();

#endif