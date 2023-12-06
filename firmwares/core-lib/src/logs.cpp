#include <stdio.h>
#include <stdarg.h>
#include <Arduino.h>
#include "responses.h"
#include "logs.h"

void _log_msg(const char *msg, va_list args) {
    char buffer[512];
    vsnprintf(buffer, sizeof(buffer), msg, args);
    Serial.print(buffer);
}

void log_info(const char *msg, ...) {
    char info[] = "INFO: %s";
    char buffer[512];
    snprintf(buffer, 512, info, msg);
    va_list args;
    va_start(args, msg);
    _log_msg(buffer, args);
    va_end(args);
}

void log_infoln(const char *msg, ...) {
    char info[] = "INFO: %s\n";
    char buffer[512];
    snprintf(buffer, 512, info, msg);
    va_list args;
    va_start(args, msg);
    _log_msg(buffer, args);
    va_end(args);
}

void log_error(const char *msg, ...) {
    char error[] = "ERROR: %s";
    char buffer[512];
    snprintf(buffer, 512, error, msg);
    va_list args;
    va_start(args, msg);
    _log_msg(buffer, args);
    va_end(args);
}

void log_errorln(const char *msg, ...) {
    char error[] = "ERROR: %s\n";
    char buffer[512];
    snprintf(buffer, 512, error, msg);
    va_list args;
    va_start(args, msg);
    _log_msg(buffer, args);
    va_end(args);
}
