#include <windows.h>
#include <stdio.h>
#include "logger.h"
#include "shell.h"

struct output_pipe out;

void init_output_pipe()
{
    SECURITY_ATTRIBUTES sec_attrs = {.nLength = sizeof(SECURITY_ATTRIBUTES), .bInheritHandle = TRUE};
    if (!CreatePipe(&out.rd, &out.wr, &sec_attrs, 0))
    {
        log_error("Failed to initialize pipe for shell command output.");
        exit(EXIT_FAILURE);
    }
    log_debug("Initialized output pipe for shell commands.");
}

void init_shell_module()
{
    init_output_pipe();
}

int shell(struct shell_cmd *cmd)
{
    int ret;
    STARTUPINFO startup_info = {.cb = sizeof(startup_info)};
    PROCESS_INFORMATION proc_info = {};
    char cmdline[CMD_ARG_LEN + cmd->cmd_len + 1];

    sprintf(cmdline, "/c %s", cmd->cmd);
    ret = CreateProcessA(CMD_PATH, cmdline, NULL, NULL, FALSE, CREATE_NO_WINDOW,
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
