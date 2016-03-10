#ifndef H
#define H

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <errno.h>

int malloc_error;

struct que_elem
{
	struct que_elem *next;
	int val;
};

struct que
{
	struct que_elem *head;
	struct que_elem *tail;
	int amount;  
};
void *bad_malloc(size_t size);

struct que *que_create();
/* returns NULL if malloc error, else returns queue's pointer */

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
