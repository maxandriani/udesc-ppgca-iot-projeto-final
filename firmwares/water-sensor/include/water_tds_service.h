#ifndef WATER_TDS_SERVICE_H
#define WATER_TDS_SERVICE_H

#include <stdlib.h>
#include <responses.h>
#include <measures.h>

#define TDS_SENSOR_PIN 37
#define TDS_VOLTAGE_REF 3.3
#define TDS_ADC_RANGE 4096
#define TDS_TEMPERATURE_REFERENCE 25.0

response_t water_tds_svc_config(measures_t *measures, tds_config_t *water_tds_config);
response_t water_tds_svc_init();
void water_tds_measure();

#endif