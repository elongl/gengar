#include <windows.h>
#include <stdio.h>
#include "logger.h"

SOCKET cnc = INVALID_SOCKET;

void _init_winsock()
{
    WSADATA wsaData;
    int res;

    res = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (res != 0)
    {
        printf("WSAStartup failed: %d\n", res);
        exit(EXIT_FAILURE);
    }
}

int connect_to_cnc()
{
}