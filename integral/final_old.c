#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <time.h>
#include <math.h>


double f( double x );

struct exp_result
{
	double integr_val;
	clock_t exp_time;
};

int startCalculations( double x1, double x2, int calcs_amount, struct exp_result * result );

double calculate( double x1, double x2, int seg_amount );


/* argv:
		1 - lower limit
		2 - upper limit
		3 - amount of calculators
		4 - number of starts
*/
int main( int argc , char * argv[] )
{
	if( argc != 5 )
	{
		printf("incorrect input arguments\n");
		exit(EXIT_FAILURE);
	}

	clock_t max_time = 0;
	clock_t min_time = 0;
	clock_t ave_time = 0;

	struct exp_result result;

	double x1 = (double) atoi( argv[1] );
	double x2 = (double) atoi( argv[2] );
	int calcs_amount = atoi( argv[3] );
	int starts_num = atoi( argv[4] );

	startCalculations( x1, x2, calcs_amount, &result );
	max_time = result.exp_time;
	min_time = result.exp_time;
	ave_time += result.exp_time;
	//printf( "\n---- %d ----\nresult = %g\nexp_time = %ld\n", 1, result.integr_val, result.exp_time ); 
	

	for( int i = 2; i <= starts_num; i++ )
	{
		startCalculations( x1, x2, calcs_amount, &result );

		if( result.exp_time > max_time)
			max_time = result.exp_time;
		if( result.exp_time < min_time)
			min_time = result.exp_time;
		ave_time += result.exp_time;
		printf( "\n---- %d ----\nresult = %g\nexp_time = %ld\n", i, result.integr_val, result.exp_time ); 
	}

	ave_time /= starts_num;

	printf("\n\n---- TIME INFO ----\nmin_time = %ld \nmax_time = %ld \nave_time = %ld \n", min_time, max_time, ave_time );
	return 0;
}


int startCalculations( double x1, double x2, int calcs_amount, struct exp_result * result )
{
	clock_t s_time, f_time;
	s_time = clock();

	result -> integr_val = calculate( x1, x2, 1000 );

	f_time = clock();
	printf("time: %ld\n", f_time-s_time);
	result -> exp_time = f_time - s_time;
	struct exp_result tmp;
	tmp = *result;
	tmp.integr_val -=1;
	return 0;
};

double calculate( double x1, double x2, int seg_amount )
{
	double integral = 0;
	double delta;
	delta = (x2 - x1)/seg_amount;
	for( int i = 0; i < seg_amount; i++ )
	{
		integral += delta/6 * ( f(x1+i*delta) + 4*f(x1 + i*delta + delta/2) + f(x1+ (i+1)*delta ));	
	}
	return integral;
};

double f( double x )
{
	double f;
	f = x*x*x;
	//f = pow( 2, x) * sin( pow( 2, -x) );
	return f;
};

