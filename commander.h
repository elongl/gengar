#define ECHO_CMD_TYPE 0
#define SHELL_CMD_TYPE 1
#define MSGBOX_CMD_TYPE 2

struct cmd
{
    unsigned int type;
};

void listen_for_cmds();
