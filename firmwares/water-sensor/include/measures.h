#ifndef MEASURES_H
#define MEASURES_H

#include <stdlib.h>
#include <responses.h>

typedef struct measure {
    double value;
    response_t status;
} measure_t;

typedef struct measures
{
    measure_t level;
    measure_t temp;
    measure_t tds;
} measures_t;

typedef struct level_config {
    double max_level;
    uint8_t sample_size; // quantidade de medidas da amostra.
} level_config_t;

typedef struct tds_config {
    uint8_t sample_size; // quantidade de medidas da amosta.
} tds_config_t;

typedef struct temp_config {

} temp_config_t;

typedef struct measures_config {
    level_config_t level;
    tds_config_t tds;
    temp_config_t temp;
} measures_config_t;

void measures_to_json_str(measures_t *measures, char *buffer, size_t size);
void measures_config_from_json_str(measures_config *config, char *json);

#endif