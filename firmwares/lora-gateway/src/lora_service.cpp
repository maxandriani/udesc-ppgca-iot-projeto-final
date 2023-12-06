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

void lora_svc_loop(void *args) {
    SPI.begin(SCK,MISO,MOSI,SS);
	_loraClient->begin(_config->freq, _config->paboost);

    for (;;) {
        _loraClient->receive();
        vTaskDelay(20);
    }
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
    xTaskCreatePinnedToCore(lora_svc_loop, "LoRa Svc Loo", 3072, NULL, 1, &_loraTaskHandle, ARDUINO_RUNNING_CORE);
    log_infoln("LoRa Monitor is Running");
    return Success;
}