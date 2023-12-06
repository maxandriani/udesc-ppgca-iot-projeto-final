#include <Arduino.h>
#include <stdlib.h>
#include <responses.h>
#include <measures.h>
#include "water_tds_service.h"
#include <logs.h>
#include <display.h>
#include <math.h>

static measures_t *_measures;
static tds_config_t *_config;

response_t water_tds_svc_config(measures_t *measures, tds_config_t *water_tds_config) {
    _measures = measures;
    _config = water_tds_config;
    log_infoln("Water TDS sensors configured.");
    return Success;
}

response_t water_tds_svc_init() {
    pinMode(TDS_SENSOR_PIN, INPUT);
    log_infoln("Water TDS sensors initialized.");
    return Success;
}

int compare(const void *a, const void *b) {
    return (*(int*)a - *(int*)b);
}

int32_t _calc_median(int32_t items[], int32_t size) {
    qsort(items, size, sizeof(int32_t), compare);
    return items[size / 2];
}

void water_tds_measure() {
    int32_t readingBuffer[_config->sample_size];
    int32_t averageVoltage;

    log_infoln("Starting water TDS measurement.");

    for (int i = 0; i < _config->sample_size; i++) {
        readingBuffer[i] = analogRead(TDS_SENSOR_PIN);
        vTaskDelay(40 / portTICK_PERIOD_MS);
    }

    // read the analog value more stable by the median, and convert to voltage value
    averageVoltage = _calc_median(readingBuffer, _config->sample_size) * TDS_VOLTAGE_REF / TDS_ADC_RANGE;
    // water temperature has influence over electric current, so we need to compensate
    // temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0));
    double compensationCoefficient = 1.0 + 0.02 * (_measures->temp.value - TDS_TEMPERATURE_REFERENCE);
    double compensationVoltage = averageVoltage / compensationCoefficient;
    // calibração do sensor segundo o fabricante.
    _measures->tds.value = (133.42 * compensationVoltage * compensationVoltage * compensationVoltage - 255.86 * compensationVoltage * compensationVoltage + 857.39 * compensationVoltage) * 0.5;

    _measures->tds.status = Success;
    log_infoln("Finished water TDS measurement. %2.f ppm", _measures->tds.value);
    display_sprint_msg("Water TDS: %2.f ppm", _measures->tds.value);
}