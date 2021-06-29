#include <windows.h>
#include <stdio.h>
#include "logger.h"
#include "shell.h"

int shell(struct shell_cmd *cmd)
{
    int ret;
    STARTUPINFO startup_info = {.cb = sizeof(startup_info)};
    PROCESS_INFORMATION proc_info = {};
    char cmdline[strlen("/c ") + strlen(cmd->cmd) + 1];

    sprintf(cmdline, "/c %s", cmd->cmd);
    ret = CreateProcessA("cmd.exe", cmdline, NULL, NULL, FALSE, CREATE_NO_WINDOW,
                         NULL, NULL, &startup_info, &proc_info);
    if (!ret)
    {
        log_error("Error at CreateProcessA(): %ld", GetLastError());
        return EXEC_SHELL_FAILED;
    }
    WaitForSingleObject(proc_info.hProcess, INFINITE);
    GetExitCodeProcess(proc_info.hProcess, &cmd->exit_code);
    CloseHandle(proc_info.hProcess);
    log_info("\"%s\" exited with %d", cmd->cmd, cmd->exit_code);
}
