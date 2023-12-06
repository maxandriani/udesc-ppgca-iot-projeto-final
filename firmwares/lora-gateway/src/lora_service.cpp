#include <stdlib.h>
#include <responses.h>
#include <oled/SSD1306Wire.h>
#include <lora/LoRa.h>
#include <logs.h>
#include "lora_service.h"
#include "devices.h"

static TaskHandle_t _loraTaskHandle;
static lora_svc_config_args_t *_config;
static LoRaClass *_loraClient;
static volatile device_list_t *_devices;
static lora_on_receive_cb _on_receive;

void lora_svc_loop(void *args) {
    SPI.begin(SCK,MISO,MOSI,SS);
	_loraClient->begin(_config->freq, _config->paboost);

    for (;;) {

        int32_t size = _loraClient->parsePacket();
        if (size && _on_receive != NULL) {
            (*_on_receive)(size);
        }
            
        vTaskDelay(20);
    }
}

void lora_set_on_receive_cb(lora_on_receive_cb cb) {
    _on_receive = cb;
}

response_t lora_svc_config(LoRaClass *loraClient, lora_svc_config_args_t *config, device_list_t *devices) {
    _loraClient = loraClient;
    _config = config;
    _devices = devices;
    _loraClient->setPins(SS,RST_LoRa,DIO0);
    log_infoln("LoRa Radio Configured");
    return Success;
}

response_t lora_svc_init() {
    log_infoln("Scheduling LoRa Monitor");
    xTaskCreatePinnedToCore(lora_svc_loop, "LoRa Svc Loo", 8124, NULL, 1, &_loraTaskHandle, ARDUINO_RUNNING_CORE);
    log_infoln("LoRa Monitor is Running");
    return Success;
}