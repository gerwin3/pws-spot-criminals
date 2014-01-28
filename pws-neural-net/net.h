#ifndef _NET_H
#define _NET_H

#include <stdlib.h>
#include <stdarg.h>
#include <math.h>

#include "list.h"

/* Definieer RND_SEED, deze macro wordt gebruikt om geoptimaliseerde
   'seeds' te maken om de 'random number generator' mee te initialiseren.
   Voor Windows en Linux is deze macro sterk, voor andere platformen
   zwakker. */
#if defined(_WIN32)
 #include "Windows.h"
 #define RND_SEED(s) \
	s = GetTickCount ();
#elif defined(__linux__)
 #include <time.h>
 #define RND_SEED(s) \
	struct timespec time; \
	clock_gettime (CLOCK_MONOTONIC, &time); \
	s = (int) time.tv_nsec;
#else
 #include <time.h>
 #define RND_SEED(s) \
	s = clock ();
#endif

#define NET_OK				0
#define NET_E_INVAL			-1

#define NET_BIASED			1
#define NET_FILE_MAGIC_0	0x84
#define NET_FILE_MAGIC_1	0xef

/* De randvoorwaardes van de 'random number generator' zijn
   standaard ingesteld op minimal -1 en maximal 1. */
static struct rnd {
	double min;
	double max;
} rnd = { -1.0, 1.0 };

/* Defineer de data types activ_func_t en activ_deriv_func_t
   die een verwijzing houden naar een implementatie van de
   output function and de bijbehorende afgeleide. */
typedef double (*activ_func_t) (double x);
typedef double (*activ_deriv_func_t) (double x);

/* De output function en afgeleide worden gegroepeerd in deze
   struct. */
struct net_mode {
	activ_func_t activ_func;
	activ_deriv_func_t activ_deriv_func;
};

/*
 *	ANN Structs
 */

struct axon {
	struct neuron* left;
	struct neuron* right;
	double weight;
	double weight_d;
};

struct neuron {
	llist_t laxons;
	llist_t raxons;
	double x;
	double delta;
};

struct layer {
	int num_neurons;
	struct neuron* neurons;
};

typedef struct net {
	int num_layers;
	struct layer* layers;
	int num_biases;
	struct neuron* biases;
	int num_axons;
	struct axon* axons;
	struct net_mode* mode;
	int flags;
} net_t;

/* Initialiseer de 'random number generator'. */
void net_init_rnd (double min, double max);

/* Zet een nieuw netwerk op met nlayers aantal layers en
   daarna door komma gescheiden aantallen neuronen per
   layer. */
net_t* net_create (int flags, int nlayers, int* nneurons);

/* Verwijder netwerk. */
void net_free (net_t* net);

void net_set_in (net_t* net, double* in);
void net_get_out (net_t* net, double* out);

/* Draai het netwerk. De input waardes kunnen worden gezet
   met net_set_in() en de output waardes opgehaald met
   net_get_out(). */
int net_run (net_t* net);

/* Verkrijg de sum-of-squares error van het netwerk. */
double net_error (net_t* net, double* target);

/* Train het netwerk. De target outputs worden gebruikt
   om het netwerk bij te stellen. */
int net_adjust (net_t* net, double* target, double lr, double mom);

/* Read network from stream <f>, just like net_create,
   this network must be net_free'd. */
net_t* net_read (FILE* f);

/* Write network to stream <f>. */
int net_write (net_t* net, FILE* f);

#endif