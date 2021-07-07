debug:
	gcc *.c -o gengar -lws2_32

release:
	gcc *.c -o gengar -lws2_32 -mwindows

release_host:
	gcc *.c -o gengar -lws2_32 -mwindows -DHOST=\"$(host)\"
