#include <windows.h>

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
    unsigned long exit_code;
    PROCESS_INFORMATION proc_info;
};

void shell_init_module();
void shell(struct shell_cmd *);
unsigned long shell_read_output(struct shell_cmd *);
void shell_close(struct shell_cmd *cmd);
