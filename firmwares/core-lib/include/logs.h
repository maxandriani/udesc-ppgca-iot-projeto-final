#ifndef LOGS_H
#define LOGS_H

#include <stdio.h>
#include <stdarg.h>
#include "responses.h"

void log_info(const char *msg, ...);
void log_infoln(const char *msg, ...);
void log_error(const char *msg, ...);
void log_errorln(const char *msg, ...);

#endif