#define ECHO_CMD_TYPE 0
#define SHELL_CMD_TYPE 1

struct cmd
{
    unsigned int type;
};

void listen_for_cmds();
