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

#define RCV_BRCAST_PORT 8000

#define f(x) x

#define handle_cr_error(msg) \
    do { fprintf(stderr, "%s\n", msg); perror("Details"); exit(EXIT_FAILURE); } while (0)

#define handle_error(msg) \
    do { fprintf(stderr, "%s\n", msg); perror("Details"); } while (0)

typedef struct 
{
	int *may_answer;
	int slaves_amount;	
} communicator_task_t;

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
	double result;
	double from;
	double to;
	double delta;
} slave_task_t;

typedef struct 
{
	int socket;
	pthread_t *slave_threads;
	int slaves_amount;
} checker_task_t;

double integr_simp( double from, double to )
{
	double integral;
	integral = (to-from)/6 * ( f(from) + 4 * f( (from+to)/2 ) + f(to) );
	return integral;
};

/*func for thread answers to broadcast requests*/
void *brcast_communicator( void *arg )
{
	communicator_task_t *task = (communicator_task_t *) arg;
	int *may_answer = task->may_answer;
	int slaves_amount = task->slaves_amount;

	/*create UDP socket*/
	int brcast_socket;
	if( ( brcast_socket = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP ) )== -1 )
		handle_cr_error("Creating broadcast socket failed");
	/*bind UDP socket*/
	struct sockaddr_in addr_bcast;
	memset( &addr_bcast, 0, sizeof(addr_bcast) );
	addr_bcast.sin_family = AF_INET;
	addr_bcast.sin_port = htons(RCV_BRCAST_PORT); /*my port*/
	addr_bcast.sin_addr.s_addr = htonl(INADDR_ANY); /*recieve from addr*/
	if( bind( brcast_socket, (struct sockaddr *) &addr_bcast, sizeof(addr_bcast) ) == -1 )
		handle_cr_error("Bind broadcast socket failed");

	/*set bcast socket nonblock*/
	//fcntl(brcast_socket, F_SETFL, O_NONBLOCK);

	struct pollfd fds;
	fds.fd = brcast_socket;
	fds.events = POLLIN;

	size_t msg_size = sizeof(char)*15;
	char msg_rcv[15];
	memset( &msg_rcv, 0, msg_size );
	server_answer_t msg_snd;
	memset( &msg_snd.msg, 0, msg_size );
	strcpy( msg_snd.msg, "Hello, client!");
	msg_snd.threads_amount = slaves_amount;
	size_t rcv_size;

	struct sockaddr_in client_addr;
	socklen_t client_addr_len = sizeof(client_addr);
 	memset( &client_addr, 0, sizeof(client_addr) );

	while(1)
	{
		if( poll( &fds, 1, -1 ) == -1 )
			handle_cr_error("Poll broadcast socket failed");
		/*recieve from UDP socket*/
		if( (rcv_size = recvfrom( brcast_socket, msg_rcv, msg_size, MSG_TRUNC, (struct sockaddr*) &client_addr, &client_addr_len )) == -1 )
			handle_cr_error("Recv from broadcast socket failed");
		/*answer*/
		if( rcv_size == 15 && strcmp( msg_rcv, "Hello, server!") == 0 && *may_answer == 1 )
		{	
			if( sendto( brcast_socket, &msg_snd, sizeof(msg_snd), 0, (struct sockaddr*) &client_addr, client_addr_len ) == -1 )
				handle_cr_error("Sendto from broadcast socket");
		}
		fds.fd = brcast_socket;
		fds.events = POLLIN;
	}
	return NULL;
}

/*func for thread checks if TCP socket is OK*/
void *checker( void *arg )
{
	checker_task_t *task = (checker_task_t *) arg;

	char msg[20];
	recv( task->socket, &msg, sizeof(char) * 20, 0 );

	for( int i = 0; i < task->slaves_amount; i++ )
		pthread_cancel( task->slave_threads[i] );

	return NULL;
}

/*func for thread calculates integral*/
void *slave( void *arg )
{
	slave_task_t *task = (slave_task_t*) arg;
	double from = task->from;
	double to = task->to;
	double delta = task->delta;
	double result = 0;

	for( ; from < to; from += delta )
		result += integr_simp( from, from + delta );

	task->result = result;
	return NULL;
}



int main( int argc, char *argv[] )
{
	if( argc != 2 )
	{
		fprintf(stdout, "Incorrect arguments! Must be one: amount of threads.\n");
		exit( EXIT_FAILURE );
	}

	int slaves_amount = atoi( argv[1] );
	if( slaves_amount < 1 )
	{
		fprintf(stdout, "Incorrect thread's amount\n");
		exit( EXIT_FAILURE );
	} 
	printf("***Server started***\n");
	/*start thread to be answering to broadcast requests*/
	int may_answer = 0; 
	pthread_t broadcast_thr;
	communicator_task_t communicator_task;
	communicator_task.may_answer = &may_answer;
	communicator_task.slaves_amount = slaves_amount;
	if( pthread_create( &broadcast_thr, NULL, brcast_communicator, (void *) &communicator_task ) == -1 )
		handle_cr_error("Can't create broadcast thread");

	/*create listen socket*/
	int listen_socket;
	if( ( listen_socket = socket( AF_INET, SOCK_STREAM, 0 ) )== -1 )
		handle_cr_error("Create listen socket failed");

	/*set SO_REUSEADDR to listen_socket*/
	int optval=1;
	if( setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1 )
		handle_cr_error("Can't set SO_REUSEADDR to listen_socket");

	/*bind listen socket*/
	struct sockaddr_in addr;
	memset( &addr, 0, sizeof(addr) );
	addr.sin_family = AF_INET;
	addr.sin_port = htons(RCV_BRCAST_PORT); /*my listen port*/
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	if( bind( listen_socket, (struct sockaddr*) &addr, sizeof(addr) ) == -1 )
		handle_cr_error("Bind listen socket failed");

	/*start waiting for connections with client_sockets*/
	if(listen(listen_socket, 1) < 0)
		handle_cr_error("Listen failed");
	may_answer = 1;

	/*wait for connections*/
	int client_socket;
	int bytes;
	int flag = 1;
	struct sockaddr_in client_socket_addr;
	socklen_t addr_len = sizeof(client_socket_addr);
	memset( &client_socket_addr, 0, sizeof(client_socket_addr) );
	while(1)
	{
		printf("Waiting for connections...\n");
		flag = 1;
		/*accept for connection*/
		if( (client_socket = accept( listen_socket, (struct sockaddr *) &client_socket_addr, &addr_len )) == -1 )
			handle_cr_error("Accept error");
		may_answer = 0; // stop answering broadcast requests
		printf("Сonnection is established\n");
		/*set KEEPALIVE flag*/
		{
			int optval = 1;
			if( setsockopt( client_socket, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval)) == -1 )
				handle_cr_error("setsockopt"); 
			optval = 1;
			if( setsockopt( client_socket, IPPROTO_TCP, TCP_KEEPIDLE, &optval, sizeof(optval)) == -1 )
				handle_cr_error("setsockopt"); 
			optval = 1;
			if( setsockopt( client_socket, IPPROTO_TCP, TCP_KEEPINTVL, &optval, sizeof(optval)) == -1 )
				handle_cr_error("setsockopt"); 
			optval = 1;
			if( setsockopt( client_socket, IPPROTO_TCP, TCP_KEEPCNT, &optval, sizeof(optval)) == -1 )
				handle_cr_error("setsockopt");
		}
		
		server_task_t task;
		memset( &task, 0, sizeof(task) );
		/*recieve task*/
		int bytes = recv( client_socket, &task, sizeof(task), 0);
		if( bytes == -1 )
		{
			handle_error("Client disconnected");
			close(client_socket);
			flag = 0;
			may_answer = 1;
		}
		/*check task data*/
		if( task.from > task.to || bytes != sizeof(task) || task.delta <= 0 )
		{
			fprintf(stderr, "Invalid data recieved from client\n");
			close(client_socket);
			flag = 0;
			may_answer = 1;
		}
		printf("Task recieved\nStart calculations...");
		/*calculations*/
		if( flag )
		{
			double part_length = (task.to - task.from) / slaves_amount; 

			pthread_t *slaves_ids = (pthread_t *) malloc( sizeof(pthread_t) * slaves_amount );
			slave_task_t *slaves_tasks = (slave_task_t *) malloc( sizeof(slave_task_t) * slaves_amount );

			/*set tasks and create slaves threads*/
			int ret_val;
			for( int i = 0; i < slaves_amount; i++ )
			{
				slaves_tasks[i].from = task.from + i*part_length;
				slaves_tasks[i].to = task.from + (i+1)*part_length;
				slaves_tasks[i].delta = task.delta;
				if( pthread_create( &slaves_ids[i], NULL, slave, (void *) &slaves_tasks[i] ) != 0 )
					handle_cr_error("Can't create slave thread");
			}

			/*start checker thread*/
			pthread_t checker_id;
			checker_task_t checker_task;
			checker_task.socket = client_socket;
			checker_task.slave_threads = slaves_ids;
			checker_task.slaves_amount = slaves_amount;
			if( pthread_create( &checker_id, NULL, checker, (void *) &checker_task ) != 0 )
				handle_cr_error("Can't create checker thread");

			/*wait for threads*/
			int slaves_canceled = 0;
			int *exit_status;
			for( int i = 0; i < slaves_amount; i++ )
			{
				pthread_join( slaves_ids[i], (void *) &exit_status);
				if( exit_status == PTHREAD_CANCELED )
					slaves_canceled = 1; //slaves were canceled
			}
			/*check why slaves exited*/			
			if( slaves_canceled == 1 )
			{
				handle_error("Client disconnected");
				close(client_socket);
				flag = 0;
				may_answer = 1;
				free(slaves_ids);
				free(slaves_tasks);
			}
			else
			{
				double result;
				for( int i = 0; i < slaves_amount; i++ )
					result += slaves_tasks[i].result;
				printf("Calculations finished\n");

				pthread_cancel( checker_id );
				/*send result*/
				bytes = send( client_socket, &result, sizeof(result), 0);
				if( bytes != sizeof(result) )
				{
					handle_error("Send to client failed");
					close(client_socket);
					flag = 0;
				}
				else
					printf("Result is sent\n");
				may_answer = 1;
				free(slaves_tasks);
				free(slaves_ids);
				close(client_socket);
			}
		}
	}	
	exit( EXIT_SUCCESS);
}