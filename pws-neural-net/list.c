#include "list.h"

llist_t ll_create ()
{
	llist_t ret =
		{ NULL, NULL };

	return ret;
}

void ll_free (llist_t* l)
{
	while (l->head != NULL) {
		ll_node_pop (l, l->head);
	}
}

ll_node_t* ll_push (llist_t* l, void* v)
{
	if (l->head == NULL)
	{
		/* create new node; fill in references
		 * to next/previous */
		ll_node_t* n =
			(ll_node_t*) malloc (sizeof (ll_node_t));

		n->v = v;
		n->previous = NULL;
		n->next = NULL;

		/* push into list */
		l->head = l->tail = n;
	}
	else {
		/* use the push function, will update
		 * list variables for us */
		ll_node_push (l, l->tail, v);
	}

	return l->tail;
}

int ll_comp_size (llist_t* l)
{
	int ret = 0;
	ll_node_t* n = NULL;

	/* simply loop over items and count until we
	 * find the end of the list */
	for (n = l->head; n->next != NULL; n = n->next) {
		ret++;
	}

	return ret;
}

ll_node_t* ll_node_push (llist_t* l, ll_node_t* n, void* v)
{
	/* create new node; fill in next/previous */
	ll_node_t* pushn =
		(ll_node_t*) malloc (sizeof (ll_node_t));

	pushn->v = v;
	pushn->previous = n;
	pushn->next = n->next;

	/* correct n->next->previous! */
	if (n->next != NULL) {
		n->next->previous = pushn;
	}

	/* push into list */
	n->next = pushn;

	/* correct list values */
	if (l->tail == n) {
		l->tail = pushn;
	}

	return pushn;
}

void* ll_node_pop (llist_t* l, ll_node_t* n)
{
	void* v = n->v;

	/* correct references in next/previous */
	if (n->previous != NULL) {
		n->previous->next = n->next;
	}

	if (n->next != NULL) {
		n->next->previous = n->previous;
	}

	/* correct list values */
	if (l->head == n) {
		l->head = n->next;
	}

	if (l->tail == n) {
		l->tail = n->previous;
	}

	/* release resources */
	free (n);

	return v;
}