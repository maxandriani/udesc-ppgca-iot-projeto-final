#include <stdlib.h>
#include <responses.h>
#include <measures.h>
#include <water_tds_service.h>
#include "water_temp_service.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include <logs.h>
#include <display.h>

static OneWire oneWire(TEMP_PIN_INPUT);
static DallasTemperature sensor(&oneWire);
static measures_t *_measures;
static temp_config_t *_config;

response_t water_temp_svc_config(measures_t *measures, temp_config_t *water_temp_config) {
    _measures = measures;
    _config = water_temp_config;
    log_infoln("Water temp sensors configured.");
    return Success;
}

response_t water_temp_svc_init() {
    log_infoln("Water temp sensors initialized.");
    pinMode(TEMP_PIN_INPUT, INPUT);
    sensor.begin();
    return Success;
}

void water_temp_measure() {
    log_infoln("Starting water temp measurement.");

    sensor.requestTemperatures();
    _measures->temp.value = sensor.getTempCByIndex(0);

    _measures->temp.status = Success;
    log_infoln("Finished water temp measurement. %2.f Cº", _measures->temp.value);
    display_sprint_msg("Water temp: %2.f Cº", _measures->temp.value);
}