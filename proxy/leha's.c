#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <sys/types.h>
#include <sys/select.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/signal.h>
#include <sys/prctl.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#define PIPE_SIZE 1024*64

/* *************************************************************************************
 * BUFFER DEFINITION BEGIN
 * proxy buffer to hold infomation
 * from prev. client and transmite
 * to the next
 */

struct buffer_t
{
    char* buf;
    int   buf_size;
    int   count;
};


void bfSetConfig( struct buffer_t* buffer , char* buf , int buf_size )
{
    if( (!buffer) || (!buf) )
    {
        fprintf( stderr , "nullptr as argument to bfSetConfig" );
        exit( EXIT_FAILURE );
    }

    buffer->buf      = buf;
    buffer->buf_size = buf_size;
    buffer->count    = 0;
}


struct buffer_t* getBuffers( long int n_cld )
{
    if( n_cld <= 0 )
        assert(0);

    struct buffer_t* bufs = ( struct buffer_t* ) malloc( sizeof( struct buffer_t ) * n_cld );
    if( bufs == 0 )
    {
        perror( "malloc" );
        exit( EXIT_FAILURE );
    }

    long int i = 0 ;
    int buf_size = 0;
    for( ; i < n_cld ; ++i )
    {
        buf_size       = 4 * pow( 3 , n_cld - i );
        bufs[i].buf    = (char*) malloc( sizeof(char) * buf_size );
        if( bufs[i].buf == 0 )
        {
            perror( "malloc" );
            exit( EXIT_FAILURE );
        }
        bufs[i].buf_size   = buf_size;
        bufs[i].count = 0;
    }
    return bufs;
}



void freeBuffers( struct buffer_t* bufs , long int n_cld )
{
    if( !bufs )
    {
        fprintf( stderr , "nullptr as argument to freeBuffers" );
        exit( EXIT_FAILURE );
    }

    if( n_cld <= 0 )
        assert(0);

    long int i = 0;

    for( ; i < n_cld ; ++i )
        free( bufs[i].buf );
    free( bufs );
}
/**************************************************************************************
 * BUFFER DEFINITION END
 */


/**************************************************************************************
 *  CLIENT DEFINITION BEGIN
 */

/*
 * every client is a child proccess main aim is to
 * transmite file from one fd to another[parent buffer]
 * If it's a first client , he will read from the given file
 * in other case , reads from parent buffer
 */

struct client_t
{
    int parent_readfd;
    int parent_writefd;
    int read_fd;
    int write_fd;

    char is_first;
    char is_last;
    char* file_name;

    int nClient;
};


void clStartTransmission( struct client_t* client )
{
    if( !client )
    {
        fprintf( stderr , "nullptr as argument to clStartTransmission" );
        exit( EXIT_FAILURE );
    }

    int fd_read = 0;
    if( client->is_first == 1 )
    {
        if( !client->file_name )
            assert(!"nullptr");
        if( ( fd_read = open( client->file_name , O_RDONLY )) == -1 )
        {
            perror( "client open" );
            exit( EXIT_FAILURE );
        }
    }
    else
        fd_read = client->read_fd;

    char buf[PIPE_SIZE];
    int res = 0;
    for(;;)
    {
        res = read(fd_read , buf , PIPE_SIZE );
        /* DEBUG */
        if( client->nClient == 2 )
            sleep(1);

        if( res == 0 )
            break;
        if( res == -1 )
        {
            perror( "client read");
            exit( EXIT_FAILURE );
        }

        res = write( client->write_fd , buf , res );
        if( (res == -1) && ( errno == EPIPE ) )
            break;
        else if( res == -1 )
        {
            perror( "client write" );
            exit( EXIT_FAILURE );
        }
    }
    close( fd_read );
    close( client->write_fd);
    exit( EXIT_SUCCESS );
}

/*
 *   CLIENT DEFINITION OVER
 */




/* MAIN PROGRAM */

/* SERVER FUNCTIONS
 * to handle an able client fd
 * by select
*/
void handleReadableFd( struct client_t* client ,
                       fd_set* readfds , fd_set* readfds_base ,
                       fd_set* writefds_base , struct buffer_t* buf );

void handleWriteableFd( struct client_t *client ,
                        fd_set* writefds , fd_set* writefds_base ,
                        fd_set* readfds_base , struct buffer_t* buf );
/* * */
/* parse cmd argument */
long int getNumber( char* str );
/* * */
int main( int argc , char* argv[] )
{
    if( argc != 3 )
        exit( EXIT_FAILURE );

    int i = 0;
    int k = 0;
    const long int n_cld = getNumber( argv[1] );
    struct client_t clients[n_cld];
    int pipe_fds[2]; /* tmp handler for pipes */
    int max_fd = 0;

    /* configures clients ,
     * connects fds between client & server */
    for( i = 0 ; i < n_cld ; ++i )
    {
        clients[i].nClient = i;

        if( i == n_cld-1)
            clients[i].parent_writefd = -1;

        if( i != 0 )
        {
            clients[i].file_name = 0;
            clients[i].is_first  = 0;
            if( i == n_cld-1 )
                clients[i].is_last = 1;
            else
                clients[i].is_last = 0;

            if( pipe( pipe_fds ) == -1 )
            {
                perror("pipe");
                exit( EXIT_FAILURE );
            }

            if( pipe_fds[0] > max_fd )
                max_fd = pipe_fds[0];

            if( pipe_fds[1] > max_fd )
                max_fd = pipe_fds[1];

            clients[i].read_fd          = pipe_fds[0];
            clients[i-1].parent_writefd = pipe_fds[1];

            fcntl(clients[i-1].parent_writefd, F_SETFL, O_NONBLOCK);
        }
        else
        {
            clients[i].file_name = argv[2];
            clients[i].is_first  = 1;
            /* it can be the only client */
            if( i == n_cld-1 )
                clients[i].is_last = 1;
            else
                clients[i].is_last = 0;

            clients[i].read_fd   = -1;
            clients[i].parent_writefd = -1;
        }

        if( pipe(pipe_fds)  == -1 )
        {
            perror("pipe");
            exit( EXIT_FAILURE );
        }
        if( pipe_fds[0] > max_fd )
            max_fd = pipe_fds[0];

        if( pipe_fds[1] > max_fd )
            max_fd = pipe_fds[1];

        clients[i].parent_readfd = pipe_fds[0];
        clients[i].write_fd      = pipe_fds[1];

        fcntl(clients[i].parent_readfd, F_SETFL, O_NONBLOCK);
    }
    /* * */

    /*
     * initial forking
     */
    int pid = 0;
    for( i = 0 ; i < n_cld ; ++i )
    {
        if( (pid = fork() ) == -1 )
            exit(EXIT_FAILURE);

        if( pid == 0 )
        {
            /*
             * close useless pipe fd
             * for client[i]
             */                
            for( k = 0 ; k < i ; ++k )
            {
                close( clients[k].parent_readfd  );
                close( clients[k].parent_writefd );
                close( clients[k].write_fd );
                if( k != 0 )
                    close( clients[k].read_fd );
            }
            close( clients[i].parent_readfd );
            if( i != n_cld-1 )
                close( clients[i].parent_writefd );
            /* * */
            /* initial client work */

            clStartTransmission( &clients[i] );
        }
    }

    /* PARENT */

    /*
     * close useless pipes fd
     * for server
     */
    for( i = 0 ; i < n_cld ; ++i )
    {
        if( i != 0 )
            close( clients[i].read_fd  );

        close( clients[i].write_fd );
    }
    /* * */

    /* initial server work */
    fd_set readfds;
    fd_set writefds;
    fd_set readfds_base;
    fd_set writefds_base;

    FD_ZERO( &readfds_base );
    FD_ZERO( &writefds_base );

    for( i = 0 ; i < n_cld ; ++i )
        FD_SET( clients[i].parent_readfd  , &readfds_base  );

    struct buffer_t* bufs = getBuffers( n_cld );
    for(;;)
    {
        readfds  = readfds_base;
        writefds = writefds_base;
        if( select( max_fd+1 , &readfds , &writefds , NULL , NULL ) == -1 )
            exit( EXIT_FAILURE );


        for( i = 0 ; i < n_cld ; ++i )
        {
            handleReadableFd( &clients[i] , &readfds ,
                              &readfds_base , &writefds_base , &bufs[i] );

            handleWriteableFd( &clients[i] , &writefds ,
                               &writefds_base , &readfds_base , &bufs[i] );

        }
    }
}


long int getNumber( char* str )
{
    if( !str )
    {
        fprintf( stderr , "nullptr as argument to getNumber" );
        exit( EXIT_FAILURE );
    }

    char* end;
    long int val = strtol( str , &end , 10 );

    if( (*end != '\0') || (end == str) || (val <= 0) )
    {
        fprintf( stderr , "bad first cmd argument");
        exit(EXIT_FAILURE);
    }

    return val;
}



void handleReadableFd( struct client_t* client ,
                       fd_set* readfds , fd_set* readfds_base ,
                       fd_set* writefds_base , struct buffer_t* buf )
{
    if( (!client) || (!readfds_base) || (!writefds_base) || (!buf) )
    {
        fprintf( stderr , "nullptr as argument to handleReadableFd" );
        exit( EXIT_FAILURE );
    }

    int res = 0;
    if( FD_ISSET(client->parent_readfd , readfds) )
    {
        if( client->is_last )
        {
            res = read( client->parent_readfd , buf->buf , buf->buf_size );

            if( res == 0 )
                exit( EXIT_SUCCESS );
            if( res == -1 )
            {
                perror(" server read" );
                exit( EXIT_FAILURE );
            }

            write( STDOUT_FILENO , buf->buf , res );
            return;
         }

         /* there are unwritten data */
         if( buf->count != 0 )
         {
            res = read( client->parent_readfd ,
                        buf->buf + buf->count ,
                        buf->buf_size - buf->count );
            /* buffer is full */
            if( res == 0 )
            {
                FD_CLR( client->parent_readfd , readfds_base );
                return;
            }
            if ( res == -1 )
            {
                perror(" server read ");
                exit( EXIT_FAILURE );
            }
          }
          else
          {
             if( buf->buf_size >= PIPE_SIZE )
                 res = read( client->parent_readfd , buf->buf , PIPE_SIZE );
             else
                 res = read( client->parent_readfd , buf->buf , buf->buf_size );

             if( res == 0 )
             {
                 close( client->parent_readfd  );
                 close( client->parent_writefd );
                 FD_CLR( client->parent_readfd , readfds_base );
                 FD_CLR( client->parent_writefd , writefds_base );

                 return;
             }
             if( res == -1 )
             {
                 perror(" server read ");
                 exit( EXIT_FAILURE );
             }
          }

          buf->count += res;
          FD_SET( client->parent_writefd , writefds_base );
     }
}



void handleWriteableFd( struct client_t *client ,
                        fd_set* writefds , fd_set* writefds_base ,
                        fd_set* readfds_base , struct buffer_t* buf )
{

    if( (!client) || (!readfds_base) || (!writefds) || (!buf) || (!writefds_base) )
    {
        fprintf( stderr ,"nullptr as arg for handleWritableFd" );
        exit( EXIT_FAILURE );
    }

    int res = 0;
    if( FD_ISSET( client->parent_writefd , writefds ) )
    {
        if( buf->count > PIPE_SIZE )
            res = write( client->parent_writefd , buf->buf , PIPE_SIZE );
        else
           res = write( client->parent_writefd , buf->buf , buf->count );

        if( (res == -1) && (errno == EAGAIN) )
        {
            close(  client->parent_readfd  );
            close(  client->parent_writefd );
            FD_CLR( client->parent_readfd , readfds_base );
            FD_CLR( client->parent_readfd , writefds_base );
            return;
         }
         if( res == -1 )
         {
             perror( "server write " );
             exit( EXIT_FAILURE );
         }

         buf->count -= res;
         int k = 0;
         if( buf->count != 0 )
         {
            char* offset_buf = buf->buf+res;
            for( k = 0 ; k < buf->count ; ++k )
                buf->buf[k] = offset_buf[k];
         }
         else
             FD_CLR( client->parent_writefd , writefds_base );

        if( !FD_ISSET( client->parent_readfd  , readfds_base ) )
             FD_SET( client->parent_readfd  , readfds_base );
    }
}