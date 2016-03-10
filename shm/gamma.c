#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#define BUF_SIZE 4098

#define FILE_FOR_KEY "a.out"
struct msg
{
  int amount; //the amount of bytes in buf 
  char buf[BUF_SIZE];
};

#define SHM_SIZE sizeof(struct msg)

struct sembuf sem_ops[3];

union semun 
{
  int val;    /* Value for SETVAL */
  struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
  unsigned short  *array;  /* Array for GETALL, SETALL */
  struct seminfo  *__buf;  /* Buffer for IPC_INFO (Linux-specific) */
};

union semun sem_un;

/*-----about semaphores-----
0: sender (if exist 1)
1: reciever (if exist 1)
2: indicates if shared memory is empty
3: indicates if shared memory is full
*/

int main( int argc, char *argv[] )
{

  //create a key
  key_t key;
  key = ftok( FILE_FOR_KEY, 0 );
  if( key == -1 )
  {
    printf( "%s: can't create a key\n", argv[0] );
    exit(-1);
  }

  // create shared memory
  int shm_id;
  shm_id = shmget( key , SHM_SIZE, IPC_CREAT | IPC_EXCL | 0777 );

  if( shm_id == -1 )
  {
    if( errno != EEXIST )
    {
      printf( "%s: can't create shared memory\n", argv[0] );
      exit(-1);
    }
    else
    {
      shm_id = shmget( key , SHM_SIZE, 0 );
      if( shm_id == -1 )
      {
        printf( "%s; can't open shared memory\n", argv[0] );
        exit(-1);
      }
    }
  }

  //attach shared memory
  struct msg * shm_adr;
  shm_adr = (struct msg *) shmat( shm_id, NULL, 0 );
  if( shm_adr == (void *) -1 )
  {
    printf( "%s; can't attach  shared memory\n", argv[0] );
    exit(-1);
  }
  
  //create semaphores
  int sem_id;
  sem_id = semget( key , 4, IPC_CREAT | IPC_EXCL | 0777 );

  if( sem_id == -1 )
  {
    if( errno != EEXIST )
    {
      printf( "%s: can't create semaphores\n", argv[0] );
      exit(-1);
    }
    else
    {
      sem_id = semget( key , 4, 0 );
      if( sem_id == -1 )
      {
        printf( "%s; can't open semaphores\n", argv[0] );
        exit(-1);
      }
    }
  }

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
    int read_amount, ret_val;
   
    //if sender is already exist
    //snd 0
    sem_ops[0].sem_num = 0;
    sem_ops[0].sem_op = 0;
    sem_ops[0].sem_flg = IPC_NOWAIT;
    //snd +1
    sem_ops[1].sem_num = 0;
    sem_ops[1].sem_op = 1;
    sem_ops[1].sem_flg = SEM_UNDO;
    //operations
    ret_val = semop( sem_id, &sem_ops[0], 2 );
    if( ( ret_val != 0 ) && ( errno == EAGAIN ) )
    {
      printf("%s: sender is already exist\n", argv[0] );
      exit(-1);
    }
    //cr section 1 ->
    
    //wait for reciever
    //rcv -1
    sem_ops[0].sem_num = 1;
    sem_ops[0].sem_op = -1;
    sem_ops[0].sem_flg = 0;
    //rcv +1
    sem_ops[1].sem_num = 1;
    sem_ops[1].sem_op = 1;
    sem_ops[1].sem_flg = 0;
    //operations
    ret_val = semop( sem_id, &sem_ops[0], 2 );
 
    do
    {
      //rcv -1
      sem_ops[0].sem_num = 1;
      sem_ops[0].sem_op = -1;
      sem_ops[0].sem_flg = IPC_NOWAIT;
      //rcv +1
      sem_ops[1].sem_num = 1;
      sem_ops[1].sem_op = 1;
      sem_ops[1].sem_flg = SEM_UNDO;
      //empty -1
      sem_ops[2].sem_num = 2;
      sem_ops[2].sem_op = -1;
      sem_ops[2].sem_flg = 0;
      //operations
      ret_val = semop( sem_id, &sem_ops[0], 3);
      if( ( ret_val != 0) && ( errno == EAGAIN ) )
      {
        printf("%s: reciever crashed\n", argv[0] );
        exit(-1);
      }
      //cr section 2 ->
      read_amount = read( fd, shm_adr -> buf, BUF_SIZE );
      shm_adr -> amount = read_amount;
      // <- 2

      // <- 1

      //full +1
      sem_ops[0].sem_num = 3;
      sem_ops[0].sem_op = 1;
      sem_ops[0].sem_flg = SEM_UNDO;
      //operations
      ret_val = semop( sem_id, &sem_ops[0], 1);

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
    int write_amount, ret_val;
    //if reciever is already exist
    //rcv 0
    sem_ops[0].sem_num = 1;
    sem_ops[0].sem_op = 0;
    sem_ops[0].sem_flg = IPC_NOWAIT;
    //rcv +1
    sem_ops[1].sem_num = 1;
    sem_ops[1].sem_op = 1;
    sem_ops[1].sem_flg = SEM_UNDO;
    //operations
    ret_val = semop( sem_id, &sem_ops[0], 2);
    if( ( ret_val != 0) && ( errno == EAGAIN ) )
    {
      printf("%s: reciever is already exist\n", argv[0] );
      exit(-1);
    }
    //cr section 1 ->

    //wait for sender
    //snd -1
    sem_ops[0].sem_num = 0;
    sem_ops[0].sem_op = -1;
    sem_ops[0].sem_flg = 0;
    //snd +1
    sem_ops[1].sem_num = 0;
    sem_ops[1].sem_op = 1;
    sem_ops[1].sem_flg = 0;
    //operations
    ret_val = semop( sem_id, &sem_ops[0], 2);

    //inintialize empty = 1
    sem_ops[0].sem_num = 2;
    sem_ops[0].sem_op = 1;
    sem_ops[0].sem_flg = SEM_UNDO;
    //operations
    ret_val = semop( sem_id, &sem_ops[0], 1);
    
    do
    {
      //snd -1
      sem_ops[0].sem_num = 0;
      sem_ops[0].sem_op = -1;
      sem_ops[0].sem_flg = IPC_NOWAIT;
      //snd +1
      sem_ops[1].sem_num = 0;
      sem_ops[1].sem_op = 1;
      sem_ops[1].sem_flg = SEM_UNDO;
      //full -1
      sem_ops[2].sem_num = 3;
      sem_ops[2].sem_op = -1;
      sem_ops[2].sem_flg = 0;
      //operations
      ret_val = semop( sem_id, &sem_ops[0], 3);
      if( ( ret_val != 0) && ( errno == EAGAIN ) )
      {
        printf("%s: sender crashed\n", argv[0] );
        exit(-1);
      }

      write_amount = write( 1, shm_adr -> buf, shm_adr -> amount);

      //empty +1
      sem_ops[0].sem_num = 2;
      sem_ops[0].sem_op = 1;
      sem_ops[0].sem_flg = SEM_UNDO;
      //operations
      ret_val = semop( sem_id, &sem_ops[0], 1);
      printf("\n\nwrite_amount = %d\n", write_amount);
    } while ( write_amount == BUF_SIZE );
    printf("\n\nexit from while\n");
    //ending
    shmdt( shm_adr );
    printf("\n\nready to rm\n\n");
    shmctl( shm_id, IPC_RMID, NULL );
    semctl( sem_id, 0, IPC_RMID, NULL );
    exit(0);
  }
}

