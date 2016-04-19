#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <time.h>
#include <math.h>
#include <pthread.h>

/* from -1000 to 10000 */

#define DELTA 0.000001

#define FROM -1000
#define TO 1000

#define f(x) (x)

//double f( double x );

double integr_simp( double from, double to )
;

struct task
{
	double from;
	double to;
};


int startCalculations( double from, double to, int threads_amount );

void * thr_calculate( void * arg );


int main( int argc , char * argv[] )
{
	if( argc != 2  )
	{
		printf("Incorrect input arguments: ");
		printf("must be one integer argument, means the amount of threads\n");
		printf("Thank you for patience!\n");
		exit( EXIT_FAILURE );
	}

	int threads_amount = atoi( argv[1] ); //if argc = 4 must be argv[3]
	
	startCalculations( FROM, TO, threads_amount );	

	return 0;
}


int startCalculations( double from, double to, int threads_amount )
{
	int ret_val;

	double segment_len;
	segment_len = (to-from) / threads_amount;

	struct task thr_task[threads_amount];
    
    pthread_t thread_ids[threads_amount];
    
    //start threads
	for( int i = 0; i < threads_amount; i++ )
	{
		thr_task[i].from = from + i * segment_len;
		thr_task[i].to = from + (i+1) * segment_len;
		ret_val = pthread_create( &thread_ids[i], NULL, thr_calculate, (void *) &thr_task[i] );
		if( ret_val != 0 )
		{
			perror("pthread_create");
			exit( EXIT_FAILURE );
		}
	}

  	double result = 0;
  	void *thr_result;

    //wait for threads
	for( int i = 0; i < threads_amount; i++)
	{
		ret_val = pthread_join( thread_ids[i], &thr_result );
		if( ret_val != 0 )
		{
			perror("pthread_join");
			exit( EXIT_FAILURE );
		}
		result += *( (double *)thr_result );
		free(thr_result);
	}


	//return result
	printf( "\nresult = %g\n", result ); 
	return 0;
};

void * thr_calculate( void * arg )
{
	//handle task
	double from, to;
	from = ( (struct task *) arg ) -> from;
	to = ( (struct task *) arg ) -> to;

	double * integral = (double *) malloc( sizeof(double) );

	int seg_amount;
	seg_amount = (int) ((to-from) / DELTA);
	double last_seg_len;
	last_seg_len = (to-from) - seg_amount * DELTA;

	//start calculations
	for( int i = 0; i < seg_amount; i++ )
		*integral += integr_simp( from + i*DELTA, from + (i+1)*DELTA );	
		
	*integral += integr_simp( to - last_seg_len, to);

	return ( (void *)integral );
};	

/*double f( double x )
{
	double f;
	f = pow( abs(x), 0.5);
	return f;
};*/

double integr_simp( double from, double to )
{
	double integral;
	integral = (to-from)/6 * ( f(from) + 4 * f( (from+to)/2 ) + f(to) );
	return integral;
};