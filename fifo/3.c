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
    if(mkfifo(FIFONAME, 0777) == -1)
    {
      //can't create fifo
      fifofd=open(FIFONAME, O_WRONLY);
      if(fifofd == -1)
      {
        printf("Error: can't create fifo\n");
        return -1;
      }
      close (fifofd);
      remove(FIFONAME);
      mkfifo(FIFONAME, 0777);
    }
    fifofd=open(FIFONAME, O_WRONLY);
    if(fifofd == -1)
    {
      //error open
      printf("Error: can't open fifo\n");
    }

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
  else if(argc == 1)
  {
    //reciever
    int fifofd;
    do
    {
      fifofd=open(FIFONAME, O_RDONLY);
    } while(fifofd == -1);
    char buf[BUFSIZE];
    int i, read_amount;
    read_amount=read(fifofd, &buf, BUFSIZE); while(read_amount>0)
    {
      write(1, &buf, read_amount);
      read_amount = read(fifofd, &buf, BUFSIZE);
    }
    close(fifofd);
    remove(FIFONAME);
    return 0;
  }
  return 0;
}

