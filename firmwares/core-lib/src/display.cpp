#include <stdio.h>
#include <stdarg.h>
#include "responses.h"
#include "display.h"
#include "logs.h"
#include <oled/SSD1306Wire.h>

static uint8_t DISPLAY_INITIALIZED = 0;
static SSD1306Wire *_display;

response_t display_config(SSD1306Wire *display) {
    _display = display;
    return Success;
}

response_t display_init() {
    if (!_display->init())
        return Error;
    
    DISPLAY_INITIALIZED = 1;
    return Success;
}

void _throw_display_not_initialized() {
    log_errorln("Display não foi inicializado. Certifique-se que você chamou 'display_init() e display_config().'");
}

void display_print_loading() {
    if (!DISPLAY_INITIALIZED)
        return _throw_display_not_initialized();

    _display->clear();
    _display->drawHorizontalLine(0, 0, 128);
    _display->drawHorizontalLine(0, 63, 128);
    _display->drawStringMaxWidth(5, 26, 118, "Loading...");
    _display->display();
}

void display_print_msg(const char *msg) {
    if (!DISPLAY_INITIALIZED)
        return _throw_display_not_initialized();
        
    _display->clear();
    _display->drawHorizontalLine(0, 0, 128);
    _display->drawHorizontalLine(0, 63, 128);
    _display->drawStringMaxWidth(5, 10, 118, msg);
    _display->display();
}

void display_sprint_msg(const char *msg, ...) {
    char buffer[384];

    va_list args;
    va_start(args, msg);
    snprintf(buffer, 384, msg, args);
    va_end(args);

    display_print_msg(buffer);
}