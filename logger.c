#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>
#include "include/logger.h"

void _log(const char *tag, const char *msg, va_list args)
{
    time_t now;
    time(&now);
    char *date = ctime(&now);
    date[strlen(date) - 1] = '\0';
    printf("%s [%s]: ", date, tag);
    vprintf(msg, args);
    printf("\n");
}

void log_debug(const char *msg, ...)
{
    va_list args;
    va_start(args, msg);
    _log("DEBUG", msg, args);
    va_end(args);
}

void log_info(const char *msg, ...)
{
    va_list args;
    va_start(args, msg);
    _log("INFO", msg, args);
    va_end(args);
}

void log_warning(const char *msg, ...)
{
    va_list args;
    va_start(args, msg);
    _log("WARNING", msg, args);
    va_end(args);
}

void log_error(const char *msg, ...)
{
    va_list args;
    va_start(args, msg);
    _log("ERROR", msg, args);
    va_end(args);
}
