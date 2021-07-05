#include <windows.h>
#include "utils.h"

#define CMD_PATH "C:\\Windows\\System32\\cmd.exe"
#define CMD_ARG_LEN STRLEN("/c ")

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
    PROCESS_INFORMATION proc_info;
};

void init_shell_module();
void shell(struct shell_cmd *);
int read_shell_output(struct shell_cmd *);
