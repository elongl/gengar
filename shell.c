#include <windows.h>
#include <stdio.h>
#include "logger.h"

int exec_shell_cmd(char *cmd, char *output)
{
    char psBuffer[128];
    FILE *pPipe;

    if ((pPipe = _popen(cmd, "r")) == NULL)
        exit(1);
    while (fgets(psBuffer, 128, pPipe))
        puts(psBuffer);

    if (feof(pPipe))
        log_info("\nProcess returned %d\n", _pclose(pPipe));
    else
        log_error("Failed to read the pipe to the end.");
}
