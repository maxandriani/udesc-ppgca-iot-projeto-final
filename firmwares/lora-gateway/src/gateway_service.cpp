#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <logs.h>
#include <display.h>
#include <lora/LoRa.h>
#include <AsyncMqttClient.h>
#include <responses.h>
#include <devices.h>
#include <lora_events.h>
#include <lora_service.h>

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

    int validTopic = sscanf(topic, "devices/%[^\]/%s", strMac[0], &cmd) == 2 // match 2 palavras
                     && sscanf(strMac[0], "%2hhx:%2hhx:%2hhx:%2hhx:%2hhx:%2hhx", &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]) == 6; // MAC ADDRESS bem formatado.

    if (!validTopic) {
        log_errorln("Invalid topic, skip parsing MQTT packer");
        display_sprint_msg("Invalid topic:\n%s\nSkip parsing MQTT packet.", topic);
        return;
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
        log_infoln("Device  %02x:%02x:%02x:%02x:%02x:%02x found!", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
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

        display_sprint_msg("MQTT packet received.\n%02x:%02x:%02x:%02x:%02x:%02x\nSent to LoRa.", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        log_infoln("MQTT packet received. %02x:%02x:%02x:%02x:%02x:%02x. Sent to LoRa. %s", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], payload);
        return;
    }
}

void lora_on_receive(int32_t packetSize) {
    // mac     cmd    size    payload
    // 6 bytes 1 byte 4 bytes ...
    uint8_t payload[2048];
    _loraClient->readBytes(payload, packetSize);

    if (packetSize < 7) {
        log_errorln("LoRa received a malformed packet. %d bytes", packetSize);
        display_sprint_msg("LoRa received a malformed packet. %d bytes", packetSize);
        return;
    }

    uint8_t mac[6] = { payload[0], payload[1], payload[2], payload[3], payload[4], payload[5] };
    lora_events_t cmd = (lora_events_t) payload[6];
    
    log_infoln("Pkt Received!  %d bytes", packetSize);
    display_sprint_msg("Pkt Received!\n%d bytes", packetSize);

    char joinTopic[256];
    char deviceTopic[256];
    char data[2048];

    switch (cmd) {
        case Join:
            sniprintf(joinTopic, 256, "join/%02x-%02x-%02x-%02x-%02x-%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
            device_insert_if_not_exists((device_list_t *)_devices, mac);
            snprintf(deviceTopic, 256, "devices/%02x-%02x-%02x-%02x-%02x-%02x/+\0", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
            _mqttClient->subscribe(deviceTopic, 1);
            log_infoln("Subscribed on %s", deviceTopic);
            _mqttClient->publish(joinTopic, 1, true, 0, 0, false, messageId);
            messageId++;
            log_infoln("MQTT Packer sent on %s", joinTopic);
            break;
        
        case Send:
            sniprintf(deviceTopic, 256, "devices/%02x-%02x-%02x-%02x-%02x-%02x/up", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
            if (device_index_of((device_list_t *)_devices, mac) == -1) {
                log_infoln("Device %02x:%02x:%02x:%02x:%02x:%02x is not our business.", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
                display_sprint_msg("Device %02x:%02x:%02x:%02x:%02x:%02x is not our business.", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
                return; // Não é nosso dispositivo
            }
            for (int i = 7; i < packetSize; i++) {
                data[i] = (char)payload[i];
            }
            
            _mqttClient->publish(deviceTopic, 1, true, data, packetSize - 7, false, messageId);
            messageId++;

            log_infoln("Message sent.\n%s", data);
            display_sprint_msg("Message sent.\n%s", data);
            break;

        default:
            log_infoln("Not for me...");
    }
}

response_t gateway_svc_config(LoRaClass *loraClient, AsyncMqttClient *mqttClient, device_list_t *devices) {
    _loraClient = loraClient;
    _mqttClient = mqttClient;
    _devices = devices;

    //_loraClient->onReceive(lora_on_receive);
    lora_set_on_receive_cb((lora_on_receive_cb)lora_on_receive);
    _mqttClient->onMessage(mqtt_on_receive);

    return Success;
}