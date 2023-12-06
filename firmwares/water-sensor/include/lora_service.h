#ifndef LORA_SERVICE_H
#define LORA_SERVICE_H

#include <stdlib.h>
#include <lora/LoRa.h>
#include "measures.h"

typedef struct lora_svc_config_args {
    double freq;
    bool paboost;
} lora_svc_config_args_t;

response_t lora_svc_config(LoRaClass *loraClient, lora_svc_config_args_t *lora_config, measures_t *measures, measures_config_t *measures_config);
response_t lora_svc_init();
void lora_svc_send_measures();

#endif