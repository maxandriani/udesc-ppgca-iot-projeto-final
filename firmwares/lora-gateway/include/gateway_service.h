#ifndef GATEWAY_SERVICE_H
#define GATEWAY_SERVICE_H

#include <stdlib.h>
#include <oled/SSD1306Wire.h>
#include <logs.h>
#include <display.h>
#include <lora/LoRa.h>
#include <AsyncMqttClient.h>
#include <responses.h>
#include <devices.h>

response_t gateway_svc_config(LoRaClass *loraClient, AsyncMqttClient *mqttClient, device_list_t *devices);

#endif