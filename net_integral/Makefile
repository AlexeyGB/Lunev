all: serv.out slave.out

serv.out: server.c Makefile
	gcc server.c -g -o serv.out

slave.out: slave.c Makefile
	gcc slave.c -g -pthread -o slave.out

final: serv_f.out slave_f.out

serv_f.out: server_f.c Makefile
	gcc server_f.c -g -o serv.out

slave_f.out: slave_f.c Makefile
	gcc slave_f.c -g -pthread -o slave.out

clean:
	rm *.o *.out