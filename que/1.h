#ifndef H
#define H

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <errno.h>

int malloc_error;

void *bad_malloc(size_t size);

// first elem is the begin of queue, last is the end of queue
struct que_elem
{
	struct que_elem *next;
	int val;
};

struct que
{
	struct que_elem *begin;
	struct que_elem *end;
	int amount;  
};

//----------------------------------------------------
//iterator

struct que_iterator
{
	struct que *queue;
	struct que_elem *current;
	int position;
};

int get_begin( struct que *queue, struct que_iterator *i );

int get_val( struct que_iterator i );
/* returns value of current element */

int get_pos( struct que_iterator i );
/* returns iterator's position */

void incr_iterator( struct que_iterator *i );
/* returns incremented iterator */

int cmp_with_end( struct que_iterator i );
/* returns 0 if i equal to end, 1 if i < end, -1 if i > end */

//int equal_iterator( struct que_iterator i1, struct que_iterator i2 );
/* returns 1 if i1 = i2, else 0 */

//----------------------------------------------------

struct que *que_create();
/* returns NULL if malloc error, else returns queue's pointer */

struct que_elem *que_create_elem();
/* returns pointer to new que's element*/

void que_rm_elem(struct que_elem *elem);
/* returns nothing */

int que_push(struct que *que_id, int val);
/* returns -1 if malloc error, else returns 0 */

int que_pop(struct que *que_id, int *val);
/* returns -1 if queue if empty, else returns 0 */

int que_size(struct que *que_id);
/* returns queue's size */

void que_rm(struct que *que_id);
/* returns nothing */

int que_empty(struct que *que_id);
/* returns 1 if empty, 1 if not empty */ 

#endif
