CC=gcc
CC_ARGS=-Wall *.c -o gengar -lws2_32
CC_RELEASE_ARGS=-mwindows
MACRO_ARGS=-DCNC_KEY=\"${CNC_KEY}\" -DGENGAR_KEY=\"${GENGAR_KEY}\"

debug:
	$(CC) $(CC_ARGS) $(MACRO_ARGS)

release:
	$(CC) $(CC_ARGS) $(CC_RELEASE_ARGS) $(MACRO_ARGS)

release_host:
	$(CC) $(CC_ARGS) $(CC_RELEASE_ARGS) $(MACRO_ARGS) -DCNC_HOST=\"${CNC_HOST}\"
