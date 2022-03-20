CC=gcc
CFLAGS=-Wall *.c -o gengar -lws2_32 -lgdi32
CFLAGS_RELEASE=-mwindows
MACRO_FLAGS=-DALAKAZAM_KEY=\"${ALAKAZAM_KEY}\" -DGENGAR_KEY=\"${GENGAR_KEY}\"


debug: validate_auth_keys
	$(CC) $(CFLAGS) $(MACRO_FLAGS)

release: validate_auth_keys
	$(CC) $(CFLAGS) $(CFLAGS_RELEASE) $(MACRO_FLAGS)

release_host: validate_auth_keys validate_alakazam_host
	$(CC) $(CFLAGS) $(CFLAGS_RELEASE) $(MACRO_FLAGS) -DALAKAZAM_HOST=\"${ALAKAZAM_HOST}\"


validate_auth_keys:
	scripts/validate_auth_key

validate_alakazam_host:
	scripts/validate_alakazam_host
