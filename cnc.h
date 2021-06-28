#include <stdio.h>

int connect_to_cnc();
int send_to_cnc(void *buf, size_t len);
int recv_from_cnc(void *buf, size_t len);
