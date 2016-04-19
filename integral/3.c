#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>
#include <math.h>
#include <pthread.h>

/* from -10000 to 10000 */

#define DELTA 0.00005
/*
double f( double x );

double integr_simp( double x1, double x2, double (*f) ( double x ) );
*/

struct rimeval t1, t2;

struct task
{
	double x1;
	double x2;
};


int startCalculations( double x1, double x2, int calcs_amount);

void * thr_calculate( void * arg );


/* argv:
		1 - lower limit
		2 - upper limit
		3 - amount of calculators
*/
int main( int argc , char * argv[] )
{
	if( argc != 4 )
	{
		printf("incorrect input arguments\n");
		exit( EXIT_FAILURE );
	}

	
	
	gettimeofday(&t1, NULL); 
	printf("start s%ld ms%ld\n", t1.tv_sec, t1.tv_usec);

	double x1 = (double) atoi( argv[1] );
	double x2 = (double) atoi( argv[2] );
	int calcs_amount = atoi( argv[3] );
	

	startCalculations( x1, x2, calcs_amount);

	return 0;
}


int startCalculations( double x1, double x2, int calcs_amount)
{
	int ret_val;

	double thr_seg_len;
	thr_seg_len = (x2-x1) / calcs_amount;

	struct task thr_task[calcs_amount];
    
    pthread_t tid[calcs_amount];


	gettimeofday(&t1, NULL); 
	printf("thr_start s%ld ms%ld\n", t1.tv_sec, t1.tv_usec);
	//start threads
	for( int i = 1; i < calcs_amount; i++ )
	{
		thr_task[i].x1 = x1 + i * thr_seg_len;
		thr_task[i].x2 = x1 + (i+1) * thr_seg_len;

		ret_val = pthread_create( &tid[i], NULL, thr_calculate, &thr_task[i] );
		if( ret_val != 0 )
		{
			perror("pthread_create");
			exit( EXIT_FAILURE );
		}
	}

  	double result = 0;
	void * rval_ptr;

    //do own work

	thr_task[0].x1 = x1;
	thr_task[0].x2 = x1 + thr_seg_len;
	
	rval_ptr = thr_calculate( &thr_task[0]);

	result += * ( (double *) rval_ptr);
	free(rval_ptr);

    //wait for threads
	for( int i = 1; i < calcs_amount; i++)
	{
		ret_val = pthread_join( tid[i], &rval_ptr );
		if( ret_val != 0 )
		{
			perror("pthread_join");
			exit( EXIT_FAILURE );
		}
		
		result += * ( (double *) rval_ptr);
		free(rval_ptr);
	}

	gettimeofday(&t2, NULL); 
	printf("finish s%ld ms%ld\n", t2.tv_sec, t2.tv_usec);
	

	printf( "\nresult = %g\n", result ); 
	return 0;
};

void * thr_calculate( void * arg )
{
	//handle task
	double x1, x2;
	x1 = ( (struct task *) arg ) -> x1;
	x2 = ( (struct task *) arg ) -> x2;

	double * integral;
	integral = (double * ) malloc( sizeof(double) );
	if( integral == NULL )
	{
		perror("malloc");
		exit( EXIT_FAILURE );
	}

	int seg_amount;
	seg_amount = (int) ((x2-x1) / DELTA);
	//double last_seg_len;
	//last_seg_len = (x2-x1) - seg_amount * DELTA;

	//start calculations
	printf("\nseg_amount = %d\n", seg_amount);
	for( int i = 0; i < seg_amount; i++ )
	{
		*integral += (DELTA)/6 * ( pow( abs(x1 + i*DELTA), 0.5) + 4 * pow( abs( x1 + i*DELTA+DELTA/2 ), 0.5) + pow( abs(x1 + (i+1)*DELTA), 0.5) );
	}
	//*integral += integr_simp( x2 - last_seg_len, x2, f);

	return (void *) integral;
};	
/*
double f( double x )
{
	double f;
	f = pow( abs(x), 0.5);
	return f;
};

double integr_simp( double x1, double x2, double (*f) ( double x ) )
{
	double integral;
	integral = (x2-x1)/6 * ( (*f)(x1) + 4 * (*f)( (x1+x2)/2 ) + (*f)(x2) );
	return integral;
}; */