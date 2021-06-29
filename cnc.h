#include <stdio.h>

int init_cnc_conn();
int send_to_cnc(void *buf, size_t len);
int recv_from_cnc(void *buf, size_t len);
