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
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define PIPE_SIZE 66560

struct child_t
{
	int chld_num;
	int first;
	int last;

	int writefd;
	int readfd;
	int prnt_writefd;
	int prnt_readfd;
};

struct buffer_t
{
	char *buf;
	int buf_size;
	int full_sp; //full space
};

/* int select(int nfds, fd_set *readfds, fd_set *writefds,
                  fd_set *exceptfds, struct timeval *timeout); */

int getnum(char *str);

int chldConfig(struct child_t *children, int chld_num);
/* configures struct children[chld_amount],
makes fds between parent and children 
returns max_fd */

void chldStartTransmission(struct child_t *child);

void handleReadableFd(struct child_t *child, fd_set *readfds, 
					  fd_set *all_readfds, fd_set *all_writefds, 
					  struct buffer_t *buf);

void handleWriteableFd(struct child_t *child, fd_set *writefds, 
					   fd_set *all_readfds, fd_set *all_writefds, 
					   struct buffer_t *buf);

struct buffer_t *getBuffers(int chld_amount);

void clearBuffers(struct buffer_t *bufs, int chld_amount);


//---------------------------------------
// MAIN PART

int main(int argc, const char *argv[])
{
	if(argc != 3)
	{
		printf("%s: error arguments\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	int chld_amount;
	chld_amount = getnum(argv[1]);
	
	//configure children's struct
	//create fds
	struct child_t children[chld_amount];
	int max_fd;
	max_fd = chldConfig(&children, chld_amount);


    //start forking
	int i;
	int k;
	pid_t pid;
	for(i = 0; i < chld_amount; i++)
	{
		pid = fork();
		if(pid == 0)
		{
			//child
			for(k = 0; k < chld_amount; k++)
			{
				if(k != i)
				{
					close(children[i].readfd);
					close(children[i].writefd);
				}

				close(children[i].prnt_writefd);
				close(children[i].prnt_readfd);				
			}
			chldStartTransmission(&children[i]);
		}
		else
		{
			//parent
			close(children[i].readfd);
			close(children[i].writefd);
		}
	}

	//parent
	//start transmission
    
    fd_set readfds;
    fd_set writefds;
    fd_set all_readfds;
    fd_set all_writefds;
    FD_ZERO(&all_readfds);
    FD_ZERO(&all_writefds);

    for(i = 0; i < chld_amount; i++)
    {
    		FD_SET(children[i].prnt_readfd, &all_readfds);
    		FD_SET(children[i].prnt_writefd, &all_writefds);
    }

    //create buffers
	struct buffer *bufs;
	bufs = getBuffers(chld_amount);

	while(0)
	{
		readfds = all_readfds;
		writefds = all_writefds;

		if(select(max_fd+1, &readfds, &writefds, NULL, NULL) == -1)
			exit(EXIT_FAILURE);

		for (i = 0; i < chld_num; ++i)
		{
			handleReadableFd( 
		}
	}



	return 0;
}



int chldConfig(struct child_t *children, int chld_amount)
{
	int i;
	int pipefd[2];
	int max_fd = 1;
	for(i = 0; i < chld_amount; i++)
	{
		children[i].chld_num = i;
		
		if(i == chld_amount-1)
		{
			children[i].last = 1;
			children[i].prnt_writefd = 1;
		}
		else
			children[i].last = 0;

		if(i != 0)
		{
			children[i].first = 0;

			if(pipe(pipe_fd) == -1)
			{
				perror("pipe");
				exit(EXIT_FAILURE);
			}

			if(pipefd[0] > max_fd)
				max_fd = pipefd[0];
			if(pipefd[1] > max_fd)
				max_fd = pipefd[1];

			children[i].readfd = pipefd[0];
			children[i-1].prnt_writefd = pipefd[1];
			fcntl(children[i].prnt_writefd, F_SETFD, O_NONBLOCK);
		}
		else
		{
			children[i].first = 1;
   			if(children[i].readfd = open(argv[2], O_RDONLY) == 0)
   			{
   				perror("open file %s", argv[2]);
   				exit(EXIT_FAILURE);
   			} 
		}
		
		if(pipe(pipe_fd) == -1)
		{
			perror("pipe");
			exit(EXIT_FAILURE);
		}

		if(pipefd[0] > max_fd)
			max_fd = pipefd[0];
		if(pipefd[1] > max_fd)
			max_fd = pipefd[1];

		children[i].writefd = pipefd[1];
		children[i].prnt_readfd = pipefd[0];
		fcntl(children[i].prnt_readfd, F_SETFD, O_NONBLOCK);
	}
}


struct buffer_t *getBuffers(int chld_amount)
{
	struct buffer_t *bufs;
	bufs = (struct buffer_t *) malloc(sizeof(struct buffer_t) * chld_amount);
	if(bufs == NULL)
	{
		perror("malloc");
		exit(EXIT_FAILURE);
	}

	int i;
	int buf_size;
	for(i = 0; i < chld_amount; i++)
	{
		buf_size = 4 * pow(3, chld_amount - i);
		bufs[i]->buf = (char *) malloc(sizeof(char) * buf_size);
		if(bufs[i]->buf == NULL)
	{
		perror("malloc");
		exit(EXIT_FAILURE);
	}
		bufs[i]->buf_size = buf_size;
		bufs[i]->full_sp = 0;
	}
	return bufs;
}

void clearBuffers(struct buffer_t *bufs, int chld_amount)
{
	int i;
	for(i = 0; i < chld_amount; i++)
	{
		free(bufs[i]->buf);
	}
	free(bufs);
}


void chldStartTransmission(struct child_t *child)
{
	int i;
	int ret_val = 0;
	int readfd, writefd;
	readfd = child->readfd;
	writefd = child->writefd;
	char buf[PIPE_SIZE];

	while(1)
	{
		ret_val = read(readfd, &buf, PIPE_SIZE);
        if(ret_val == 0)
        	break;
        if(ret_val == -1)
        {
        	perror("child %d read", child->chld_num);
        	break;
        }

		ret_val = write(writefd, &buf, ret_val);
		if(ret_val ==  -1)
		{
			if(errno == EPIPE)
				break;
			else
			{
				perror("child %d write", child->chld_num);
				break;
			}
		}
		close(readfd);
		close(writefd);
		exit(EXIT_SUCCESS);
	}
    
}

void handleReadableFd(struct child_t *child, fd_set *readfds, 
					  fd_set *all_readfds, fd_set *all_writefds, 
					  struct buffer_t *buf)
{
	if( FD_ISSET(child->prnt_readfd, readfds) )
	{
		int ret_val;
		if(buf->full_sp != 0)
		{
			//some data is in buf
			ret_val = read(child->prnt_readfd, buf->buf, buf_size - buf->full_sp);
			
			if(ret_val == 0)
			{
				//buffer is full
				FD_CLR(child->prnt_readfd, readfds);
				return;
			}

			if(ret_val == -1)
			{
				perror("parent read");
				exit(EXIT_FAILURE);
			}
		}
		else
		{
			if(buf->buf_size > PIPE_SIZE)
				ret_val = read(child->prnt_readfd,  buf->buf, PIPE_SIZE);
			else
				ret_val = read(child->prnt_readfd,  buf->buf, buf->buf_size);

			if(ret_val == -1)
			{
				perror("parent read");
				exit(EXIT_FAILURE);
			}
			if(ret_val == 0)
			{
				FD_CLR(child->prnt_readfd, all_readfds);
				FD_CLR(child->prnt_writefd, all_writefds);
				close(child->prnt_readfd);
				close(child->prnt_writefd);
				return;
			}
			
		}
	}
}

void handleWriteableFd(struct child_t *child, fd_set *writefds, 
					   fd_set *all_readfds, fd_set *all_writefds, 
					   struct buffer_t *buf);
