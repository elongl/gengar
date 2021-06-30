#include <stdarg.h>

void _log(const char *tag, const char *msg, va_list args);
void log_debug(const char *msg, ...);
void log_info(const char *msg, ...);
void log_warning(const char *msg, ...);
void log_error(const char *msg, ...);
