#ifndef SENSOR_SERVICE_H
#define SENSOR_SERVICE_H

#include <stdlib.h>
#include <responses.h>

response_t sensor_config(bool *start_token);
response_t sensor_init();

#endif
