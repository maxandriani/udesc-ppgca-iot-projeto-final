#include <stdlib.h>
#include <responses.h>
#include "sensor_service.h"
#include "water_level_service.h"
#include "water_tds_service.h"
#include "water_temp_service.h"
#include "lora_service.h"
#include <freertos/task.h>
#include <leds.h>
#include <logs.h>
#include <display.h>

static bool *_start_token;
static TaskHandle_t _sensorTaskHandle;

void measure_sensors(void *args) {
    for (;;) {
        if ((*_start_token) == false){
            vTaskDelay(pdMS_TO_TICKS(10000));
            continue;
        }

        led_up();
        log_infoln("Collect sensor data.");
        water_temp_measure();
        water_tds_measure();
        water_level_measure();
        led_start_blink();
        log_infoln("Transmitting sensor data.");
        lora_svc_send_measures();
        led_stop_blink();
        log_infoln("Sensor collect done.");
        display_sprint_msg("Temp: %2.f C\nTDS: %2.f ppm\nLevel: %2.f cm");

        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}

response_t sensor_config(bool *start_token) {
    _start_token = start_token;
    return Success;
}

response_t sensor_init() {
    log_infoln("Scheduling Sensor Monitor");
    xTaskCreatePinnedToCore(measure_sensors, "Measure sensors", 4096, NULL, 1, &_sensorTaskHandle, ARDUINO_RUNNING_CORE);
    log_infoln("Sensor Monitor is Running");
    return Success;
}
