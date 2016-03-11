#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

int main( int argc, char *argv[] )
{
  if( argc == 2)
  {
    //program
    int child_num, msqid, childr_amount;
    childr_amount = atoi( argv[1] );
    msqid = msgget( IPC_PRIVATE, 0666 | IPC_CREAT );
    struct my_msg
    {
      long mtype;
      int num;
    };
    struct my_msg msg;
    //create cildren
    pid_t pid;
    for( child_num = 1; child_num <= childr_amount; child_num++ )
    {
      pid = fork();
      if( pid == 0 ) break;
    }
    if( pid > 0 ) 
    {
      //parent
      msg.mtype = childr_amount;
      msg.num = childr_amount+1;
      msgsnd( msqid, &msg, sizeof(msg)-sizeof(long), IPC_NOWAIT );
      printf("p: msg.mtype = %d \n", childr_amount+1);
      fflush(stdout);
      msgrcv( msqid, &msg, sizeof(msg)-sizeof(long), 1, 0 );
      msgctl( msqid, IPC_RMID, NULL );
      printf("\n");
    } 
    else
    {
      //child
      msgrcv( msqid, &msg, sizeof(msg)-sizeof(long), child_num+1, 0 );
      printf("%d:  ", child_num);
      fflush(stdout);
      msg.mtype--;
      printf("msg.mtype = %ld \n", msg.mtype);
      fflush(stdout);
      msgsnd( msqid, &msg, sizeof(msg)-sizeof(long), IPC_NOWAIT );
    }
  }
  else 
  {
    printf("%s: wrong arguments\n", argv[0]);
    exit(-1);
  }
  exit(0);
}
