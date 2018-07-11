udp: udp_client.c udp_server.c protocols.c
	gcc -o protocols.o -c protocols.c
	gcc -o udps.o -c udp_server.c
	gcc -o udpc.o -c udp_client.c
	gcc -o udp_server.o udps.o protocols.o -g
	gcc -o udp_client.o udpc.o protocols.o -g
