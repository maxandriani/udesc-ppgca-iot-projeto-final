#include <Arduino.h>
#include <leds.h>
#include <logs.h>
#include <display.h>
#include <oled/SSD1306Wire.h>
#include <WiFi.h>
#include <AsyncMqttClient.h>
#include "mqtt_service.h"
#include "lora_service.h"
#include "gateway_service.h"

static SSD1306Wire _display(0x3c, SDA_OLED, SCL_OLED, RST_OLED, GEOMETRY_128_64);
static AsyncMqttClient _mqttClient;
static LoRaClass _loraClient;

static mqtt_svc_config_args_t _mqttConfig = {
  .wifi = {
    .ssid = "Whisky_a_GoGo",
    .passwd = "summertime" },
  .mqtt = {
    .server = "192.168.86.47", // "fe80:0:0:0:f5eb:4e98:1c5:c235",// "192.168.86.47",  // fe80::f5eb:4e98:1c5:c235
    .port = 1883,
    .user = "",
    .passwd = "",
    .client_id = "lora_gateway_0" } };

static lora_svc_config_args_t _loraConfig = {
  .freq = 915E6
};

static device_list_t _devices = {
    .list = {},
    .size = 0 };

void setup() {
  Serial.begin(9600);
  leds_config();

  log_info("Blink Test...");
  for (int i = 0; i < 10; i++) {
    led_up_and_wait(200);
    led_down_and_wait(200);
  }
  log_infoln(" done.");

  // Até aqui, não existem logs ou interface..
  int init_result = 0;
  init_result << display_config(&_display);
  init_result << display_init();

  if (init_result > 0) {
    led_start_panic(); // Não foi possível inicializar as interfaces de comunicação. Só resta comunicar o SOS e resetar...
    delay(10000);
    ESP.restart();
  }

  display_print_loading();
  mqtt_svc_config(&_mqttClient, &_mqttConfig, &_devices);
  lora_svc_config(&_loraClient, &_loraConfig, &_devices);
  gateway_svc_config(&_loraClient, &_mqttClient, &_devices);

  mqtt_svc_init();
  lora_svc_init();
}

void loop() {
  // put your main code here, to run repeatedly:
}