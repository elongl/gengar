#include <stdio.h>

#define CNC_PORT "5000"
#define SLEEP_INTERVAL_ON_CONNREFUSED_MS 30 * 1000

void init_cnc_conn();
int send_to_cnc(void *buf, size_t len);
int recv_from_cnc(void *buf, size_t len);
