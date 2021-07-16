#define STRLEN(s) ((sizeof(s) / sizeof(s[0])) - sizeof(s[0]))

void fatal_error(char *msg, ...);
