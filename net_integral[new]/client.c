#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include <sys/types.h>
#include <netinet/tcp.h>
#include <time.h>
#include <math.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <ifaddrs.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/select.h>
#include <poll.h>
#include <string.h>

#define SND_BRCAST_PORT 8000
#define POLL_TIMEOUT 5000

#define FROM	-1000
#define TO		10000
#define DELTA 0.000003	

#define handle_cr_error(msg) \
    do { fprintf(stderr, "%s\n", msg); perror("Details"); exit(EXIT_FAILURE); } while (0)

#define handle_error(msg) \
    do { fprintf(stderr, "%s\n", msg); perror("Details"); } while (0)

typedef struct 
{
	struct sockaddr_in addr;
	int threads_on_server;
} server_info_t; 

typedef struct 
{
	int threads_amount;
	char msg[15];
}server_answer_t;

typedef struct 
{
	double from;
	double to;
	double delta;
} server_task_t;

typedef struct 
{
	double from;
	double to;
	double delta;
	double result;
	struct sockaddr_in *server_addr;

} communicator_task_t;

void *communicator( void *arg )
{
	communicator_task_t *task = (communicator_task_t *) arg;

	int server_socket;
	server_socket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	if( server_socket == -1 )
		handle_cr_error("Can't create server socket");

	if( connect( server_socket, (struct sockaddr *) task->server_addr, sizeof(*(task->server_addr)) ) == -1 )
		handle_cr_error("Connecting to server failed");
	{
		int optval = 1;
		if( setsockopt( server_socket, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval)) == -1 )
			handle_cr_error("setsockopt"); 
		optval = 1;
		if( setsockopt( server_socket, IPPROTO_TCP, TCP_KEEPIDLE, &optval, sizeof(optval)) == -1 )
			handle_cr_error("setsockopt"); 
		optval = 1;
		if( setsockopt( server_socket, IPPROTO_TCP, TCP_KEEPINTVL, &optval, sizeof(optval)) == -1 )
			handle_cr_error("setsockopt"); 
		optval = 1;
		if( setsockopt( server_socket, IPPROTO_TCP, TCP_KEEPCNT, &optval, sizeof(optval)) == -1 )
			handle_cr_error("setsockopt");
	}

	server_task_t server_task;
	server_task.from = task->from;
	server_task.to = task->to;
	server_task.delta = task->delta;

	if( send( server_socket, &server_task, sizeof(server_task), 0) == -1 )
		handle_cr_error("Sending task to server failed");

	int recv_bytes;
	recv_bytes = recv( server_socket, &(task->result), sizeof(task->result), 0 );
	if( recv_bytes == -1 )
		handle_cr_error("Recieving result from server failed");
	if( recv_bytes == 0 )
		handle_cr_error("Connection to server failed");

	return NULL;
}


int main(int argc, char const *argv[])
{
	int max_servers = 0;
	if( argc > 1 )
	{
		max_servers = atoi(argv[1]);
		if( max_servers < 1 )
		{
			fprintf(stdout, "Incorrect argument!\nShould be one argument: positive amount of servers\n");
			exit(EXIT_FAILURE);
		}
	}
	printf("***Client started***\n");
	double from = FROM;
	double to = TO;
	double delta = DELTA;

	/*create udp socket*/
	int brcast_socket;
	if( ( brcast_socket = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP ) )== -1 )
		handle_cr_error("Creating broadcast socket failed");

	/*set SO_BROADCAST to upd_sock*/
	int optval = 1;
	if( setsockopt(brcast_socket, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(optval)) == -1 )
		handle_cr_error("Can't set SO_BROADCAST to brcast_socket");

	/*addr broadcast to*/
	struct sockaddr_in brcast_addr;
	brcast_addr.sin_family = AF_INET;
	brcast_addr.sin_port = htons(SND_BRCAST_PORT); /*slave's port*/	
	brcast_addr.sin_addr.s_addr = htonl(INADDR_BROADCAST); /*send to addr*/

	char msg_snd[15];
	memset( &msg_snd, 0, sizeof(char)*15 );
	strcpy( msg_snd, "Hello, server!");
	/*send broadcast request to servers*/
	if( sendto(brcast_socket, &msg_snd, sizeof(char)*15, 0, (struct sockaddr *) &brcast_addr, sizeof(brcast_addr)) == -1 )
		handle_cr_error("Sending brcast request failed");
	printf("Broadcast request is sent\n");
	struct pollfd fds;
	fds.fd = brcast_socket;
	fds.events = POLLIN;	

	/*start recieving answers and making servers' list*/
	int servers_amount = 0;
	int all_threads_amount = 0;
	server_info_t *servers;
	servers = (server_info_t *) malloc( sizeof( server_info_t) * 4 );
	if( servers == NULL )
		handle_cr_error("Malloc failed");
	int servers_array_size	= 4;
	memset(servers, 0, servers_array_size);
	int serv_addr_len = sizeof(struct sockaddr_in);

	server_answer_t msg_rcv;
	printf("Waiting for servers...\n");
	while( poll( &fds, 1, POLL_TIMEOUT) > 0 )
	{
		int recv_bytes = recvfrom( brcast_socket, &msg_rcv, sizeof(msg_rcv), MSG_TRUNC, (struct sockaddr *) &servers[servers_amount].addr, &serv_addr_len );
		if( recv_bytes == sizeof(msg_rcv) && strcmp( msg_rcv.msg, "Hello, client!") && msg_rcv.threads_amount > 0 )
		{
			servers[servers_amount].threads_on_server = msg_rcv.threads_amount;
			all_threads_amount += msg_rcv.threads_amount;
			servers_amount++;
		}
		if( max_servers <= servers_amount && argc > 1)
			break;
		if( servers_amount >= servers_array_size )
		{
			servers = ( server_info_t *) realloc( servers, sizeof(server_info_t)*servers_array_size*2 );
			memset( &servers[servers_amount], 0, sizeof(server_info_t)*servers_array_size);
			servers_array_size *= 2;
			if( servers == NULL )
				handle_cr_error("Realloc failed");
		}
		fds.fd = brcast_socket;
		fds.events = POLLIN;
	}
	if( servers_amount < 1 )
	{
		fprintf(stdout, "No servers were found!\n");
		exit( EXIT_FAILURE);
	}
	else
		printf("Found %d servers\n", servers_amount);

	/*set tasks and start communicators*/
	printf("Send tasks\n");
	communicator_task_t *communicators_tasks = (communicator_task_t *) malloc( sizeof(communicator_task_t)*servers_amount );
	if( communicators_tasks == NULL )
		handle_cr_error("Malloc failed");
	pthread_t *communicators_ids = (pthread_t *) malloc( sizeof(pthread_t) * servers_amount );
	double part_length = (to - from) / all_threads_amount; 
	for( int i = 0; i < servers_amount; i++ )
	{
		communicators_tasks[i].from = from + i*part_length;
		communicators_tasks[i].to = from + (i+servers[i].threads_on_server)*part_length;
		communicators_tasks[i].delta = delta;
		communicators_tasks[i].server_addr = &servers[i].addr;
		from += servers[i].threads_on_server*part_length;
	
		if( pthread_create( &communicators_ids[i], NULL, communicator, (void *) &communicators_tasks[i]) == -1 )
			handle_cr_error("Can't create thread");
	}
	printf("Waiting for results...\n");
	/*wait for communicators threads and sum results*/
	double result;
	for( int i = 0; i < servers_amount; i++ )
	{
		pthread_join( communicators_ids[i], NULL );
		result += communicators_tasks[i].result;
	}

	fprintf(stdout, "Result %g\n", result);

	free(communicators_ids);
	free(servers);

	exit( EXIT_SUCCESS);
}