#include <Arduino.h>
#include <stdlib.h>
#include <responses.h>
#include <measures.h>
#include "water_level_service.h"
#include <logs.h>
#include <display.h>

static volatile measures_t *_measures;
static volatile level_config_t *_config;

response_t water_level_svc_config(measures_t *measures, level_config_t *water_level_config) {
    _measures = measures;
    _config = water_level_config;
    log_infoln("Water level sensors configured.");
    return Success;
}

response_t water_level_svc_init() {
    pinMode(WATER_LEVEL_TRIGGER_PIN, OUTPUT);
    pinMode(WATER_LEVEL_ECHO_PIN, INPUT);
    log_infoln("Water level sensors initialized.");
    return Success;
}

void water_level_measure() {
    long duration;
    double distance;
    _measures->level.value = 0;

    log_infoln("Starting water level measurement.");

    for (int i = 0; i < _config->sample_size; i++) {
        // envia sinal para o módulo
        digitalWrite(WATER_LEVEL_TRIGGER_PIN, LOW);
        delayMicroseconds(2);
        digitalWrite(WATER_LEVEL_TRIGGER_PIN, HIGH);
        delayMicroseconds(10);
        digitalWrite(WATER_LEVEL_TRIGGER_PIN, LOW);

        duration = pulseIn(WATER_LEVEL_ECHO_PIN, HIGH);

        // ida e volta, portanto dividindo por 2 vc tem a distância da ida.
        distance = duration * WATER_LEVEL_SOUND_SPEED_CM / 2;
        // média incremental
        _measures->level.value = _measures->level.value + ((distance - _measures->level.value) / (i + 1));
    }

    _measures->level.status = Success;
    log_infoln("Finished water level measurement. %2.f cm", _measures->level.value);
    display_sprint_msg("Water level: %2.f cm", _measures->level.value);
}
