#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <signal.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>


int write_char = 0, digit = 128;
pid_t pid;

//signal's dispositions

void one(int signo)
{
  write_char = write_char + digit; // <- cr section from here
  digit = digit / 2;
  kill(pid, SIGUSR1); // <- cr section to here
}

void zero(int signo)
{
  digit = digit / 2; // <- cr section from here
  kill(pid, SIGUSR1); // <- cr section to here
}

void child_died(int signo)
{
  exit(0);
}

void parent_died(int signo)
{
  exit(0);
}

void empty(int signo)
{

}

int main(int argc, char *argv[])
{
  if(argc != 2)
  {
    printf("%s: incorrect arguments\n", argv[0]);
    exit(0);
  }
  pid_t ppid;
	ppid = getpid();
    
  //parent's dispositions

  //SIGUSR1 -- one()
  struct sigaction act_one;
  memset(&act_one, 0, sizeof(act_one));
  act_one.sa_handler = one;
  sigfillset(&act_one.sa_mask);
  sigaction(SIGUSR1, &act_one, NULL);     
  
  //SIGUSR2 -- zero()
  struct sigaction act_zero;
  memset(&act_zero, 0, sizeof(act_zero));
  act_zero.sa_handler = zero;
  sigfillset(&act_zero.sa_mask);
  sigaction(SIGUSR2, &act_zero, NULL);
 
  //SIGCHLD -- child_died()
  struct sigaction act_child_died;
  memset(&act_child_died, 0, sizeof(act_child_died));
  act_child_died.sa_handler = child_died;
  sigfillset(&act_child_died.sa_mask);
  sigaction(SIGCHLD, &act_child_died, NULL);

  //set mask for both parent and child
  //SIGUSR1, SIGUSR2, SIGCHLD
  sigset_t set;
  sigemptyset(&set);
  //sigaddset(&set, SIGCHLD);    
  sigaddset(&set, SIGUSR1);
  sigaddset(&set, SIGUSR2);
  sigprocmask(SIG_BLOCK, &set, NULL);
  // <- cr section from here to the end
  sigemptyset(&set);  


  //set *set to full\SIGUSR1, SIGUSR2, SIGCHLD, SIGALRM
  sigfillset(&set);
  sigdelset(&set, SIGUSR1);
  sigdelset(&set, SIGUSR2);
  sigdelset(&set, SIGCHLD);
  sigdelset(&set, SIGALRM);
  //born child
  pid = fork();
   
  //----------------------------------------------------------------
  //child (sender)
  if(pid == 0)
  {
  
    //child's dispositions

    //SIGUSR1 -- empty()
    struct sigaction act_empty;
    memset(&act_empty, 0, sizeof(act_empty));
    act_empty.sa_handler = empty;
    sigfillset(&act_empty.sa_mask);
    sigaction(SIGUSR1, &act_empty, NULL);

    //SIGALRM -- parent_died()
    struct sigaction act_parent_died;
    memset(&act_parent_died, 0, sizeof(act_parent_died));
    act_parent_died.sa_handler = parent_died;
    sigfillset(&act_parent_died.sa_mask);
    sigaction(SIGALRM, &act_parent_died, NULL);
 
    //sigemptyset(&set);
    //open file
    int fd;
    fd = open(argv[1], O_RDONLY);
    if(fd < 0)
    {
      printf("%s: can't open %s\n", argv[0], argv[1]);
    }

    //reading and sending
    char read_char;
    int i;
    while(read(fd, &read_char, 1) > 0)
    {
      for(i = 128; i > 0; i = i / 2)
    	{ // <- cr section from here

    		//bit is 1
     		if(read_char & i)
     		{
          kill(ppid, SIGUSR1);
     		}
     		//bit is 0
     		else
     		{
    			kill(ppid, SIGUSR2);
     		}
     		//wait for parent's signal
     		alarm(1);   
        // <- cr section to here
        sigsuspend(&set);
      } 
    }
      
    //ending
    close(fd);
    exit(0);      
  }
    
  //----------------------------------------------------------------
  //parent (reciever)
  else
  {
    
    //recieving and writing
    while(1)
    {  // <- cr section from here
    	if(digit == 0)
     	{
    		write(1, &write_char, 1);
     		fflush(stdout);
     		digit = 128;
     		write_char = 0;
     	}
     	//wait for child's signal
      // <- cr section to here
      sigsuspend(&set);
    }

  }
  exit(0);
}