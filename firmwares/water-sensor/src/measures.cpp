#include <stdlib.h>
#include <stdio.h>
#include "measures.h"
#include <ArduinoJson.h>
#include <logs.h>

void measures_to_json_str(measures_t *measures, char *buffer, size_t size) {
    snprintf(buffer, size, "{\"temp\":%.2f, \"tds\":%.2f, \"level\": %.2f}", measures->temp.value, measures->tds.value, measures->level.value);
}

void measures_config_from_json_str(measures_config *config, char *json) {
    // { "level": { "max_level": 22.9, "sample_size": 30 }, "tds": { "sample_size": 30 }, "temp": {} }
    StaticJsonDocument<1024> doc;

    // Deserialize the JSON document
    DeserializationError error = deserializeJson(doc, json);
    if (error)
        log_infoln("Failed serialize json");

    config->level.max_level = doc["level"]["max_level"] | 20;
    config->level.sample_size = doc["level"]["sample_size"] | 30;
    config->tds.sample_size = doc["tds"]["sample_size"] | 30;
}