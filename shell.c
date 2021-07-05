#include <windows.h>
#include <stdio.h>
#include "logger.h"
#include "shell.h"

struct output_pipe out_pipe;

void init_output_pipe()
{
    SECURITY_ATTRIBUTES sec_attrs = {.nLength = sizeof(SECURITY_ATTRIBUTES), .bInheritHandle = TRUE};
    if (!CreatePipe(&out_pipe.rd, &out_pipe.wr, &sec_attrs, 0))
        fatal_error("Failed to initialize pipe for shell command output.");
}

void init_shell_module()
{
    init_output_pipe();
    log_debug("Initialized shell module.");
}

int read_shell_output(struct shell_cmd *cmd)
{
    int ret;
    DWORD bytes_read, bytes_available;

    ret = PeekNamedPipe(out_pipe.rd, NULL, 0, NULL, &bytes_available, NULL);
    if (!ret)
        fatal_error("Failed to get availables bytes from output pipe: %ld", GetLastError());
    if (!bytes_available)
        return 0;

    ret = ReadFile(out_pipe.rd, cmd->out, sizeof(cmd->out), &bytes_read, NULL);
    if (!ret)
        fatal_error("Failed to read from output pipe: %ld", GetLastError());
    return bytes_read;
}

void shell(struct shell_cmd *cmd)
{
    int ret;
    char cmdline[CMD_ARG_LEN + cmd->cmd_len + 1];
    PROCESS_INFORMATION proc_info;
    STARTUPINFO startup_info = {.cb = sizeof(startup_info), .dwFlags = STARTF_USESTDHANDLES, .hStdError = out_pipe.wr, .hStdOutput = out_pipe.wr};

    sprintf(cmdline, "/c %s", cmd->cmd);
    ret = CreateProcessA(CMD_PATH, cmdline, NULL, NULL, TRUE, CREATE_NO_WINDOW,
                         NULL, NULL, &startup_info, &proc_info);
    if (!ret)
        fatal_error("Error at CreateProcessA(): %ld", GetLastError());
    WaitForSingleObject(proc_info.hProcess, INFINITE);
    GetExitCodeProcess(proc_info.hProcess, &cmd->exit_code);
    CloseHandle(proc_info.hProcess);
    CloseHandle(proc_info.hThread);
    log_info("\"%s\" exited with %d", cmd->cmd, cmd->exit_code);
}
