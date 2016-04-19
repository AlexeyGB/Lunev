#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

#define INTEGRATE_STEP (double)0.00001


long int getNumber( char* str )
{

    char* end;
    long int val = strtol( str , &end , 10 );

    if( (*end != '\0') || (end == str) )
    {
        fprintf( stderr , "bad argument");
        exit(EXIT_FAILURE);
    }

    return val;
}



typedef struct ThreadPack
{
    double begin;
    double end;
    double result;

} ThreadPack;



double integrand( double arg )
{
    return arg*arg*arg;
}



void* calculation( void* arg )
{
    ThreadPack* t_pack = arg;

    double t_res = 0;
    double x = t_pack->begin;

    for( ; x < t_pack->end ; x += INTEGRATE_STEP )
    {
        double arg = x + (INTEGRATE_STEP/2);
        t_res += integrand( arg )*INTEGRATE_STEP;
    }
}



int main(int argc, char *argv[])
{
    if( argc != 4 )
        exit(EXIT_FAILURE);

    double result = 0;

    long int n_threads = getNumber(argv[1]);
    if( n_threads < 0 )
        perror("EXIT_FAILURE");

    pthread_t threads[n_threads];

    double begin     = getNumber(argv[2]);
    double end       = getNumber(argv[3]);

    const double step = ( end - begin )/n_threads;
    ThreadPack t_pack[n_threads];
    int i = 0;
    int r_code = 0;

    for( ; i < n_threads ; ++i )
    {
        t_pack[i].begin     = begin;
        t_pack[i].end       = begin + step;
        t_pack[i].result    = 0;

        begin += step;
        r_code = pthread_create(&threads[i], NULL, calculation , &t_pack[i] );
        if( r_code == -1 )
        {
            perror( "pthread_create:" );
            exit(EXIT_FAILURE);
        }
    }

    for(i = 0 ; i < n_threads ; ++i )
    {
        pthread_join( threads[i] , NULL );
        result += t_pack[i].result;
    }

    printf("result:%f\n" , result );

    return 0;
}
