#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include <sys/types.h>
#include <time.h>
#include <math.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <ifaddrs.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>

#define SLAVE_PORT 4000
#define LISTEN_PORT 8000

#define f(x) x

#define handle_error(msg) \
    do { perror(msg); exit(EXIT_FAILURE); } while (0)


void disconnect(int signo)
{
	printf("disconnect\n");
	exit( EXIT_FAILURE );
};

struct thread_task
{
	double result;
	double from;
	double to;
	double delta;
};

struct slave_task
{
	double from;
	double to;
	double delta;
	int threads_amount;
};

struct in_addr getIP();

double integr_simp( double from, double to );

void * thread( void * task );

double calculate( int threads_amount, double from, double to, double delta );

int main( int argc, char * argv[] )
{
	if( argc != 3 )
	{
		printf("Incorrect arguments!\nMust be two arguments: port and the amount of threads.\n");
		exit( EXIT_FAILURE );
	}
	int threads_amount = atoi( argv[2] );
	
	/*set action for SIGPIPE*/
	struct sigaction act_disconnect;
  	memset(&act_disconnect, 0, sizeof(act_disconnect));
  	act_disconnect.sa_handler = disconnect;
  	sigfillset(&act_disconnect.sa_mask);
  	sigaction( SIGPIPE, &act_disconnect, NULL );

	/*create UDP socket*/
	printf("***slave started***\n");
	int udp_socket;
	if( ( udp_socket = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP ) )== -1 )
		handle_error("socket");
	
	struct sockaddr_in si_me;
	memset( &si_me, 0, sizeof(si_me) );
	si_me.sin_family = AF_INET;
	si_me.sin_port = htons(atoi(argv[1]));//PORT); /*my port*/
	si_me.sin_addr.s_addr = htonl(INADDR_ANY); /*recieve from addr*/

	if( bind( udp_socket, (struct sockaddr *) &si_me, sizeof(si_me) ) == -1 )
		handle_error("bind");
	
	/*recieve server IP from UDP socket*/
	struct in_addr serverIP;
	socklen_t addrlen = sizeof(si_me);
	if(	recvfrom( udp_socket, &serverIP, sizeof(serverIP), 0, (struct sockaddr *) &si_me, &addrlen ) == -1 )
		handle_error("recvfrom");
	printf("recieved server IP %s\n", inet_ntoa(serverIP));
	
	close(udp_socket);
	
	/*create TCP socket*/
	int tcp_socket;
	if( ( tcp_socket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP ) )== -1 )
		handle_error("socket");
	struct sockaddr_in si_serv;
	memset( &si_serv, 0, sizeof(si_serv) );
	si_serv.sin_family = AF_INET;
	si_serv.sin_port = htons(LISTEN_PORT); /*my port*/
	si_serv.sin_addr = serverIP; /*serv  addr*/

	/*set socket unbreakable*/
	int optval = 1;
	if( setsockopt( tcp_socket, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval)) == -1 )
		handle_error("setsockopt"); 
	optval = 1;
	if( setsockopt( tcp_socket, IPPROTO_TCP, TCP_KEEPIDLE, &optval, sizeof(optval)) == -1 )
		handle_error("setsockopt"); 
	optval = 1;
	if( setsockopt( tcp_socket, IPPROTO_TCP, TCP_KEEPINTVL, &optval, sizeof(optval)) == -1 )
		handle_error("setsockopt"); 
	optval = 1;
	if( setsockopt( tcp_socket, IPPROTO_TCP, TCP_KEEPCNT, &optval, sizeof(optval)) == -1 )
		handle_error("setsockopt");

	/*try to connect with server*/
	if( connect( tcp_socket, (struct sockaddr *) &si_serv, sizeof(si_serv) ) == -1 )
		handle_error("connect");

	printf("connected\n");
	
	printf("\n**start sending**\n");
	
	/*send amount of threads to server*/
	if( send(tcp_socket, &threads_amount, sizeof(threads_amount), 0) == -1 )
		handle_error("send");
	printf("send thr_num %d\n", threads_amount);
	printf("\n**start recieving**\n");
	
	/*get task*/
	int ret_val;
	struct slave_task task;
	if( ( ret_val = recv( tcp_socket, &task, sizeof(task), MSG_WAITALL ) ) == -1 )
		handle_error("recv");
	if( ret_val < sizeof(task) )
		disconnect(0);
	printf("recieved task\n");

	/*create pipe*/
	int pipefd[2];
	if( pipe(pipefd) == -1 )
		handle_error("pipe");

	/*start calculations*/
	task.threads_amount = threads_amount;
	pthread_t calc_thr_id;
	if( pthread_create( &calc_thr_id, NULL, calculate, (void *) &task ) == -1 )
		handle_error("pthread_create");
//todo
	double result;
	result = calculate( threads_amount, from, to, delta );
	printf("\n**start sending**\n");
	
	/*send reslult back to server*/
	if( send(tcp_socket, &result, sizeof(result), 0) == -1 )
		handle_error("send");
	printf("sent result\n");
	/*close connection*/
	close(tcp_socket);
	exit( EXIT_SUCCESS );
	return 0;
}


void * thread( void * task )
{
	double to = ((struct thread_task *)task )->to;
    double from = ((struct thread_task *)task )->from;
	double delta = ((struct thread_task *)task )->delta;
    
	double result = 0;

	for( ; from < to; from += delta )
		result += integr_simp( from, to );

	( (struct thread_task *)task )->result = result;
	return NULL;
};


double integr_simp( double from, double to )
{
	double integral;
	integral = (to-from)/6 * ( f(from) + 4 * f( (from+to)/2 ) + f(to) );
	return integral;
};
void *calculate( void * task )
{
	int threads_amount = ((struct slave_task *)task )->threads_amount;
	double from = ((struct slave_task *)task )->from;
	double to = ((struct slave_task *)task )->to;
	double delta = ((struct slave_task *)task )->delta;

	double part_length = (to - from) / threads_amount; 

	pthread_t thread_ids[threads_amount];
	struct thread_task thread_tasks[threads_amount];

	int ret_val;
	for( int i = 0; i < threads_amount; i++ )
	{
		thread_tasks[i].from = from + i*part_length;
		thread_tasks[i].to = from + (i+1)*part_length;
		thread_tasks[i].delta = delta;
		ret_val = pthread_create( &thread_ids[i], NULL, thread, (void *) &thread_tasks[i] );
		if( ret_val != 0 )
		{
			perror("pthread_create");
			exit( EXIT_FAILURE );
		}
	}

	//wait for threads
	for( int i = 0; i < threads_amount; i++ )
	{
		ret_val = pthread_join( thread_ids[i], NULL );
		if( ret_val != 0 )
		{
			perror("pthread_join");
			exit( EXIT_FAILURE );
		}
	}

	double result = 0;
	
	for( int i = 0; i < threads_amount; i++ )
		result += thread_tasks[i].result;

	write( pipefd[1], &result, sizeof(result) );
};

struct in_addr getIP()
{
    struct in_addr myIP;
    struct ifaddrs *ifAddrStruct = NULL;
    struct ifaddrs *ifa = NULL;
 
    getifaddrs(&ifAddrStruct);

    for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa ->ifa_addr->sa_family==AF_INET) { // Check it is
            // a valid IPv4 address
            if( !( ifa->ifa_name[0] == 'l' && ifa->ifa_name[1] == 'o') )
            {
                myIP = ((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
                break;
            }
        }
    }
    if (ifAddrStruct != NULL)
        freeifaddrs(ifAddrStruct);
    return myIP;
}
