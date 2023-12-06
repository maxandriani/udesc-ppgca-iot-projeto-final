#ifndef WATER_TEMP_SERVICE_H
#define WATER_TEMP_SERVICE_H

#include <stdlib.h>
#include <responses.h>
#include <measures.h>

#define TEMP_PIN_INPUT 38

response_t water_temp_svc_config(measures_t *measures, temp_config_t *water_temp_config);
response_t water_temp_svc_init();
void water_temp_measure();

#endif