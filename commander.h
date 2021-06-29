#define ECHO_CMD_TYPE 0
#define SHELL_CMD_TYPE 1

struct cmd
{
    unsigned int type;
};

struct echo_cmd
{
    unsigned int text_len;
    char *text;
};

struct shell_cmd
{
    unsigned int cmd_len;
    char *cmd;
    char *output;
};

void listen_for_cmds();