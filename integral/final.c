#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <time.h>
#include <math.h>
#include <pthread.h>

#define FROM	-1000
#define TO		10000

#define DELTA 0.000003	

#define f(x) x

struct thread_task
{
	double result;
	double from;
	double to;
	double delta;
};

double integr_simp( double from, double to );

void * slave( void * task );

double calculate( int slaves_amount, double from, double to, double delta );

int main( int argc, char * argv[] )
{
	if( argc != 2 )
	{
		printf("Incorrect arguments!\nMust be one argument, the amount of slaves.\n");
		exit( EXIT_FAILURE );
	}
	int slaves_amount = atoi( argv[1] );
	double from = FROM;
	double to = TO;
	double delta = DELTA;

	double result;
	result = calculate( slaves_amount, from, to, delta );

	/*int ret_val;
	
	double part_length = (to - from) / slaves_amount; 

	pthread_t thread_ids[slaves_amount];
	struct thread_task thread_tasks[slaves_amount];

	for( int i = 0; i < slaves_amount; i++ )
	{
		thread_tasks[i].from = from + i*part_length;
		thread_tasks[i].to = from + (i+1)*part_length;
		thread_tasks[i].delta = delta;
		ret_val = pthread_create( &thread_ids[i], NULL, slave, (void *) &thread_tasks[i] );
		if( ret_val != 0 )
		{
			perror("pthread_create");
			exit( EXIT_FAILURE );
		}
	}

	//wait for threads
	for( int i = 0; i < slaves_amount; i++ )
	{
		ret_val = pthread_join( thread_ids[i], NULL );
		if( ret_val != 0 )
		{
			perror("pthread_join");
			exit( EXIT_FAILURE );
		}
	}

	double result = 0;
	
	for( int i = 0; i < slaves_amount; i++ )
		result += thread_tasks[i].result;*/

	printf( "\nresult = %g\n", result ); 
	return 0;
}


void * slave( void * task )
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


double calculate( int slaves_amount, double from, double to, double delta )
{
	double part_length = (to - from) / slaves_amount; 

	pthread_t thread_ids[slaves_amount];
	struct thread_task thread_tasks[slaves_amount];

	int ret_val;
	for( int i = 0; i < slaves_amount; i++ )
	{
		thread_tasks[i].from = from + i*part_length;
		thread_tasks[i].to = from + (i+1)*part_length;
		thread_tasks[i].delta = delta;
		ret_val = pthread_create( &thread_ids[i], NULL, slave, (void *) &thread_tasks[i] );
		if( ret_val != 0 )
		{
			perror("pthread_create");
			exit( EXIT_FAILURE );
		}
	}

	//wait for threads
	for( int i = 0; i < slaves_amount; i++ )
	{
		ret_val = pthread_join( thread_ids[i], NULL );
		if( ret_val != 0 )
		{
			perror("pthread_join");
			exit( EXIT_FAILURE );
		}
	}

	double result = 0;
	
	for( int i = 0; i < slaves_amount; i++ )
		result += thread_tasks[i].result;

	return result;
};