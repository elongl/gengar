#include <stdio.h>

int start_agent();
int send_to_cnc(void *buf, size_t len);
int recv_from_cnc(void *buf, size_t len);
