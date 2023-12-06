#include <Arduino.h>
#include <stdlib.h>
#include "mqtt_service.h"
#include <AsyncMqttClient.h>
#include <oled/OLEDDisplay.h>
#include <WiFi.h>
#include <freertos/timers.h>
#include "devices.h"

static TaskHandle_t _mqttTaskHandle;
static TaskHandle_t _wifiTaskHandle;
static mqtt_svc_config_args_t *_config;
static AsyncMqttClient *_mqttClient;
static volatile device_list_t *_devices;

void connect_to_wifi(void *args) {
    extern WiFiClass WiFi;

    for (;;) {
        if (WiFi.isConnected()) {
            vTaskDelay(2000 / portTICK_PERIOD_MS);
            continue;
        }

        log_infoln("Connecting to WiFi\n%s, %s", _config->wifi.ssid, _config->wifi.passwd);
        display_sprint_msg("Connecting to WiFi\n%s, %s", _config->wifi.ssid, _config->wifi.passwd);
        WiFi.begin(_config->wifi.ssid, _config->wifi.passwd);
        while (WiFi.status() != WL_CONNECTED)
        {
            vTaskDelay(2000 / portTICK_PERIOD_MS);
        }
    }
}

void connect_to_mqtt() {
    log_infoln("Connecting to MQTT %s on %d", _config->mqtt.server, _config->mqtt.port);
    display_sprint_msg("Connecting to MQTT\n%s on %d", _config->mqtt.server, _config->mqtt.port);
    _mqttClient->connect();
}

response_t mqtt_svc_config(AsyncMqttClient *mqttClient, mqtt_svc_config_args_t *config, device_list_t *devices) {
    _mqttClient = mqttClient;
    _config = config;
    _devices = devices;

    WiFi.mode(WIFI_STA);
    WiFi.enableIpV6();
    WiFi.onEvent([](WiFiEvent_t event) {
        log_infoln("WiFi evento raised.");
        switch (event) {
            case SYSTEM_EVENT_STA_GOT_IP:
                log_infoln("WiFi connected");
                log_infoln("IP address: %s", WiFi.localIPv6().toString());
                display_sprint_msg("Connected on %s\nGot ipv6 %s", _config->wifi.ssid, WiFi.localIPv6().toString());
                connect_to_mqtt();
                break;
        } });

    _mqttClient->setServer(_config->mqtt.server, _config->mqtt.port);
    _mqttClient->setClientId(_config->mqtt.client_id);
    _mqttClient->onConnect([](bool session) {
        log_infoln("Connected on MQTT %s:%d", _config->mqtt.server, _config->mqtt.port);
        log_infoln("Subscribing topics...");

        for (int i = 0; i < _devices->size; i++) {
            char buffer[255];
            snprintf(buffer, sizeof(buffer), "devices/%04x:%04x:%04x:%04x:%04x:%04x/+",
                     _devices->list[i].mac[0],
                     _devices->list[i].mac[1],
                     _devices->list[i].mac[2],
                     _devices->list[i].mac[3],
                     _devices->list[i].mac[4],
                     _devices->list[i].mac[5]);
            _mqttClient->subscribe(buffer, 1);
            log_infoln("Subscribed on %s", buffer);
        }

        log_infoln("Topics subscribed successfully.");
        display_sprint_msg("Connected on MQTT %s on %d", _config->mqtt.server, _config->mqtt.port);
    });

    _mqttClient->onDisconnect([](AsyncMqttClientDisconnectReason reason) {
        log_errorln("Disconnected from MQTT.");
        display_print_msg("Disconnected from MQTT.");

        if (WiFi.isConnected()) {
            connect_to_mqtt();
        }
    });
    log_infoln("MQTT Server configured.");
    return Success;
}

response_t mqtt_svc_init() {
    
    xTaskCreatePinnedToCore(connect_to_wifi, "WiFi connection", 4096, NULL, 1, &_wifiTaskHandle, CONFIG_ARDUINO_RUNNING_CORE);
    
    return Success;
}
