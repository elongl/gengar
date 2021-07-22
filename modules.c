#include "include/logger.h"
#include "include/shell.h"

void modules_init()
{
    log_info("Initializing modules.");
    shell_init_module();
    log_info("Initialized modules.");
}
