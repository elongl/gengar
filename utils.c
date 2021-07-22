#include <stdarg.h>
#include <stdlib.h>
#include "include/logger.h"

void fatal_error(char *msg, ...)
{
    va_list args;

    va_start(args, msg);
    _log("FATAL", msg, args);
    va_end(args);

    exit(EXIT_FAILURE);
}
