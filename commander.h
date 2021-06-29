#define SHELL_CMD_TYPE 0

struct cmd
{
    unsigned int type;
};

void listen_for_cmds();