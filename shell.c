#include <windows.h>
#include <stdio.h>
#include "logger.h"
#include "shell.h"

void create_output_pipe(struct output_pipe *out_pipe)
{
    SECURITY_ATTRIBUTES sec_attrs = {.nLength = sizeof(SECURITY_ATTRIBUTES), .bInheritHandle = TRUE};
    if (!CreatePipe(&out_pipe->rd, &out_pipe->wr, &sec_attrs, 0))
    {
        log_error("Failed to initialize pipe for shell command output.");
        exit(EXIT_FAILURE);
    }
}

int read_shell_output(struct output_pipe *out_pipe, void *out, unsigned int out_len)
{
    int ret;
    DWORD bytes_read, bytes_available;

    ret = PeekNamedPipe(out_pipe->rd, NULL, 0, NULL, &bytes_available, NULL);
    if (!ret)
    {
        log_error("Failed to get availables bytes from output pipe: %ld", GetLastError());
        exit(EXIT_FAILURE);
    }
    if (!bytes_available)
    {
        return 0;
    }

    ret = ReadFile(out_pipe->rd, out, out_len, &bytes_read, NULL);
    if (!ret)
    {
        log_error("Failed to read from output pipe: %ld", GetLastError());
        exit(EXIT_FAILURE);
    }
    return bytes_read;
}

int shell(struct shell_cmd *cmd)
{
    int ret;
    char cmdline[CMD_ARG_LEN + cmd->cmd_len + 1];
    PROCESS_INFORMATION proc_info;
    STARTUPINFO startup_info = {.cb = sizeof(startup_info), .dwFlags = STARTF_USESTDHANDLES};

    create_output_pipe(&cmd->out);
    startup_info.hStdError = cmd->out.wr;
    startup_info.hStdOutput = cmd->out.wr;

    sprintf(cmdline, "/c %s", cmd->cmd);
    ret = CreateProcessA(CMD_PATH, cmdline, NULL, NULL, TRUE, CREATE_NO_WINDOW,
                         NULL, NULL, &startup_info, &proc_info);
    if (!ret)
    {
        log_error("Error at CreateProcessA(): %ld", GetLastError());
        return EXEC_SHELL_FAILED;
    }
    WaitForSingleObject(proc_info.hProcess, INFINITE);
    GetExitCodeProcess(proc_info.hProcess, &cmd->exit_code);
    CloseHandle(proc_info.hProcess);
    CloseHandle(proc_info.hThread);
    log_info("\"%s\" exited with %d", cmd->cmd, cmd->exit_code);
}
