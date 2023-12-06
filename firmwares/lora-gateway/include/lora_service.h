#ifndef LORA_SERVICE_H
#define LORA_SERVICE_H

#include <stdlib.h>
#include <responses.h>
#include <oled/SSD1306Wire.h>
#include <lora/LoRa.h>
#include "devices.h"

typedef struct lora_svc_config_args {
    double freq;
    bool paboost;
} lora_svc_config_args_t;

response_t lora_svc_config(LoRaClass *loraClient, lora_svc_config_args_t *config, device_list_t *devices);
response_t lora_svc_init();

#endif