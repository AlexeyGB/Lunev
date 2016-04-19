#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#define BUFSIZE 100

#define LIST "list.fifo"

int copy_file(int input, int output);

int exponent_max_pid();

int main( int argc, char *argv[] )
{

  //too many arguments
  if( argc > 2 )
  {
    printf( "%s: too many arguments\n", argv[0] );
    exit(-1);
  }

  // transmitter
  if( argc == 2 )
  {
    //create path name
    int pid;
    pid = getpid();
    int exp_pid;
    exp_pid = exponent_max_pid();
    char path[ exp_pid + 1 ];
    int return_val;
    return_val = sprintf( &path[0], "%d", pid );
    int i;
    for( i = return_val; i < exp_pid; i++)
    {
      path[i] = '0';
    }
    path[ exp_pid ] = 0;

    //create path
    return_val = mkfifo( path, 0666 );
    if( (return_val != 0) && (errno != EEXIST) )
    {
      printf( "%s: Can't create FIFO\n", argv[0] );
      exit(-1);
    }
   /* if(errno == EEXIST )
    {
      //printf("%s: path already exists\n", argv[0]);
      unlink( path );
      //printf("%s: path unlinked \n", argv[0]);
      return_val = mkfifo( path, 0666 );
      if( (return_val != 0) && (errno != EEXIST) )
      {
        printf( "%s: Can't create FIFO\n", argv[0] );
        unlink( path );
        exit(-1);
      }
    } */
    //create list
    return_val = mkfifo( LIST, 0666 );
    if( (return_val != 0) && (errno != EEXIST) )
    {
      printf( "%s: Can't create FIFO\n", argv[0] );
      unlink( path );
      exit(-1);
    }
    // open file
    int file_d;
    file_d = open( argv[1], O_RDONLY );
    if( file_d == -1 )
    {
      printf( "%s: Can't open %s for reading\n", argv[0], argv[1]);
      unlink( path);
      exit(-1);
    }
    //open list
    int list_d;
    list_d = open( LIST, O_WRONLY );
    if( list_d == -1 )
    {
      printf( "%s: Can't open FIFO for reading & writing\n", argv[0] );
      unlink( path );
      exit(-1);
    }
    //put path name in list
    write( list_d, &path, exp_pid + 1);
    //open path
    int path_d;
    path_d = open( path, O_WRONLY );
    if( path_d == -1 )
    {
      printf( "%s: Can't open FIFO for reading & writing\n", argv[0] );
      unlink( path );
      exit(-1);
    }
    //copy file
    copy_file( file_d, path_d );
    //end
    close( file_d );
    close( list_d );
    close( path_d );
    exit(0);
  }
//-------------------------------------
  // reciever
  else
  {
    //create list
    int return_val;
    return_val = mkfifo( LIST, 0666 );
    if( (return_val != 0) && (errno != EEXIST) )
    {
      printf( "%s: Can't create FIFO\n", argv[0] );
      exit(-1);
    }
    //open list
    int list_d;
    list_d = open( LIST, O_RDONLY );
    if( list_d == -1 )
    {
      printf( "%s: Can't open FIFO for reading & writing\n", argv[0] );
      exit(-1);
    }
    //get path name
    int exp_pid;
    exp_pid = exponent_max_pid();
    char path[ exp_pid + 1 ];
    read( list_d, &path, exp_pid+1 );
    //open path
    int path_d;
    path_d = open( path, O_RDONLY );
    if( path_d == -1 )
    {
      printf( "%s: Can't open FIFO for reading & writing\n", argv[0] );
      exit(-1);
    }
    //copy file
    copy_file( path_d, 1 );
    //exit
    close( path_d );
    close( list_d );
    unlink( path );
    exit(0);

  }

}

int copy_file( int input, int output )
{
  int read_amount;
  char buf[BUFSIZE];
  read_amount = read( input, &buf, BUFSIZE );
  while( read_amount > 0 )
  {
    write( output, &buf, read_amount );
    read_amount = read( input, &buf, BUFSIZE );
  }
  return 0;
}

int exponent_max_pid()
{
  int pid_max;
  int pid_max_fd;
  pid_max_fd = open( "/proc/sys/kernel/pid_max", O_RDONLY );
  FILE *f;
  f=fdopen(pid_max_fd, "r");
  fscanf(f, "%d", &pid_max);
  fclose(f);
  close(pid_max_fd);

  int exp=0;
  while( pid_max!=0 )
  {
    exp++;
    pid_max=pid_max/10;
  }
  return exp;


}
