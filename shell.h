#define EXEC_SHELL_FAILED -1

struct shell_cmd
{
    unsigned int cmd_len;
    char *cmd;
    void *output;
    unsigned int output_len;
    long unsigned int exit_code;
};

int shell(struct shell_cmd *);