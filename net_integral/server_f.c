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
#include <ifaddrs.h>
#include <string.h>
#include <arpa/inet.h>

#define SLAVE_PORT 4000
#define LISTEN_PORT 8000
#define TIMEOUT 1 /*seconds*/

#define FROM	-1000
#define TO		10000
#define DELTA 0.000003	

#define handle_error(msg) \
    do { perror(msg); exit(EXIT_FAILURE); } while (0)

struct slave_task
{
	double from;
	double to;
	double delta;
};

struct in_addr getIP();

void disconnect(int signo)
{
	printf("disconnect\n");
	exit( EXIT_FAILURE );
};

double calculate( double from, double to, double delta );

int main( int argc, char * argv[] )
{
	double from = FROM;
	double to = TO;
	double delta = DELTA;

	double result;
	/*start calculations*/
	result = calculate( from, to, delta );

	printf( "\nresult = %g\n", result ); 
	return 0;
}

double calculate( double from, double to, double delta )
{
	/*set action for SIGPIPE*/
	struct sigaction act_disconnect;
  	memset(&act_disconnect, 0, sizeof(act_disconnect));
  	act_disconnect.sa_handler = disconnect;
  	sigfillset(&act_disconnect.sa_mask);
  	sigaction( SIGPIPE, &act_disconnect, NULL );
  	
	/*create listen socket*/
	int listen_socket;
	if( ( listen_socket = socket( AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP ) )== -1 )
		handle_error("socket");
	struct sockaddr_in si_my;
	memset( &si_my, 0, sizeof(si_my) );
	si_my.sin_family = AF_INET;
	si_my.sin_port = htons(LISTEN_PORT); /*my listen port*/
	si_my.sin_addr.s_addr = htonl(INADDR_ANY);
	if( bind( listen_socket, (struct sockaddr*) &si_my, sizeof(si_my) ) == -1 )
		handle_error("bind");
	if( listen( listen_socket, 255 ) == -1 )
		handle_error("listen");
	
	/*create UDP socket*/
	printf("***server started***\n");
	int udp_socket;
	if( ( udp_socket = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP ) )== -1 )
		handle_error("socket");
	
	struct sockaddr_in si_other;
	memset( &si_other, 0, sizeof(si_other) );
	si_other.sin_family = AF_INET;
	si_other.sin_port = htons(SLAVE_PORT); /*slave's port*/	
	si_other.sin_addr.s_addr = htonl(INADDR_BROADCAST); /*send to addr*/

	/*set broadcast flag*/
	int broadcast = 1;
	if( setsockopt( udp_socket, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast) ) == -1 )
		handle_error("setsockopt");
	
	/*broadcast notification*/
	struct in_addr myIP = getIP();
	if( sendto( udp_socket, &myIP, sizeof(myIP), /*0*/0, (struct sockaddr *) &si_other, sizeof(si_other) ) == -1 )
		handle_error("sendto");

	close(udp_socket);
	printf("my IP %s\n", inet_ntoa(myIP));
	printf("broadcast finished\n");
	

	/*listen for connections*/
	int slaves_amount = 0;
	int keepalive = 1;
	int slaves_sockets[255];
	clock_t startTime = clock();
	clock_t currentTime = clock();
	while( currentTime-startTime <= TIMEOUT * CLOCKS_PER_SEC && slaves_amount < 255)
	{
		if( ( slaves_sockets[slaves_amount] = accept( listen_socket, NULL, NULL ) ) == -1 )
			if( errno != EAGAIN && errno != EWOULDBLOCK )
				handle_error("accept");
		
		if( slaves_sockets[slaves_amount] != -1 )
		{
			if( setsockopt( slaves_sockets[slaves_amount], SOL_SOCKET, SO_KEEPALIVE, &keepalive, sizeof(keepalive)) == -1 )
				handle_error("setsockopt"); 
			slaves_amount++;
			printf("connected with %d\n", slaves_amount);
		}

		currentTime = clock();
	}
	printf("\n**start recieving**\n");
	/*get threads amount from each slave*/
	int threads_amount = 0, threads_in_slave[255];
	int ret_val;
	for( int i = 0; i < slaves_amount; i++ )
	{
		if( ( ret_val = recv( slaves_sockets[i], &threads_in_slave[i], sizeof(threads_in_slave[i]), MSG_WAITALL ) ) == -1 )
			handle_error("recv");
		if( ret_val < sizeof(threads_in_slave[i]) )
			disconnect(0);
		printf("recieved from: %d thr_num: %d\n", i+1, threads_in_slave[i]);
		threads_amount += threads_in_slave[i];
	}

	/*set tasks*/
	double part_length = (to - from) / threads_amount; 
	struct slave_task *slaves_tasks = (struct slave_task*) malloc( sizeof(struct slave_task)*slaves_amount );
	double _from = from;
	for( int i = 0; i < slaves_amount; i++ )
	{
		slaves_tasks[i].from = _from + i*part_length;
		slaves_tasks[i].to = _from + (i+threads_in_slave[i])*part_length;
		slaves_tasks[i].delta = delta;
		_from += threads_in_slave[i]*part_length;
	}
	printf("\n**start sending**\n");
	
	/*send tasks*/
	for( int i = 0; i < slaves_amount; i++ )
	{
		if( send( slaves_sockets[i], &slaves_tasks[i], sizeof(slaves_tasks[i]), 0) == -1 )
			handle_error("send");
		printf("send task to %d\n", i+1);
	}
	printf("\n**start recieving**\n");
	
	/*get for results*/
	double result = 0;
	double tmp_result;
	for( int i = 0; i < slaves_amount; i++ )
	{
		if( ( ret_val = recv( slaves_sockets[i], &tmp_result, sizeof(tmp_result), MSG_WAITALL ) ) == -1 )
			handle_error("recv");
		if( ret_val < sizeof(tmp_result) )
			disconnect(0);
		printf("recieved result from %d\n", i);
		result += tmp_result;
		close(slaves_sockets[i]);
	}
	
	return result;
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