#include "utils.h"

#define CMD_PATH "C:\\Windows\\System32\\cmd.exe"
#define CMD_ARG_LEN STRLEN("/c ")

#define EXEC_SHELL_FAILED -1

struct shell_cmd
{
    unsigned int cmd_len;
    char *cmd;
    void *output;
    unsigned int output_len;
    long unsigned int exit_code;
};

struct output_pipe
{
    HANDLE rd;
    HANDLE wr;
};

void init_shell_module();
int shell(struct shell_cmd *);