#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/shm.h>
#include <sys/sem.h>

#define BUF_SIZE 4098

struct msg
{
  int amount; //the amount of bytes in buf 
  char buf[BUF_SIZE];
};

#define SHM_SIZE sizeof(struct msg);

struct sembuf sem_ops[2];
struct sembuf sem_op; 

/*-----about semaphores-----
0: mutex
1: indicates if shared memory is empty
2: indicates if shared memory is full
*/

int main( int argc, char *argv[] )
{

  //create a key
  key_t key;
  key = ftok( "a.out", 0 );
  if( key == -1 )
  {
    printf( "%s: can't create a key\n", argv[0] );
    exit(-1);
  }

  
  //create semaphores
  int sem_id;
  sem_id = semget( key , 3, IPC_CREAT | IPC_EXCL | 0777 );

  if( sem_id == -1 )
  {
    if( errno != EEXIST )
    {
      printf( "%s: can't create semaphores\n", argv[0] );
      exit(-1);
    }
    else
    {
      sem_id = semget( key , 3, 0 );
      if( sem_id == -1 )
      {
        printf( "%s; can't open semaphores\n", argv[0] );
        exit(-1);
      }
    }
  }
  
  union semun 
  {
    int val;    /* Value for SETVAL */
    struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
    unsigned short  *array;  /* Array for GETALL, SETALL */
    struct seminfo  *__buf;  /* Buffer for IPC_INFO (Linux-specific) */
  };


  union semun sem_un;

  semctl( sem_id, 0, IPC_STAT, sem_un);
  printf("otime %d\n", (int)sem_un.buf->sem_otime);

    //initialization semaphores
  /*

  //--------------------------------------------------------------
  if( argc == 2 )
  {
    //sender
    
    //open file
    int fd;
    fd = open( argv[1], O_RDONLY );
    if( fd == -1 )
    {
      printf( "%s: can't open %s\n", argv[0], argv[1] );
      exit(-1);
    }
    
    //copy from file fo shared memory
    int read_amount;
    do
    {
      //empty --
      //mutex --
      read_amount = read( fd, shm_adr -> buf, BUF_SIZE );
      shm_adr -> amount = read_amount;
      //mutex ++
      //full ++
      //ending
    } while ( read_amount == BUF_SIZE );
    
    //ending
    close( fd );
    shmdt( shm_adr );
    //printf( "%s: end\n", argv[0] );
    exit(0);
  }
  //------------------------------------------------------------
  if( argc == 1 )
  {
    //reciever

    //copy from shared memory to stdout
    int write_amount;
    do
    {
      //full --
      //mutex --
      write_amount = write( 1, shm_adr -> buf, shm_adr -> amount);
      //mutex ++
      //empty ++
    } while ( write_amount == BUF_SIZE );
    
    //ending
    shmdt( shm_adr );
    shmctl( shm_id, IPC_RMID, NULL );
    exit(0);
  }*/
    exit(0);
}

