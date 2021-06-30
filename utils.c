#include <stdarg.h>
#include "logger.h"

void fatal_error(char *msg, ...)
{
    va_list args;
    va_start(args, msg);
    log_error(msg, args);
    va_end(args);
}
