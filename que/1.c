#include "1.h"

void *bad_malloc(size_t size)
{
	if(malloc_error == 1) return NULL;
	return malloc(size);
};

//------------------------------------------------
//iterator

int get_begin( struct que *queue, struct que_iterator *i )
{
	if( queue -> amount == 0 )
		return -1;
	i -> queue = queue;
	i -> current = queue -> begin;
	i -> position = 0;
	return 1;	
};

/*struct que_iterator get_end( struct que *queue )
{
	struct que_iterator i;
	i.queue = queue;
	i.current = queue -> end;
	i.position = queue -> amount -1;
	return i;
};*/

int cmp_with_end( struct que_iterator i )
{
	if ( i.queue -> amount - 1 == i.position )
		return 0;
	else if( i.queue->amount - 1 > i.position )
		return 1;
	else 
		return -1;
};

int get_val( struct que_iterator i )
{
	int value;
	value = i.current -> val; 
	return value; 
};

int get_pos( struct que_iterator i )
{
	int pos;
	pos = i.position;
	return pos;
};

void incr_iterator( struct que_iterator *i )
{
	struct que_elem *next;
	next = i -> current -> next;
	i -> current = next;
	i->position++;
	return;
};

/*int equal_iterator( struct que_iterator i1, struct que_iterator i2 )
{
//	if( i1.position == -1 || i2.position == -1 )
//		retunr -1;
	if( i1.position == i2.position )
		return 1;
	else 
		return 0;
};*/

//------------------------------------------------

struct que *que_create()
{
	struct que *que_id;
	que_id = bad_malloc(sizeof(struct que));
	if(que_id == NULL)
	{
		//printf("bad_malloc error\n");
		return NULL;
	}
	que_id->amount = 0;
	
	return que_id;
};

struct que_elem *que_create_elem()
{
	struct que_elem *new;
	new = bad_malloc(sizeof(struct que_elem));
	if(new == NULL)
    {
		//printf("bad_malloc error\n");
		return NULL;
	}
	
	return new;
};

int que_push(struct que *que_id, int val)
{
	struct que_elem *new;
	new = que_create_elem();
	if(new == NULL)
		return -1;

	if(que_id->amount == 0)
	{
		que_id->begin = new;
	}
	else
	{
	    que_id->end->next = new;
	}
	que_id->end = new;
    new->val = val;
	que_id->amount++;
	
	return 0;
};

void que_rm_elem(struct que_elem *elem)
{
	free(elem);
};

int que_pop(struct que *que_id, int *val)
{
	if(que_id->amount == 0)
	{
		//printf("que is empty\n");
		return -1;
	}
	struct que_elem *poped;
    poped = que_id->begin;
	
	*val = poped->val;
    que_id->begin = poped->next;
    
    que_rm_elem(poped);
    que_id->amount--;
    
    return 0;

};

int que_size(struct que *que_id)
{
	return que_id->amount;
};

int que_empty(struct que *que_id)
{
	if(que_id->amount) return 0;
	return 1;
};

void que_rm(struct que *que_id)
{
	struct que_elem *del;
	for( ; que_id->amount != 0; que_id->amount--)
	{
		del = que_id->begin;
		que_id->begin = del->next;
		free(del);
	}
	free(que_id);
};
