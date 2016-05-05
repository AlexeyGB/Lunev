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

typedef  void (*Handler) ( taskData *arg );

struct task
{
    task( Handler handler, struct taskData *arg )
    {
        _handler = handler;
        _arg = arg;
    };
    Handler _handler;
    taskData *_arg;
};

class thread_pool
{
private:
    int _slaves_num;
    std::vector<std::thread> slaves;
    std::queue<struct task> _tasks;
    std::mutex tasksLock;
    std::mutex coutLock;
    std::condition_variable cvTaskCheck;
    std::atomic<int> ready;
    std::atomic<int> tasksAmount;
    std::atomic<bool> turn_off;
    std::atomic<int> num_of_done_tasks;
    static void slave( std::mutex &tasksLock, std::mutex &coutLock,
                         std::queue<struct task> &tasks, std::condition_variable &cvTaskCheck, 
                         std::atomic<int> &ready, std::atomic<int> &tasksAmount, 
                         std::atomic<bool> &turn_off, std::atomic<int> &num_of_done_tasks );
public:
    thread_pool( int slaves_num );
    ~thread_pool();
    void post( Handler handler, taskData *arg );
    void join();
};

thread_pool::thread_pool( int slaves_num ) : 
    _slaves_num(slaves_num),
    ready(0), 
    tasksAmount(0),
    turn_off(false),
    num_of_done_tasks(0) 
{
    if( slaves_num > 0) 
    {
        for( int i = 0; i < slaves_num; i++ )
            slaves.push_back( std::thread( slave, std::ref(tasksLock), std::ref(coutLock), 
                                            std::ref(_tasks),std::ref(cvTaskCheck), std::ref(ready), 
                                            std::ref(tasksAmount), std::ref(turn_off), std::ref(num_of_done_tasks) ) );
    }
};  

thread_pool::~thread_pool()
{
    
};

void thread_pool::post( Handler handler, taskData *arg )
{
    tasksLock.lock();
    _tasks.push(task( handler, arg ));
    tasksLock.unlock();
    tasksAmount++;
    coutLock.lock();
    coutLock.unlock();

    cvTaskCheck.notify_one();
};

void thread_pool::slave( std::mutex &tasksLock, std::mutex &coutLock,
                         std::queue<struct task> &tasks, std::condition_variable &cvTaskCheck, 
                         std::atomic<int> &ready, std::atomic<int> &tasksAmount, 
                         std::atomic<bool> &turn_off, std::atomic<int> &num_of_done_tasks )
{   
    Handler handler;
    taskData *arg;

    coutLock.lock();
   // std::cout << "Thread " << std::this_thread::get_id() << " created" << std::endl;
    coutLock.unlock();

        ready++;
        std::unique_lock<std::mutex> locker(tasksLock);
        cvTaskCheck.wait( locker, [&](){ 
            bool task_exist = 0;
            if(tasksAmount != 0 ) task_exist = 1;
            return (turn_off || task_exist); } );

        if( turn_off )
        {
            ready--;
            tasksLock.unlock();
        }
        else
        {
            handler = tasks.front()._handler;
            arg = tasks.front()._arg;
            tasks.pop();
            ready--;
            tasksAmount--;
            tasksLock.unlock();
            handler(arg);
            num_of_done_tasks++;
        }
};

void thread_pool::join()
{
    for( auto &thread: slaves )
        thread.join();
};
//*********************************************************************************************

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
        result += integr_simp( from, to );

    arg->result = result;
};

int main( int argc, char *argv[] )
{ 
    if( argc != 2 )
    {
        printf("Incorrect arguments!\nMust be one argument, the amount of threads.\n");
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

    thread_pool slaves(slaves_amount);

    for( int i = 0; i < slaves_amount; i++ )
        slaves.post( integralCounter, &tasks[i] );

    slaves.join();

    double result;
    for( int i = 0; i < slaves_amount; i++ )
        result += tasks[i].result;

    std::cout << result << "\n";
    return 0;
};