#include <Arduino.h>
#include <stdlib.h>
#include <leds.h>
#include <logs.h>
#include <display.h>
#include <oled/SSD1306Wire.h>
#include <WiFi.h>
#include "lora_service.h"
#include "water_level_service.h"
#include "water_tds_service.h"
#include "water_temp_service.h"
#include "water_level_service.h"
#include "sensor_service.h"

static SSD1306Wire _display(0x3c, SDA_OLED, SCL_OLED, RST_OLED, GEOMETRY_128_64);
static LoRaClass _loraClient;
static measures_t _measures;
static measures_config_t _config;

static volatile bool SENSOR_START_TOKEN = false;

static lora_svc_config_args_t _loraConfig = {
  .freq = 915E6
};

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
  water_temp_svc_config(&_measures, &_config.temp);
  water_tds_svc_config(&_measures, &_config.tds);
  water_level_svc_config(&_measures, &_config.level);
  sensor_config((bool *)&SENSOR_START_TOKEN);
  lora_svc_config(&_loraClient, &_loraConfig, &_measures, &_config);
  
  display_print_loading();
  water_temp_svc_init();
  water_tds_svc_init();
  water_level_svc_init();
  sensor_init();
  lora_svc_init();
}

void loop() {
  // put your main code here, to run repeatedly:
}