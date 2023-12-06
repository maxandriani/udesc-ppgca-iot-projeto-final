#include <Arduino.h>
#include <stdlib.h>
#include <board.h>
#include "leds.h"

static volatile uint8_t BLINK_DELAY = 300u;
static volatile uint8_t BLINK_STATUS = 0;
static volatile uint8_t PANIC_STATUS = 0;

static TaskHandle_t panicTask;
static TaskHandle_t blinkTask;

static void _leds_panic(void *args) {
    while (PANIC_STATUS == 1) {
        led_up_and_wait(300u);
        led_down_and_wait(300u);
        led_up_and_wait(300u);
        led_down_and_wait(300u);
        led_up_and_wait(300u);
        led_down_and_wait(300u);

        led_up_and_wait(1000u);
        led_down_and_wait(1000u);
        led_up_and_wait(1000u);
        led_down_and_wait(1000u);
        led_up_and_wait(1000u);
        led_down_and_wait(1000u);

        led_up_and_wait(300u);
        led_down_and_wait(300u);
        led_up_and_wait(300u);
        led_down_and_wait(300u);
        led_up_and_wait(300u);
        led_down_and_wait(300u);

        delay(3000u);

        yield();
    }
}

static void _leds_blink(void *args) {
    while (BLINK_STATUS == 1) {
        led_up_and_wait(BLINK_DELAY);
        led_down_and_wait(BLINK_DELAY);
        yield();
    }
}

void leds_config() {
    pinMode(BOARD_LED, OUTPUT);
}

void led_up() {
    digitalWrite(BOARD_LED, HIGH);
}

void led_down() {
    digitalWrite(BOARD_LED, LOW);
}

void led_up_and_wait(uint32_t seconds) {
    led_up();
    delay(seconds);
}

void led_down_and_wait(uint32_t seconds) {
    led_down();
    delay(seconds);
}

void led_start_blink() {
    led_start_blink(300u);
}

void led_start_blink(uint8_t delay) {
    BLINK_DELAY = delay;
    BLINK_STATUS = 1;
    PANIC_STATUS = 0;
    xTaskCreate(_leds_blink, "Blink Loop", 1024, NULL, 1, &blinkTask);
}

void led_stop_blink() {
    BLINK_STATUS = 0;
    PANIC_STATUS = 0;
}

void led_start_panic() {
    BLINK_STATUS = 0;
    PANIC_STATUS = 1;
    xTaskCreate(_leds_panic, "Panic Loop", 1024, NULL, 1, &panicTask);
}

void led_stop_panic() {
    BLINK_STATUS = 0;
    PANIC_STATUS = 0;
}