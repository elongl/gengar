debug:
	gcc -Wall *.c -o gengar -lws2_32

release:
	gcc -Wall *.c -o gengar -lws2_32 -mwindows

release_host:
	gcc -Wall *.c -o gengar -lws2_32 -mwindows -DCNC_HOST=\"$(cnc_host)\"
