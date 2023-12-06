#ifndef DISPLAY_H
#define DISPLAY_H

#include "responses.h"
#include <oled/SSD1306Wire.h>

response_t display_config(SSD1306Wire *display);
response_t display_init();
void display_print_loading();
void display_print_msg(const char *msg);
void display_sprint_msg(const char *msg, ...);

#endif