#include <windows.h>
#include "utils.h"

#define CMD_PATH "C:\\Windows\\System32\\cmd.exe"
#define CMD_ARG_LEN STRLEN("/c ")

#define EXEC_SHELL_FAILED -1

struct output_pipe
{
    HANDLE rd;
    HANDLE wr;
};

struct shell_cmd
{
    char *cmd;
    unsigned int cmd_len;
    struct output_pipe out;
    long unsigned int exit_code;
};

int shell(struct shell_cmd *);
int read_shell_output(struct output_pipe *, void *, unsigned int);
