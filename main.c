#include "commander.h"
#include "logger.h"

int main()
{
    log_info("Gengar started.");
    listen_for_cmds();
}