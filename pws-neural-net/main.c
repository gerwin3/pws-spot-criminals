#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "net.h"
#include "sigmoid.h"

#define DATA_NUM_LANDMARKS		77
#define DATA_MAX_PRECISION		15
#define DATA_MAX_LINE			(2 * DATA_MAX_PRECISION) + 2
#define DATA_RESOLUTION			200

#define DATA_SEQ_PATH_0			"J:\\Datasets\\dataset_faces_lms\\%d.lms"
#define DATA_SEQ_PATH_1			"J:\\Datasets\\dataset_inmates_lms\\t000%d.lms"
#define DATA_PATH_0				"data/ps0.dat"
#define DATA_PATH_1				"data/ps1.dat"

#define NET_PATH				"data/net-optim.dat"

#define NET_NUM_EPOCHS			1
#define NET_TRAINING_SHARE		0.7

#define NET_INIT_WEIGHTS_MIN	0
#define NET_INIT_WEIGHTS_MAX	0.01

#define NET_NUM_LAYERS			3
#define NET_LIN_NUM_NEURONS		(DATA_NUM_LANDMARKS * 2)
#define NET_LOUT_NUM_NEURONS	1
#define NET_LH1_NUM_NEURONS		100				
#define NET_LH2_NUM_NEURONS		unused
#define NET_LH3_NUM_NEURONS		unused

#define NET_LEARNING_RATE		0.20
#define NET_MOMENTUM			0.80

/*
 *	By turning on the FINETUNING flag while compiling,
 *	several network variables can be adjusted on runtime by
 *	chaning the g_* global values.
 */
#define FINETUNING 0

#if FINETUNING

static int g_NET_LH1_NUM_NEURONS = 100;
static double g_NET_LEARNING_RATE = 0.30;
static double g_NET_MOMENTUM = 0.50;

 #define NET_LH1_NUM_NEURONS	g_NET_LH1_NUM_NEURONS
 #define NET_LEARNING_RATE		g_NET_LEARNING_RATE
 #define NET_MOMENTUM			g_NET_MOMENTUM

#endif

static double g_NET_EPOCH_AVG_ERR = 0;
static double g_NET_EVALUATION = 0;

typedef struct {
	struct {
		double x,
		       y;
	} x[DATA_NUM_LANDMARKS];
} face_lms_t;

typedef struct {
	face_lms_t	in;
	double		target;
} pattern_t;

/*
 *	f_lms_parse; Parses a .lmk file. This type of file
 *		holds x and y coordinates of 77 facial landmarks
 *		found by stasm.
 */
int f_lms_parse (const char* floc, face_lms_t* lms)
{
	int j;
	FILE* f = fopen (floc, "r");

	if (f) {
		char line[DATA_MAX_LINE];

		/* read the 77 facial landmarks */
		for (j = 0; j < DATA_NUM_LANDMARKS; j++) {
			/* get line */
			if (!fgets (line, DATA_MAX_LINE, f) ) {
				return j; /* failed to read */
			}

			/* read two space-seperated double x and y
			   from the line */
			sscanf (line,
					"%lf %lf",
					&lms->x[j].x,
					&lms->x[j].y);

			/* divide by resolution */
			lms->x[j].x /= (double) DATA_RESOLUTION;
			lms->x[j].y /= (double) DATA_RESOLUTION;
		}

		fclose (f);

 		return DATA_NUM_LANDMARKS;
	} else {
		return -1; /* can't open */
	}
}

/*
 *	f_lms_read_seq_0; Reads LandMarkS-files in bulk
 *		from DATA_SEQ_PATH_0. Allocates ps for you,
 *		returns number of patterns read.
 */
int f_lms_read_seq_0 (pattern_t** ps)
{
	static const int begin = 1,
		             end   = 22511;
	int i, c;
	*ps = (pattern_t*)
		calloc ( (end - begin), sizeof (pattern_t) );

	for (i = begin, c = 0; i < end; i++) {
		int ret;
		char loc[MAX_PATH];

		/* construct current file location */
		sprintf (loc, DATA_SEQ_PATH_0, i);

		ret = f_lms_parse (loc, &(*ps)[c].in);

		if (ret == DATA_NUM_LANDMARKS) {
			/* this probably ain't no criminal */
			(*ps)[c].target = 0;

			c++;
		} else if (ret > 0) {
			printf (" [ fail ] >> read err/malformed %d\n", i);
		} else {
			/* skipped, whatever */
		}		
	}

	/* return number of successful entries */
	return c;
}

/*
 *	f_lms_read_seq_1; Reads LandMarkS-files in bulk
 *		from DATA_SEQ_PATH_1. Allocates ps for you,
 *		returns number of patterns read.
 */
int f_lms_read_seq_1 (pattern_t** ps)
{
	static const int begin = 1000000,
		             end   = 1030000;
	int i, c;
	*ps = (pattern_t*)
		calloc ( (end - begin), sizeof (pattern_t) );

	for (i = begin, c = 0; i < end; i++) {
		int ret;
		char loc[MAX_PATH];

		/* construct current file location */
		sprintf (loc, DATA_SEQ_PATH_1, i);

		ret = f_lms_parse (loc, &(*ps)[c].in);

		if (ret == DATA_NUM_LANDMARKS) {
			/* this is a criminal for sure because it's from
			   the inmates set. */
			(*ps)[c].target = 1;

			c++;
		} else if (ret > 0) {
			printf (" [ fail ] >> read err/malformed %d\n", i);
		} else {
			/* skipped, whatever */
		}
	}

	/* return number of successful entries */
	return c;
}

/*
 *	f_ps_write; Contencates multiple patterns (ps) into one
 *		single file (loc). Returns the actual number of patt-
 *		erns written, -1 on failure.
 */
int f_ps_write (const char* loc, pattern_t* ps, int ps_num)
{
	int ret;
	int i;
	FILE* f = fopen (loc, "wb");

	if (!f) {
		return -1;
	}

	for (i = 0; i < ps_num; i++) {
		/* write xy's of facial landmarks */
		ret = fwrite ( (void*) &ps[i].in.x,
					  2 * sizeof (double),
					  DATA_NUM_LANDMARKS,
					  f);

		if (ret <= 0) {
			goto err;
		}

		/* write target output (criminal or not) */
		ret = fwrite ( (void*) &ps[i].target,
			           sizeof (double),
					   1, f);

		if (ret <= 0) {
			goto err;
		}
	}

	fclose (f);
	return i;

err:
	fclose (f);
	return -1;
}

/*
 *	f_lms_2_ps_file; Contencates patterns, they are read
 *		by their respective bulk functions and written to
 *		DATA_PATH_0 and DATA_PATH_1.
 */
int f_lms_2_ps_file ()
{
	pattern_t* ps0;
	pattern_t* ps1;

	int ps0n =
		f_lms_read_seq_0 (&ps0);

	int ps1n =
		f_lms_read_seq_1 (&ps1);

	if (!(ps0n && ps1n) ) {
		return -1;
	}

	return (f_ps_write (DATA_PATH_0, ps0, ps0n) &&
		    f_ps_write (DATA_PATH_1, ps1, ps1n) ) ? 0 : -1;
}

/*
 *	f_ps_read; Reads multiple patterns from a file (loc)
 *		written by f_ps_write. Allocates memory in ps,
 *		returns number of patterns read, -1 on failure.
 */
int f_ps_read (const char* loc, pattern_t** ps)
{
	int ret;
	int i;
	int ps_num;

	int fsize;
	FILE* f = fopen (loc, "rb");
	
	if (!f) {
		return -1;
	}

	/* get file size */
	fseek (f, 0, SEEK_END);
	fsize = ftell (f);
	fseek (f, 0, SEEK_SET);

	/* calculate number of items in file */
	ps_num =
		fsize / ( (DATA_NUM_LANDMARKS *
		        (2 * sizeof (double) ) ) +
				sizeof (double) );

	/* alloc mem on heap */
	*ps = (pattern_t*)
		calloc (ps_num, sizeof (pattern_t) );

	for (i = 0; i < ps_num; i++) {
		pattern_t* p = &(*ps)[i];
		
		/* read input values, the landmarks xy's */
		ret = fread ( (void*) &p->in.x,
			         (2 * sizeof (double) ),
				     DATA_NUM_LANDMARKS,
				     f);

		if (ret <= 0) {
			goto err;
		}

		/* read target ouput (0 or 1) */
		ret = fread ( (void*) &p->target,
			         sizeof (double),
					 1, f);

		if (ret <= 0) {
			goto err;
		}
	}

	fclose (f);
	return i;

err:
	fclose (f);
	return -1;
}

/*
 *	f_net_save; Saves the network to the default location.
 *		 Returns -1 on failure, 0 on success;
 */
int f_net_save (net_t* net, const char* floc)
{
	int ret;
	FILE* f = fopen (floc, "wb");

	if (f) {
		ret = net_write (net, f);
	} else {
		return -1;
	}

	fclose (f);

	return ret;
}

/*
 *	f_net_open; Opens the network from the default location.
 *		Returns a newly allocated pointer to the network on
 *		success, NULL otherwise.
 */
net_t* f_net_open (const char* floc)
{
	FILE* f = fopen (floc, "rb");
	net_t* net;

	if (f) {
		net = net_read (f);
	} else {
		return NULL;
	}

	fclose (f);

	return net;
}

/*
 *	net_learn; Make the network learn.
 */
void net_learn (const char* finloc, const char* foutloc)
{
	int i, j,
		ps0_i = 0,
		ps1_i = 0,
		good = 0,
		wrong = 0;

	/* load the patterns, ps0 contains normal faces while
	   ps1 contains detainees */
	pattern_t* ps0;
	pattern_t* ps1;

	int ps0_num = f_ps_read (DATA_PATH_0, &ps0),
		ps1_num = f_ps_read (DATA_PATH_1, &ps1);

	/* number of patterns used for training */
	int train_num =
		(int) (NET_TRAINING_SHARE * (ps0_num + ps1_num) );

	/* number of patterns used for evaluation */
	int eval_max =
		( (ps0_num < ps1_num) ? ps0_num : ps1_num);

	net_t* net;

	printf (" [ info ] >> loaded %d patterns in set 0\n", ps0_num);
	printf (" [ info ] >> loaded %d patterns in set 1\n", ps1_num);

	/* balance weights with network */
	net_init_rnd (NET_INIT_WEIGHTS_MIN,
		          NET_INIT_WEIGHTS_MAX);

	net = (finloc != NULL) ? f_net_open (finloc) : 0;

	if (!net) {
		int net_setup[NET_NUM_LAYERS] =
			{ NET_LIN_NUM_NEURONS,
			  NET_LH1_NUM_NEURONS,
			  NET_LOUT_NUM_NEURONS };

		/* couldn't find file, create new network. */
		net = net_create (NET_BIASED,
			              NET_NUM_LAYERS,
						  net_setup);

		if (!net) {
			printf (" [ fail ] >> failed to make net");
			goto out;
		}
	}

	net->mode = &net_mode_sigmoid;

	/* phase 1: training */

	for (i = 0; i < NET_NUM_EPOCHS; i++) {
		double sum_err = 0;
		ps0_i = ps1_i = 0;

		for (j = 0; j < train_num; j++) {
			pattern_t* p = (j % 2 == 0)
								? &ps0[ps0_i++]
								: &ps1[ps1_i++];

			net_set_in (net, (double*) &p->in.x);
			net_run (net);
			
			sum_err += net_error (net, &p->target);

			net_adjust (net,
				        &p->target,
				        NET_LEARNING_RATE,
						NET_MOMENTUM);

			if (j % 10000 == 5000) {
				printf (" [ info ] >> current avg. error : %f\n",
					    sum_err / j);
			}
		}

		printf (" [ info ] >> epoch avg. error   : %f\n",
			    sum_err / train_num);

		g_NET_EPOCH_AVG_ERR = sum_err / train_num;
	}

	/* phase 2: evaluation */

	for (i = ps0_i; i < eval_max; i++) {
		double out;

		net_set_in (net, (double*) &ps0[i].in);
		net_run (net);
		net_get_out (net, &out);

		if (out < 0.5) {
			good++;
		} else {
			wrong++;
		}
	}

	for (i = ps1_i; i < eval_max; i++) {
		double out;

		net_set_in (net, (double*) &ps1[i].in);
		net_run (net);
		net_get_out (net, &out);

		if (out > 0.5) {
			good++;
		} else {
			wrong++;
		}
	}

	g_NET_EVALUATION = (double) good / (double) (good + wrong) * 100;

	printf (" [ info ] >> evaluation: %f\%%\n",
		    ( (double) good / (double) (good + wrong) * 100 ) );

	/* save the network in either the default path
	   or the given path. */
	f_net_save (net, (foutloc != NULL) ? foutloc : NET_PATH);

	printf (" [ info ] >> network saved\n");

out:
	if (net)
		net_free (net);

	if (ps0)
		free (ps0);

	if (ps1)
		free (ps1);
}

/*
 *	net_test; Make the network test a face. Returns percentage
 *		chance this is a criminal.
 */
int net_test (const char* f_lms_loc)
{
	net_t* net;
	face_lms_t lms;
	double out;

	/* balance weights with network */
	net_init_rnd (NET_INIT_WEIGHTS_MIN,
		          NET_INIT_WEIGHTS_MAX);

	/* 1. open network */

	net = f_net_open (NET_PATH);
	if (!net) {
		printf (" [ fail ] >> failed to open network at %s\n", NET_PATH);
		goto out;
	}

	net->mode = &net_mode_sigmoid;

	/* 2. open landmarks */

	if (f_lms_parse (f_lms_loc, &lms) != DATA_NUM_LANDMARKS) {
		printf (" [ fail ] >> failed to open landmarks-file at %s", f_lms_loc);
		goto out;
	}

	/* 3. run network */

	net_set_in (net, (double*) &lms.x);
	net_run (net);
	net_get_out (net, &out);

	/* 4. result */

	out *= 100.0; /* factor to percentage */

	printf (" [ info ] >> de persoon in %s is %f%% crimineel!\n",
		    f_lms_loc,
			out);

	return out;

out:
	if (net)
		net_free (net);

	return -1;
}

void usage ()
{
	printf ("usage: pws-neural-net [L/l|T/t] <t-lms-file>\n");
	printf ("         L/l -> Start or continue learning.\n");
	printf ("         T/t -> Test <t-lms-file>.\n");
}

int main (int argc, char* argv[])
{
#ifdef _DEBUG

	net_test ("in.lms");

	getchar ();

#else

	int ret = 0;

	switch (argc) {
		case 2: {
			if (argv[1] == "L" || argv[1] == "l") {
				net_learn (NULL, NULL);
			} else {
				usage ();
			}
		} break;
		case 3: {
			if (argv[1] == "T" || argv[1] == "t") {
				ret = net_test (argv[2]);
			} else if (argv[1] == "L" || argv[1] == "l") {
				net_learn (argv[2], argv[2]);
			} else {
				usage ();
			}
		}
		default:
			usage ();
	}

	return ret;

#endif
}