#ifndef WATER_LEVEL_SERVICE_H
#define WATER_LEVEL_SERVICE_H

#include <stdlib.h>
#include <responses.h>
#include <measures.h>

#define WATER_LEVEL_TRIGGER_PIN 12
#define WATER_LEVEL_ECHO_PIN 36
#define WATER_LEVEL_SOUND_SPEED_CM 0.0343

response_t water_level_svc_config(measures_t *measures, level_config_t *water_level_config);
response_t water_level_svc_init();
void water_level_measure();

#endif