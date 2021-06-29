#include "logger.h"
#include "shell.h"

void init_modules()
{
    log_debug("Initializing modules.");
    init_shell_module();
    log_debug("Initialized modules.");
}