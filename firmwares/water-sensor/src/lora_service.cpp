#include <stdlib.h>
#include <stdio.h>
#include <measures.h>
#include <display.h>
#include <logs.h>
#include <oled/SSD1306Wire.h>
#include <WiFi.h>
#include <lora_events.h>
#include "lora_service.h"
#include <lora/LoRa.h>

static TaskHandle_t _loraTaskHandle;
static lora_svc_config_args_t *_lora_config;
static volatile measures_config_t *_measures_config;
static volatile measures_t *_measures;
static LoRaClass *_loraClient;

void lora_send_join() {
    String macStr = WiFi.macAddress();
    uint8_t mac[6];
    sscanf(macStr.c_str(), "%2hhx:%2hhx:%2hhx:%2hhx:%2hhx:%2hhx", &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);
    log_infoln("Joining into LoRa Network.\n%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    char payload[1024];
    measures_to_json_str((measures_t *) _measures, payload, sizeof(payload));
    uint32_t payloadSize = strlen(payload);
    // mac     cmd    size    payload
    // 6 bytes 1 byte 4 bytes ...
    _loraClient->beginPacket();
    _loraClient->write(mac[0]);
    _loraClient->write(mac[1]);
    _loraClient->write(mac[2]);
    _loraClient->write(mac[3]);
    _loraClient->write(mac[4]);
    _loraClient->write(mac[5]);
    _loraClient->write(Join);
    // for (int i = 0; i < payloadSize; i++) {
    //     _loraClient->write(payload[i]);
    // }
    _loraClient->endPacket();

    log_infoln("Join packet sent");
    display_print_msg("Join packet sent");
    return;
}

void lora_svc_loop(void *args) {
    SPI.begin(SCK,MISO,MOSI,SS);
	_loraClient->begin(_lora_config->freq, _lora_config->paboost);

    lora_send_join();

    for (;;) {
        //_loraClient->receive();
        lora_on_receive(LoRa.parsePacket());
        vTaskDelay(20);
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
    char macStr[17];
    lora_events_t cmd;
    uint32_t size;

    for (int i = 0; i < 6; i++) {
        mac[i] = _loraClient->read();
    }
    
    _loraClient->readBytes((uint8_t *) mac, 6);
    _loraClient->readBytes((uint8_t *) &cmd, 1);
    _loraClient->readBytes((uint8_t *) &size, 4);

    snprintf(macStr, 17, "%04x:%04x:%04x:%04x:%04x:%04x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    if (memcmp(macStr, WiFi.macAddress().c_str(), sizeof(char) * 17) != 0) {
        log_infoln("O pkt não é nosso.");
        return; // Não é pra nois...
    }
        

    char data[size];

    _loraClient->readBytes((uint8_t *) &data, size);

    log_infoln("Config received!\n %s", data);
    display_sprint_msg("Config received!\n %s", data);

    switch (cmd) {
        case Joint:
            measures_config_from_json_str((measures_config *)_measures_config, data);
            break;

        default:
            log_infoln("Not for me...");
    }
}

response_t lora_svc_config(LoRaClass *loraClient, lora_svc_config_args_t *lora_config, measures_t *measures, measures_config_t *measures_config) {
    _loraClient = loraClient;
    _lora_config = lora_config;
    _measures = measures;
    _measures_config = measures_config;
    _loraClient->setPins(SS,RST_LoRa,DIO0);
    _loraClient->onReceive(lora_on_receive);
    log_infoln("LoRa Radio Configured");
    return Success;
}

response_t lora_svc_init() {
    log_infoln("Scheduling LoRa Monitor");
    xTaskCreatePinnedToCore(lora_svc_loop, "LoRa Svc", 8192, NULL, 1, &_loraTaskHandle, ARDUINO_RUNNING_CORE);
    log_infoln("LoRa Monitor is Running");
    return Success;
}

void lora_svc_send_measures() {
    String macStr = WiFi.macAddress();
    uint8_t mac[6];
    sscanf(macStr.c_str(), "%2s:%2s:%2s:%2s:%2s:%2s", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    log_infoln("Sending LoRa packet.\n%04x:%04x:%04x:%04x:%04x:%04x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    char payload[1024];
    measures_to_json_str((measures_t *) _measures, payload, sizeof(payload));
    uint32_t payloadSize = strlen(payload);
    // mac     cmd    size    payload
    // 6 bytes 1 byte 4 bytes ...
    _loraClient->beginPacket();
    _loraClient->write(mac[0]);
    _loraClient->write(mac[1]);
    _loraClient->write(mac[2]);
    _loraClient->write(mac[3]);
    _loraClient->write(mac[4]);
    _loraClient->write(mac[5]);
    _loraClient->write(Send);
    _loraClient->write(payloadSize >> 32);
    _loraClient->write(payloadSize >> 16);
    _loraClient->write(payloadSize >> 8);
    _loraClient->write(payloadSize);
    for (int i = 0; i < payloadSize; i++) {
        _loraClient->write(payload[i]);
    }
    _loraClient->endPacket(true);

    display_sprint_msg("LoRa packet sent.\n%04x:%04x:%04x:%04x:%04x:%04x\n%s", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], payload);
    log_infoln("LoRa packet sent.\n%04x:%04x:%04x:%04x:%04x:%04x\n%s", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], payload);
    return;
}
