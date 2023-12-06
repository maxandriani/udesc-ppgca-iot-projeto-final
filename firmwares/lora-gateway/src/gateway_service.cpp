#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <oled/SSD1306Wire.h>
#include <logs.h>
#include <display.h>
#include <lora/LoRa.h>
#include <AsyncMqttClient.h>
#include <responses.h>
#include <devices.h>
#include <lora_events.h>

static LoRaClass *_loraClient;
static AsyncMqttClient *_mqttClient;
static volatile device_list_t *_devices;
static uint32_t messageId = 0;

void mqtt_on_receive(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
    char strMac[6][3];
    uint8_t mac[6];
    char cmd[50];
    lora_events_t event;

    log_infoln("MQTT packet received.\n%s", topic);

    int validTopic = sscanf(topic, "devices/%[^\]/%s", strMac[0], cmd) == 2 // match 2 palavras
                     && sscanf(strMac[0], "%2s:%2s:%2s:%2s:%2s:%2s", strMac[0], strMac[1], strMac[2], strMac[3], strMac[4], strMac[5]) == 6; // MAC ADDRESS bem formatado.

    if (!validTopic) {
        log_errorln("Invalid topic, skip parsing MQTT packer");
        display_sprint_msg("Invalid topic:\n%s\nSkip parsing MQTT packet.", topic);
        return;
    }

    for (int i = 0; i < 6; i++) {
        mac[i] = (uint8_t)strtoul(strMac[i], NULL, 16);
    }

    if (memcmp(cmd, "down", sizeof(cmd)) == 0) {
        event = Receive;
    } else if (memcmp(cmd, "config", sizeof(cmd)) == 0) {
        event = Joint;
    } else {
        log_infoln("unknown command. Skipping.");
        display_sprint_msg("Unknown command %s. Skipping", cmd);
        return;
    }

    uint8_t device_exists = device_index_of((device_list_t *)_devices, mac);
    if (device_exists > -1) {
        // device encontrado!
        log_infoln("Device  %04x:%04x:%04x:%04x:%04x:%04x found!", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        // mac     cmd    size    payload
        // 6 bytes 1 byte 4 bytes ...
        _loraClient->beginPacket();
        _loraClient->write(mac[0]);
        _loraClient->write(mac[1]);
        _loraClient->write(mac[2]);
        _loraClient->write(mac[3]);
        _loraClient->write(mac[4]);
        _loraClient->write(mac[5]);
        _loraClient->write(event);
        _loraClient->write(len >> 32);
        _loraClient->write(len >> 16);
        _loraClient->write(len >> 8);
        _loraClient->write(len);
        for (int i = 0; i < len; i++) {
            _loraClient->write(payload[i]);
        }
        _loraClient->endPacket(true);

        display_sprint_msg("MQTT packet received.\n%04x:%04x:%04x:%04x:%04x:%04x\nSent to LoRa.", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        log_infoln("MQTT packet received. %04x:%04x:%04x:%04x:%04x:%04x. Sent to LoRa. %s", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], payload);
        return;
    }
}

void lora_on_receive(int32_t packetSize) {
    // mac     cmd    size    payload
    // 6 bytes 1 byte 4 bytes ...

    if (packetSize < 12) {
        log_errorln("LoRa received a malformed packet. %d bytes", packetSize);
        display_sprint_msg("LoRa received a malformed packet. %d bytes", packetSize);
        return;
    }

    uint8_t mac[6];
    lora_events_t cmd;
    uint32_t size;

    for (int i = 0; i < 6; i++) {
        mac[i] = _loraClient->read();
    }
    
    _loraClient->readBytes((uint8_t *) mac, 6);
    _loraClient->readBytes((uint8_t *) &cmd, 1);
    _loraClient->readBytes((uint8_t *) &size, 4);
    
    char data[size];

    _loraClient->readBytes((uint8_t *) &data, size);

    log_info("Pkt Received! From: %04x:%04x:%04x:%04x:%04x:%04x Msg: %s", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], data);
    display_sprint_msg("Pkt Received!\nFrom %04x:%04x:%04x:%04x:%04x:%04x\nMsg: %s", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], data);

    char topic[256];
    char buffer[256];

    switch (cmd) {
        case Join:
            sniprintf(topic, 256, "join/%04x:%04x:%04x:%04x:%04x:%04x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
            device_insert_if_not_exists((device_list_t *)_devices, mac);
            snprintf(buffer, sizeof(buffer), "devices/%04x:%04x:%04x:%04x:%04x:%04x/+", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
            _mqttClient->subscribe(buffer, 1);
            log_infoln("Subscribed on %s", buffer);
            _mqttClient->publish(topic, 1, true, data, sizeof(data), false, messageId);
            messageId++;
            log_infoln("MQTT Packer sent on %s", topic);
            break;
        
        case Send:
            sniprintf(topic, 256, "devices/%04x:%04x:%04x:%04x:%04x:%04x/up", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
            if (device_index_of((device_list_t *)_devices, mac) == -1) {
                log_infoln("Device %04x:%04x:%04x:%04x:%04x:%04x is not our business.", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
                display_sprint_msg("Device %04x:%04x:%04x:%04x:%04x:%04x is not our business.", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
                return; // Não é nosso dispositivo
            }
            _mqttClient->publish(topic, 1, true, data, sizeof(data), false, messageId);
            messageId++;
            break;

        default:
            log_infoln("Not for me...");
    }
}

response_t gateway_svc_config(LoRaClass *loraClient, AsyncMqttClient *_mqttClient, device_list_t *devices) {
    _loraClient = loraClient;
    _mqttClient = _mqttClient;
    _devices = devices;

    _loraClient->onReceive(lora_on_receive);
    _mqttClient->onMessage(mqtt_on_receive);

    return Success;
}