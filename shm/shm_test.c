#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/shm.h>

#define BUFFER_SIZE 4098

int main( int argc, char *argv[] )
{
  int shm_id, ret;

  if( argc == 2 )
  {
    shm_id = shmget( ftok("111", 0) , BUFFER_SIZE, IPC_CREAT | 0777 );
    /*if(shm_id == -1)
    {
      printf("%s: can't create/open shared memory\n", argv[0] );
      exit(-1);
    }*/

    void *shm;
    shm = shmat( shm_id, NULL, 0);
    /*if( shm == -1)
    {
      printf("%s: can't attach shared memory\n", argv[0] );
      exit(-1);
    }*/

    int fd;
    int read_amount;
    fd = open( argv[1], O_RDONLY );
    read_amount = read( fd, shm, BUFFER_SIZE );
    printf("%s: read_amount = %d\n", argv[0], read_amount);
    close(fd);
    shmdt(shm);
    printf("%s: end\n", argv[0]);
    exit(0);
  }
  if( argc == 1 )
  {
    shm_id = shmget( ftok("111", 0) , BUFFER_SIZE, IPC_CREAT | 0777 );
    void *shm;
    shm = shmat( shm_id, NULL, 0);
    int write_amount;
    write_amount = write(1, shm, BUFFER_SIZE);
    printf("\n\n\n%s: write_amount = %d\n", argv[0], write_amount);
    //printf("\n%s: end\n", argv[0]);
    shmdt(shm);
    shmctl(shm_id, IPC_RMID, 0);
    exit(0);
  }
}
