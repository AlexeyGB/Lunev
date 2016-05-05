#include <cstdio>
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <queue>
#include <string>
#include <atomic>

#define FROM    -1000
#define TO      10000

#define DELTA 0.000003  

#define f(x) x

struct taskData
{
    double from;
    double to;
    double delta;
    double result;
};

double integr_simp( double from, double to )
{
    double integral;
    integral = (to-from)/6 * ( f(from) + 4 * f( (from+to)/2 ) + f(to) );
    return integral;
};

void integralCounter( taskData *arg )
{
    double to = arg->to;
    double from = arg->from;
    double delta = arg->delta;

    double result = 0;

    for( ; from < to; from += delta )
        //result += integr_simp( from, to );
        result +=(to-from)/6 * ( f(from) + 4 * f( (from+to)/2 ) + f(to) );
    arg->result = result;
};

int main( int argc, char *argv[] )
{
	if( argc != 2 )
    {
        std::cout << "Incorrect arguments!\nMust be one argument, the amount of threads.\n";
        exit( EXIT_FAILURE );
    }
    
    int slaves_amount = atoi( argv[1] );
    double from = FROM;
    double to = TO;
    double delta = DELTA;

    double part_length = (to - from) / slaves_amount;

	taskData *tasks = (struct taskData *) malloc( slaves_amount * sizeof(struct taskData) );
    for( int i = 0; i < slaves_amount; i++ )
    {
        tasks[i].from = from + i*part_length;
        tasks[i].to = from + (i+1)*part_length;
        tasks[i].delta = delta;
        tasks[i].result = 0;      
    };

    std::vector<std::thread> slaves;

    for( int i = 0; i < slaves_amount; i++ )
    	slaves.push_back(std::thread(integralCounter, &tasks[i]));

    double result = 0;
	
	for( auto &thread: slaves )
        thread.join();

    for( int i = 0; i < slaves_amount; i++ )
    	result += tasks[i].result;

};
