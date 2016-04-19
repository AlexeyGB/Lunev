#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#define BUFSIZE 4096

#define FIFO_PATH "chanel.fifo"
#define CHECK_TRANSMITTER_FIFO "check_transmitter.fifo"
#define CHECK_RECIEVER_FIFO "check_reciever.fifo"

int copy_file(int input, int output);

int main( int argc, char *argv[] )
{

  // transmitter
  if( argc == 2 )
  {
    // check if any transmitter has been already existed
    int return_val;
    return_val = mkfifo( CHECK_TRANSMITTER_FIFO, 0644 );
    if( (return_val != 0) && (errno != EEXIST) )
    {
      printf( "%s: Can't create FIFO\n", argv[0] );
      exit(-1);
    }

    int check_rd_d;
    check_rd_d = open( CHECK_TRANSMITTER_FIFO, O_RDONLY | O_NONBLOCK );
    if( check_rd_d == -1 )
    {
      printf( "%s: Can't open FIFO for reading\n", argv[0] );
      exit(-1);
    }

    char buf[8];
    do
    {
      return_val = read(check_rd_d, &buf, 4);
    } while( return_val == -1 );

    int check_wr_d;
    check_wr_d = open( CHECK_TRANSMITTER_FIFO, O_WRONLY );
    if( check_wr_d == -1 )
    {
      printf( "%s: Can't open FIFO for writing\n", argv[0] );
      exit(-1);
    }

    // check if any FIFO_PATH has been already existed
    do
    {
      return_val = mkfifo( FIFO_PATH, 0644 );
      if( (return_val != 0) && (errno != EEXIST) )
      {
        printf( "%s: Can't create FIFO\n", argv[0] );
        exit(-1);
      }
    } while( (return_val != 0) && (errno == EEXIST) );

    // open FIFO_PATH
    int fifod;
    fifod = open( FIFO_PATH, O_WRONLY );
    if( fifod == -1 )
    {
      printf( "%s: Can't open FIFO for writing\n", argv[0] );
      exit(-1);
    }

    // open file
    int filed;
    filed = open( argv[1], O_RDONLY );
    if( filed == -1 )
    {
      printf( "%s: Can't open %s for reading\n", argv[0], argv[1]);
      exit(-1);
    }

    //copying
    copy_file( filed, fifod );

    //end
    close( filed );
    close( check_rd_d );
    remove(CHECK_TRANSMITTER_FIFO);
    close( check_wr_d );
    close( fifod );
    exit(0);
  }
//-------------------------------------
  // reciever
  else
  {
    // check if any reciever has been already existed
    int return_val;
    return_val = mkfifo( CHECK_RECIEVER_FIFO, 0644 );
    if( (return_val != 0) && (errno != EEXIST) )
    {
      printf( "%s: Can't create FIFO\n", argv[0] );
      exit(-1);
    }

    int check_rd_d;
    check_rd_d = open( CHECK_RECIEVER_FIFO, O_RDONLY | O_NONBLOCK );
    if( check_rd_d == -1 )
    {
      printf( "%s: Can't open FIFO for reading\n", argv[0] );
      exit(-1);
    }

    char buf[8];
    do
    {
      return_val = read(check_rd_d, &buf, 4);
    } while( return_val == -1 );

    int check_wr_d;
    check_wr_d = open( CHECK_RECIEVER_FIFO, O_WRONLY );
    if( check_wr_d == -1 )
    {
      printf( "%s: Can't open FIFO for writing\n", argv[0] );
      exit(-1);
    }

    // open FIFO
    int fifod;
    do
    {
      fifod = open( FIFO_PATH, O_RDONLY );
    } while( fifod == -1 );

    //copying
    copy_file( fifod, 1);

    //end
    close( fifod );
    close( check_rd_d );
    remove( FIFO_PATH );
    remove(CHECK_RECIEVER_FIFO);
    close( check_wr_d );
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
