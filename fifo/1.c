#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
  if(argc == 2)
  {
    //sender
    int pipefd[2];
    int const bufsize = 4096;
    pid_t pid;
    if(pipe(pipefd) == -1)
    {
      //error pipe
      printf("Error: pipe\n");
      return -1;
    }
    pid = fork();
    if(pid > 0)
    {
      //parent
      close(pipefd[1]); 
      char buf[bufsize];
      int i, read_amount;
      read_amount=read(pipefd[0], &buf ,bufsize);
      while(read_amount>0)
      { 
        write(1, &buf, read_amount);
        read_amount = read(pipefd[0], &buf, bufsize);
      }
      wait(NULL);
      close(pipefd[0]);
      return 0;
    }
    else if(pid == 0)
    {
      //child
      close(pipefd[0]);
      int filefd, read_amount;
      char buf[bufsize];
      filefd=open(argv[1], O_RDONLY);
      if(filefd == -1)
      {
        //error open
        printf("Error: can't open file\n");
      }
      read_amount=read(filefd, &buf, bufsize);
      while(read_amount>0)
      {
        write(pipefd[1], &buf, read_amount);
        read_amount=read(filefd, &buf, bufsize);
      }
     // close(pipefd[1]);
      return 0;
    }
    else
    {
      //error fork
      printf("Error: can't create new process (fork)\n");
      return -1;
    }
  }
  else if(argc == 1)
  {
    //reciever
    printf("Error: no argument\n");
    return -1;
  }
  return 0;
}
