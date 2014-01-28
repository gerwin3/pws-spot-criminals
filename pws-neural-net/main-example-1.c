#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <time.h>

#include "net.h"
#include "sigmoid.h"

#define NUM_PATTERNS	100000
#define NUM_EPOCHS		25

#define TEST_FUNC(x)	sin (x)

struct pattern {
	double x;
	double y;
};

struct pattern* create_patterns (int n)
{
	int i;

	/* Reserveer geheugen voor n aantal patterns. */
	struct pattern* patterns = (struct pattern*)
		calloc (n, sizeof (struct pattern) );

	for (i = 0; i < n; i++) {
		/* Initialiseer de patronen, de input waarde
		   wordt een willekeurige waarde tussen 0 en
		   1. De output de sinus van de input. */
		patterns[i].x = ( (double) rand () / RAND_MAX);
		patterns[i].y = TEST_FUNC(patterns[i].x);
	}

	return patterns;
}

void evaluate (net_t* net)
{
	int i;

	/* Dit zijn de patterns aan de hand waarvan we
	   het netwerk evalueren. Links de input en
	   rechts the verwachte output. */
	struct pattern tests[10] = {
		{ 0.20		 , TEST_FUNC(tests[0].x) },
		{ 0.7		 , TEST_FUNC(tests[1].x) },
		{ 0.123		 , TEST_FUNC(tests[2].x) },
		{ 0.5050	 , TEST_FUNC(tests[3].x) },
		{ 0.32		 , TEST_FUNC(tests[4].x) },
		{ 0.23423	 , TEST_FUNC(tests[5].x) },
		{ 0.234		 , TEST_FUNC(tests[6].x) },
		{ 0.98		 , TEST_FUNC(tests[7].x) },
		{ 0.235345	 , TEST_FUNC(tests[8].x) },
		{ 0.34534645 , TEST_FUNC(tests[9].x) },
	};

	for (i = 0; i < 10; i++) {
		double out;

		/* Draai het netwerk met de test cases. */
		net_set_in (net, &tests[i].x);
		net_run (net);
		net_get_out (net, &out);

		/* Vergelijk de verwachte output met de 
		   echte output van het netwerk. */
		printf (" [ Evaluatie ] >> Input            : %f\n", tests[i].x);
		printf ("               >> Output           : %f\n", out);
		printf ("               >> Verwachte output : %f\n", tests[i].y);
		printf ("               >> Verschil         : %f (lager is beter)\n", fabs (tests[i].y - out) );
		printf ("\n");
	}
}

int main (int argc, char* argv[])
{
	/* Neem de tijd op. */
	clock_t t = clock ();

	int i, j;

	/* Deze functie maakt NUM_PATTERNS patterns die
	   we gebruiken voor de training. */
	struct pattern* ptrns = create_patterns (NUM_PATTERNS);

	net_t* net;
	int net_setup[] = {1, 20, 1};

	/* We moeten de begin weights laag houden omdat 
	   het netwerk groot is. De vuistregel is:
	    Begin weight = 1 / Max aant. neuronen/laag
	   De 2de laag heeft er honder, dus 1/100. */
	net_init_rnd (0, 0.01);

	/* Initialiseer netwerk met sigmoid function. */
	net = net_create (NET_BIASED, 3, net_setup);
	net->mode = &net_mode_sigmoid;

	/* Draai door alle patterns NUM_EPOCHS keer. */
	for (i = 0; i < NUM_EPOCHS; i++) {
		double net_err = 0;

		/* Trainen op NUM_PATTERNS aantal patterns. */
		for (j = 0; j < NUM_PATTERNS; j++) {
			/* Laat het netwerk deze pattern oplossen. */
			net_set_in (net, &ptrns[j].x);
			net_run (net);

			/* Sla error alvast op. */
			net_err += net_error (net, &ptrns[j].y);

			/* Train het ANN door de weights te
			   veranderen aan de hand van de fouten
			   die het netwerk maakte. */
			net_adjust (net, &ptrns[j].y, 0.28, 0.04);
		}

		printf ("  [ Training ] >> Gemiddelde sum-of-squares error : %f\n", (net_err / NUM_PATTERNS) );
	}

	/* Evalueer het netwerk om te kijken of de training
	   nut heeft gehad. */
	evaluate (net);

	/* Print benodigde tijd op het scherm. */
	printf (" [ Evaluatie ] >> Benodigde tijd : %f seconde",
		    (double) (clock () - t) / CLOCKS_PER_SEC );
	
	/* Verwijder het netwerk. */
	net_free (net);

	/* Verwijder de patterns. */
	free (ptrns);

	/* Wacht op een toest alvorens het scherm te sluiten. */
	getchar ();

	return 0;
}