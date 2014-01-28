#ifndef _LIST_H
#define _LIST_H

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

typedef struct ll_node
{
	void* v;
	struct ll_node* next;
	struct ll_node* previous;
}
ll_node_t;

/*
 *	llist_t
 *	-------
 *	 => head - ptr to first node
 *	 => tail - ptr to last node
 *
 *	- example:
 *		struct llist_node* n;
 *		for (n = l->head; n->next != NULL; n = n->next) { ... }
 */
typedef struct
{
	ll_node_t* head;
	ll_node_t* tail;
}
llist_t;

llist_t ll_create ();
void ll_free (llist_t* l);

ll_node_t* ll_push (llist_t* l, void* v);
int ll_comp_size (llist_t* l);

ll_node_t* ll_node_push (llist_t* l, ll_node_t* n, void* v);
void* ll_node_pop (llist_t* l, ll_node_t* n);

#endif