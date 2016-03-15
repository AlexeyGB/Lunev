#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <sys/types.h>
#include <sys/select.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/signal.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define PIPE_SIZE 65536

struct child_t
{
	int num; //num from 0 to child_amount - 1
	int first; // if child is first
	int last; // if child is last

    // fds for child
	int read_fd; 
	int write_fd;
	// fds for parent
	int prnt_read_fd;
	int prnt_write_fd;	
};

struct buffer_t
{
	char *buf; 
	int size; // buffer size
	int filled; // size of filled space
};

int getnum( char *str );

struct buffer_t *getBufs( int child_amount );

void freeBuf( struct buffer_t *buf );

void childStartTransmiting( struct child_t *child );

void handleReadableFd( struct child_t *child, fd_set *read_fds, 
					 fd_set *read_fds_dflt, fd_set *write_fds_dflt,
					 struct buffer_t *buf);

void handleWriteableFd( struct child_t *child, fd_set *write_fds, 
					 fd_set *write_fds_dflt, fd_set *read_fds_dflt,
					 struct buffer_t *buf);

//---------------------------------------
// MAIN PART

int main( int argc, char *argv[] )
{
	if( argc != 3 )
	{
		fprintf(stderr, "bad arguments\n" );
		exit( EXIT_FAILURE );
	}		

	int child_amount;
	child_amount = getnum( argv[1] );

	// config children's structures
	// make fds between children and parent
	int i, j;
	int pipe_fds[2];
	int max_fd = 1;
	struct child_t children[ child_amount ];
	
	for( i = 0; i < child_amount; i++ )
	{
		children[i].num = i;
		if( i == child_amount - 1)
		{
			children[i].last = 1;
			children[i].prnt_write_fd = -1;
		}
		else
			children[i].last = 0;

		if( i != 0 )
		{
			children[i].first = 0;

			if( pipe( pipe_fds ) == -1 )
			{
				perror("pipe");
				exit( EXIT_FAILURE );
			}

			if( pipe_fds[0] > max_fd )
				max_fd = pipe_fds[0];
			if( pipe_fds[1] > max_fd )
				max_fd = pipe_fds[1];

			children[i].read_fd = pipe_fds[0];
			children[i-1].prnt_write_fd = pipe_fds[1];
			fcntl( children[i-1].prnt_write_fd, F_SETFL, O_NONBLOCK );
		}
		else
		{
			children[i].first = 1;
			children[i].read_fd = -1;
		}

		if( pipe( pipe_fds ) == -1 )
		{
			perror("pipe");
			exit( EXIT_FAILURE );
		}

		if( pipe_fds[0] > max_fd ) 
			max_fd = pipe_fds[0];
		if( pipe_fds[1] > max_fd )
			max_fd = pipe_fds[1];

		children[i].write_fd = pipe_fds[1];
		children[i].prnt_read_fd = pipe_fds[0];
		fcntl( children[i].prnt_read_fd, F_SETFL, O_NONBLOCK );  
	}

	//start fororking
	pid_t pid;
	for( i = 0; i < child_amount; i++ )
	{
		if( pid = fork() == -1 )
		{
			perror("fork");
			exit( EXIT_FAILURE );
		}

		//child
		if( pid == 0 )
		{
			//close unused fds
			for( j = 0; j < child_amount; j++ )
			{
				printf("\ni = %d\n", i);
				close(children[j].prnt_read_fd);
				if( j != child_amount - 1)
					close(children[j].prnt_write_fd);
				
				if( j != i )
				{
					if( j != 0 )
						close(children[j].read_fd);
					close(children[j].write_fd);
				}
			}

			//start work
			childStartTransmiting( &children[i], );
			break;
		}

		//parent
		else
		{
			//close unused fds
			if( i != 0)
				close(children[i].read_fd);
			close(children[i].write_fd);
			break;
		}
	}

	//parent
	// read and write fds
	fd_set read_fds;
	fd_set write_fds;
	// default read and write fds
	fd_set read_fds_dflt;
	fd_set write_fds_dflt;

	FD_ZERO( &read_fds_dflt );
	FD_ZERO( &write_fds_dflt );
	for( i = 0; i < child_amount; i++ )
		FD_SET( children[i].prnt_read_fd, &read_fds_dflt );

	struct buffer_t *bufs;
	bufs = getBufs( child_amount );
	while(1)
	{
		read_fds = read_fds_dflt;
		write_fds = write_fds_dflt;
		if( select( max_fd + 1, read_fds, write_fds, NULL, NULL ) == -1 )
		{
			perror("select");
			exit( EXIT_FAILURE );
		}

		for( i = 0; i < child_amount; i++ )
		{
			handleReadableFd( &children[i], &read_fds, 
							  &read_fds_dflt, &write_fds_dflt, &bufs[i]);

			handleWriteableFd( &children[i], &write_fds,
							   &write_fds_dflt, &read_fds_dflt, &bufs[i]);
		}


	}
	



}

int getnum( char *str )
{
	if( str == NULL )
	{
		fprintf( stderr, "bad first argument\n" );
		exit( EXIT_FAILURE );
	}	

	int num;
	num = atoi( str );
	
	if( num <= 0 )
	{
		fprintf( stderr, "bad first argument\n" );
		exit( EXIT_FAILURE );		
	}

	return num;
}

struct buffer_t *getBufs( int child_amount )
{
	if( child_amount <= 0 )
	{
		assert(0);
		exit( EXIT_FAILURE );
	}

	struct buffer_t *bufs;
	bufs = ( struct buffer_t *) malloc( sizeof( struct buffer_t ) * child_amount );
	if( bufs == NULL )
	{
		perror( "malloc" );
		exit( EXIT_FAILURE );
	}

	int i;
	int buf_size;
	for( i = 0; i < child_amount; i++ )
	{
		buf_size = (int) ( sizeof( char) * (pow( 3, child_amount ) - i) * 4 );
		bufs[i].buf = (char *) malloc( (size_t) buf_size );
		
		if( bufs[i].buf == NULL )
		{
			perror( "malloc" );
			exit( EXIT_FAILURE );			
		}

		bufs[i].size = buf_size;
		bufs[i].filled = 0;
	}

	return bufs;
}

void freeBuf( struct buffer_t *buf )
{
	if( bufs == NULL )
	{
		assert(0);
		exit( EXIT_FAILURE );
	}

	free( buf -> buf );
	
	free( buf );
}

void childStartTransmiting( struct child_t *child )
{
	if( child == NULL )
	{
		assert(0);
		exit(EXIT_FAILURE);
	}
	int ret_val;
	int write_fd, read_fd;
	if( child->num == 0 )
	{
		read_fd = open( argv[2], O_RDONLY );
		if( read_fd == -1 )
		{
			perror("open file");
			exit( EXIT_FAILURE );
		}
		if( read_fd == 0 )
		{
			exit( EXIT_FAILURE );
		}
	}
	else
		read_fd = child->read_fd;
	write_fd = child->write_fd;
	
	char buf[PIPE_SIZE];

	while(1)
	{
		ret_val = read( read_fd, &buf, PIPE_SIZE );
		if( ret_val == 0 )
			break;
		if( ret_val == -1 )
		{
			perror("child read");
			exit( EXIT_FAILURE );
		}

		ret_val	= write( write_fd, &buf, ret_val );
		if( ret_val == -1 )
		{
			if( errno == EPIPE)
				break;
			perror("child write");
			exit( EXIT_FAILURE );
		}
	}
	close(write_fd);
	close(read_fd);
	exit( EXIT_SUCCESS );
}

void handleReadableFd( struct child_t *child, fd_set *read_fds, 
					 fd_set *read_fds_dflt, fd_set *write_fds_dflt,
					 struct buffer_t *buf)
{
	int prnt_read_fd = child -> prnt_read_fd;
	int prnt_write_fd = child -> prnt_write_fd;

	if( FD_ISSET( prnt_read_fd, read_fds ) )
	{
		int ret_val;
		// last child
		if( child -> last )
		{
			ret_val = read( prnt_read_fd, buf -> buf, buf -> size);
			if( ret_val == -1 && errno == EAGAIN )
			{
				close(  prnt_read_fd );
				close(  prnt_write_fd );
				FD_CLR( prnt_read_fd, read_fds_dflt );
				FD_CLR( prnt_write_fd, write_fds_dflt );
				freeBuf( buf );

				return;	
			}
			if( ret_val == -1 )
			{
				perror("parent read");
				exit( EXIT_FAILURE );
			}
			if( ret_val == 0 )
			{
				exit( EXIT_SUCCESS );
			}

			write( 1, buf -> buf, ret_val);

			return;
		}

		// buf is empty
		if( buf -> filled == 0 )
		{
			if( buf -> size >= PIPE_SIZE )
				ret_val = read( prnt_read_fd, buf -> buf, PIPE_SIZE );	
			else
				ret_val = read( prnt_read_fd, buf -> buf, buf -> size);
			
			if( ret_val == -1 && errno == EAGAIN )
			{
				close(  prnt_read_fd );
				close(  prnt_write_fd );
				FD_CLR( prnt_read_fd, read_fds_dflt );
				FD_CLR( prnt_write_fd, write_fds_dflt );
				freeBuf( buf );

				return;	
			}
			if( ret_val == -1 )
			{
				perror("parent read");
				exit( EXIT_FAILURE );
			}
			if( ret_val == 0 )
			{
				close(  prnt_read_fd );
				close(  prnt_write_fd );
				FD_CLR( prnt_read_fd, read_fds_dflt );
				FD_CLR( prnt_write_fd, write_fds_dflt );
				freeBuf( buf );

				return;
			}

			
		}
		// buf isn't empty
		else
		{	
			if( buf -> size - buf -> filled >= PIPE_SIZE )
				ret_val = read( prnt_read_fd, buf -> buf, PIPE_SIZE );
			else
				ret_val = read( prnt_read_fd, buf -> buf, buf -> size - buf -> filled );
			
			if( ret_val == -1 && errno == EAGAIN )
			{
				close(  prnt_read_fd );
				close(  prnt_write_fd );
				FD_CLR( prnt_read_fd, read_fds_dflt );
				FD_CLR( prnt_write_fd, write_fds_dflt );
				freeBuf( buf );

				return;	
			}
			if( ret_val == -1 )
			{
				perror("parent write");
				exit( EXIT_FAILURE );
			}
			
			if( ret_val == 0 )
			{
				FD_CLR( prnt_read_fd, read_fds_dflt );
				return;
			}
		}

		buf -> filled += ret_val;
		FD_SET( prnt_write_fd, write_fds_dflt );
	}

}

void handleWriteableFd( struct child_t *child, fd_set *write_fds, 
					 fd_set *write_fds_dflt, fd_set *read_fds_dflt,
					 struct buffer_t *buf)
{
	int prnt_read_fd = child -> prnt_read_fd;
	int prnt_write_fd = child -> prnt_write_fd;

	if( FD_ISSET( prnt_write_fd, write_fds ) )
	{
		int ret_val;

		if( buf -> filled >= PIPE_SIZE )
			ret_val = write( prnt_write_fd, but -> buf, PIPE_SIZE );
		else
			ret_val = write( prnt_write_fd, buf -> buf, buf -> filled );

		if( ret_val == -1 && errno == EAGAIN )
		{
			close(  prnt_read_fd );
			close(  prnt_write_fd );
			FD_CLR( prnt_read_fd, read_fds_dflt );
			FD_CLR( prnt_write_fd, write_fds_dflt );
			freeBuf( buf );
			return;	
		}
		if( ret_val == -1 )
		{
			perror("parent write");
			exit( EXIT_FAILURE );
		}
		buf -> filled -= ret_val;

		if( buf -> filled == 0 )
		{
			// buf is empty now
			FD_CLR( prnt_write_fd, write_fds_dflt );
		}
		else
		{
			// some data is still in buf
			int k;
			char *offset_buf = buf -> buf + ret_val;
			for( k = 0; k < buf -> filled; k++ )
			{
				buf -> buf[k] = offset_buf[k];
			}
		}
		FD_SET( prnt_read_fd, read_fds_dflt );
	}

}
