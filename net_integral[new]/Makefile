all: client.out server.out

local: server_l.out client_l.out

server.out: server.c Makefile
	gcc server.c -g -pthread -o server.out

client.out: client.c Makefile
	gcc client.c -g -pthread -o client.out

server_l.out: server_local.c Makefile
	gcc server_local.c -g -pthread -o server_l.out

client_l.out: client_local.c Makefile
	gcc client_local.c -g -pthread -o client_l.out

clean:
	rm *.o *.out