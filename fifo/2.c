#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#define FIFONAME "fifo01"
#define BUFSIZE 4096 

int main(int argc, char *argv[])
{
  if(argc == 2)
  {
    //sender
    int fifofd;
    pid_t pid;
    if(mkfifo(FIFONAME, 0777) == -1)
    {
      //error fifo
      printf("Error: can't create fifo\n");
      return -1;
    }
    pid = fork();
    if(pid > 0)
    {
      //parent
      fifofd=open(FIFONAME, O_RDONLY);
      char buf[BUFSIZE];
      int i, read_amount;
      read_amount=read(fifofd, &buf, BUFSIZE); while(read_amount>0)
      { 
        write(1, &buf, read_amount);
        read_amount = read(fifofd, &buf, BUFSIZE);
      }
      wait(NULL);
      close(fifofd);
      remove(FIFONAME);
      return 0;
    }
    else if(pid == 0)
    {
      //child
      fifofd=open(FIFONAME, O_WRONLY);
      int filefd, read_amount;
      char buf[BUFSIZE];
      filefd=open(argv[1], O_RDONLY);
      if(filefd == -1)
      {
        //error open
        printf("Error: can't open file\n");
      }
      read_amount=read(filefd, &buf, BUFSIZE );
      while(read_amount>0)
      {
        write(fifofd, &buf, read_amount);
        read_amount=read(filefd, &buf, BUFSIZE);
      }
      close(fifofd);
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
