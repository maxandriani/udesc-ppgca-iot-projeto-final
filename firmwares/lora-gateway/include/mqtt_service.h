#ifndef MQTT_SERVICE_H
#define MQTT_SERVICE_H

#include <stdlib.h>
#include <responses.h>
#include <oled/SSD1306Wire.h>
#include <display.h>
#include <logs.h>
#include <AsyncMqttClient.h>
#include "devices.h"

typedef struct wifi_vsc_config 
{
    char *ssid;
    char *passwd;
} wifi_svc_config_t;

typedef struct mqtt_svc_config {
    char *server;
    int port;
    char *user;
    char *passwd;
    char *client_id;
} mqtt_svc_config_t;

typedef struct mqtt_svc_config_args {
    wifi_svc_config_t wifi;
    mqtt_svc_config_t mqtt;
} mqtt_svc_config_args_t;

response_t mqtt_svc_config(AsyncMqttClient *mqttClient, mqtt_svc_config_args_t *config, device_list_t *devices);
response_t mqtt_svc_init();

#endif