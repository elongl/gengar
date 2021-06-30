#include "logger.h"
#include "shell.h"

void init_modules()
{
    log_info("Initializing modules.");
    init_shell_module();
    log_info("Initialized modules.");
}