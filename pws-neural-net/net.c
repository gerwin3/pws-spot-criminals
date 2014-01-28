#include "net.h"

double net__rnd ()
{
	/*  We normalizeren de waarde uit rand() zodat die tussen rnd.min en
	 *	rnd.max komt te liggen.
	 *
	 *			     ( rand * (max - min) )
	 *  normalized = ______________________ + x
	 *						RAND_MAX
	 *
	 */
	return ( ( ( (double) (rand () ) ) * (rnd.max - rnd.min) ) / RAND_MAX ) + rnd.min;
}

void net__connect_all (net_t* net)
{
	/* Lopers. */
	int c, i, j, k;

	for (i = 0; i < net->num_layers; i++) {
		/* Zolang we niet in de laatste layer zitten, bereken
		   het aantal neuronen tussen deze laag en de volgende
		   om zo tot het totaal aantal neuronen te komen. */
		if (i != net->num_layers - 1) {
			net->num_axons += (net->layers[i].num_neurons *
				               net->layers[i + 1].num_neurons);
		}
	}

	for (i = 0; i < net->num_biases; i++) {
		/* Elke bias is verbonden met 1 hidden layer, per axon
		   is dat dan ook num_neurons van de volgende layer
		   aantal axonen. */
		net->num_axons += net->layers[i + 1].num_neurons;
	}

	/* Maak ruimte voor num_axons aantal axonen. */
	net->axons = (struct axon*)
		calloc (net->num_axons, sizeof (struct axon) );

	for (c = 0, i = 0; i < (net->num_layers - 1); i++) {

		/* Haal referenties op naar laag #i en de layer
		   daarna, waar tussen we axonen aanleggen. */
		struct layer* llayer = &net->layers[i];
		struct layer* rlayer = &net->layers[i + 1];

		for (j = 0; j < llayer->num_neurons; j++) {
			for (k = 0; k < rlayer->num_neurons; k++, c++) {

				/* Neem de volgende axon om in te vullen
				   tussen #j en #k. */
				struct axon* axon = &net->axons[c];

				axon->left		= &llayer->neurons[j];
				axon->right		= &rlayer->neurons[k];
				axon->weight	= net__rnd ();

				/* Voeg ook referenties naar de axon in in
				   de betreffende neuronen. */
				ll_push (&llayer->neurons[j].raxons, axon);
				ll_push (&rlayer->neurons[k].laxons, axon);
			}
		}
	}

	for (i = 0; i < net->num_biases; i++) {
		/* Haal referentie op naar layer van deze bias. */
		struct layer* layer = &net->layers[i + 1];

		for (j = 0; j < layer->num_neurons; j++, c++) {
			/* Neem de volgende axon om in te vullen
			   tussen bias #i en laag #i+1. */
			struct axon* axon = &net->axons[c];

			axon->left		= &net->biases[i];
			axon->right		= &layer->neurons[j];
			axon->weight	= net__rnd ();

			/* Voeg ook referenties naar de axon in in
			   de betreffende neuronen/bias. */
			ll_push (&net->biases[i].raxons, axon);
			ll_push (&layer->neurons[j].laxons, axon);
		}
	}
}

void net_init_rnd (double min, double max)
{
	/* Maak de 'seed' die we nodig hebben voor het in-
	   itialiseren van de 'random number generator'. */
	int seed;
	RND_SEED(seed);
	
	/* Voer seed in. */
	srand (seed);

	/* Update randomizer struct. */
	rnd.min = min;
	rnd.max = max;
}

net_t* net_create (int flags, int nlayers, int* nneurons)
{
	net_t* net =
		(net_t*) calloc (1, sizeof (net_t) );

	int i;

	/* Sla de opties op. */
	net->flags = flags;

	/* Sla het aantal layers op. */
	net->num_layers = nlayers;

	/* Reserveer geheugen voor de layers. */
	net->layers = (struct layer*)
		calloc (net->num_layers, sizeof (struct layer) );

	for (i = 0; i < net->num_layers; i++) {
		/* Haal het aantal neuronen op en sla die op in
		   de laag struct. */
		net->layers[i].num_neurons = *(nneurons++);

		/* Reserveer geheugen voor neuronen. */
		net->layers[i].neurons = (struct neuron*)
			calloc (net->layers[i].num_neurons, sizeof (struct neuron) );
	}

	if (net->flags & NET_BIASED) {
		/* Reserveer ruimte voor een bias voor elke
		   hidden layer. */
		net->num_biases = net->num_layers - 2;
		net->biases = (struct neuron*)
			calloc (net->num_biases, sizeof (struct neuron) );
	}

	for (i = 0; i < net->num_biases; i++) {
		/* Een bias heeft altijd 1 als output. */
		net->biases[i].x = 1.0;
	}

	/* Verbind alle neuronen met elkaar dmv axonen. */
	net__connect_all (net);

	return net;
}

void net_free (net_t* net)
{
	int i, j;
	for (i = 0; i < net->num_layers; i++) {
		/* Verwijder de lijsten van linker en rechter axonen
		   die werden bijgehouden door de neuron. */
		for (j = 0; j < net->layers[i].num_neurons; j++) {
			ll_free (&net->layers[i].neurons[j].laxons);
			ll_free (&net->layers[i].neurons[j].raxons);
		}

		/* Verwijder neuronen. */
		free (net->layers[i].neurons);
	}

	/* Verwijder layers. */
	free (net->layers);

	/* Verwijder biases. */
	free (net->biases);

	/* Verwijder axonen. */
	free (net->axons);

	/* Verwijder netwerk zelf. */
	free (net);
}

void net_set_in (net_t* net, double* in)
{
	int i;
	for (i = 0; i < net->layers[0].num_neurons; i++) {
		/* Kopieer de input waardes naar de neuronen 
		   in layer #0 (eerste laag: input layer). */
		net->layers[0].neurons[i].x = in[i];
	}
}

void net_get_out (net_t* net, double* out)
{
	int i;
	int iback = net->num_layers - 1;
	for (i = 0; i < net->layers[iback].num_neurons; i++) {
		/* Kopieer de output waardes uit de neuronen
		   uit de output layer (laatste layer) naar out. */
		out[i] = net->layers[iback].neurons[i].x;
	}
}

int net_run (net_t* net)
{
	int i, j;

	if (!net->mode) {
		/* De output functions zijn niet gespecificeerd. */	
		return NET_E_INVAL;
	}

	for (i = 1; i < net->num_layers; i++) {
		for (j = 0; j < net->layers[i].num_neurons; j++) {
			/* Haal referentie naar neuron #j uit layer
			   #i op. */
			struct neuron* neuron =
				&net->layers[i].neurons[j];

			double activation = 0;
			ll_node_t* n;

			for (n = neuron->laxons.head; n != NULL; n = n->next) {
				/* Verkrijg huidige axon. */
				struct axon* axon = (struct axon*) n->v;

				/* Vermenigvuldig waarde van de left-connected neuron met
				   de betreffende weight en voeg die to aan de sum.*/
				activation +=
					axon->left->x * axon->weight;
			}

			/* Haal de waarde door de output function en sla hem op. */
			neuron->x =
				net->mode->activ_func (activation);
		}
	}

	return NET_OK;
}

double net_error (net_t* net, double* target)
{
	int i;
	struct layer* outlayer =
		&net->layers[net->num_layers - 1];

	double error = 0.0;

	for (i = 0; i < outlayer->num_neurons; i++) {
		/* Tel de sum-squared-error van deze neuron op bij
		   het totaal. De output van het neuron minus de
		   target output, dat geheel gekwadrateerd en
		   vermenigvuldig met een half.*/
		error +=
			0.5 * pow (target[i] - outlayer->neurons[i].x, 2);
	}

	return error;
}

int net_adjust (net_t* net, double* target, double lr, double mom)
{
	int i, j;
	struct layer* outlayer =
		&net->layers[net->num_layers - 1];

	if (!net->mode) {
		/* De output functions zijn niet gespecificeerd. */	
		return NET_E_INVAL;
	}

	for (j = 0; j < outlayer->num_neurons; j++) {
		/* Referentie naar neuron #j uit de output layer. */
		struct neuron* neuron =
			&outlayer->neurons[j];

		/* Het verschil tussen de target output en de echte
		   output wordt vermenigvuldig met de afgeleide van
		   de output function. */
		neuron->delta = (target[j] - neuron->x) *
						net->mode->activ_deriv_func (neuron->x);
	}

	for (i = (net->num_layers - 2); i > 0; i--) {
		for (j = 0; j < net->layers[i].num_neurons; j++) {
			/* Referentie naar neuron #j uit laag #i, we lopen
			   niet door de output layer.*/
			struct neuron* neuron =
				&net->layers[i].neurons[j];

			double deltas = 0;
			ll_node_t* n;

			/* Loop door alle rechter axonen. */
			for (n = neuron->raxons.head; n != NULL; n = n->next) {
				struct axon* axon =
					(struct axon*) n->v;

				/* Tel de delta van de right-connected neuron
				   vermenigvuldig met de weight op bij het totaal. */
				deltas +=
					(axon->right->delta * axon->weight);
			}

			/* De gevonden wogen deltas kunnen we nu, net als in de
			   output layer, in de afgeleide van de output function
			   stoppen om er de uiteindelijke delta waarde uit the
			   krijgen. */
			neuron->delta = deltas *
							net->mode->activ_deriv_func (neuron->x);
		}
	}

	/* Loop door alle axonen. */
	for (i = 0; i < net->num_axons; i++) {
		struct axon* axon =
			&net->axons[i];

		/* Pas het backpropagation algoritme toe. */
		double weight_d =
			/* Learning rate keer rechter delta keer linker output value. */
			(lr * axon->right->delta * axon->left->x) +
			/* Momentum constante keer het vorige weight verschil. */
			(mom * axon->weight_d);

		/* Sla weight verschil op voor volgende keer. */
		axon->weight_d = weight_d;		

		/* Stel de weight bij! */
		axon->weight += axon->weight_d;
	}

	return NET_OK;
}

net_t* net_read (FILE* f)
{
	int i, j;

	net_t* net;
	int* net_setup;

	uint16_t mode;
	uint32_t flags,
		     nlayers;

	if (feof (f) || ferror (f) ) {
		return NULL;
	}

	/* Check the first bytes to make sure we're reading
	   a valid file. */
	if ( (fgetc (f) != NET_FILE_MAGIC_0) ||
		 (fgetc (f) != NET_FILE_MAGIC_1) ) {
		return NULL;
	}

	/* Reinit random number generator. */
	fread (&rnd.min, sizeof (double), 1, f);
	fread (&rnd.max, sizeof (double), 1, f);

	fread (&mode, sizeof (uint16_t), 1, f);
	fread (&flags, sizeof (uint32_t), 1, f);
	fread (&nlayers, sizeof (uint32_t), 1, f);

	/* Don't create anything if the file wasn't ok. */
	if (ferror (f) ) {
		return NULL;
	}
	
	net_setup = (int*)
		calloc (nlayers, sizeof (int) );

	/* Read network setup information; the number of neurons
	   for each layer. */
	for (i = 0; i < nlayers; i++) {
		fread (&net_setup[i], sizeof (uint32_t), 1, f);
	}

	/* Create network from read info. */
	net = net_create (flags, nlayers, net_setup);

	free (net_setup);

	if (!net) {
		/* Failed to create network. */
		return NULL;
	}

	for (i = 0; i < net->num_layers; i++) {
		for (j = 0; j < net->layers[i].num_neurons; j++) {
			struct neuron* neuron =
				&net->layers[i].neurons[j];

			/* Read neuron. */
			fread (&neuron->x, sizeof (double), 1, f);
			fread (&neuron->delta, sizeof (double), 1, f);
		}
	}

	for (i = 0; i < net->num_axons; i++) {
		/* Read axons. */
		fread (&net->axons[i].weight, sizeof (double), 1, f);
		fread (&net->axons[i].weight_d, sizeof (double), 1, f);
	}

	if (ferror (f) ) {
		/* The file was corrupted, no valid network. */
		free (net);
		return NULL;
	}

	return net;
}

int net_write (net_t* net, FILE* f)
{
	static const double defmode = 0;

	int i, j;

	if (ferror (f) ) {
		return NULL;
	}

	/* Write magic (for file indentification). */
	fputc (NET_FILE_MAGIC_0, f);
	fputc (NET_FILE_MAGIC_1, f);

	/* Write random number generator settings. */
	fwrite (&rnd.min, sizeof (double), 1, f);
	fwrite (&rnd.max, sizeof (double), 1, f);

	fwrite (&defmode, sizeof (uint16_t), 1, f);
	fwrite (&net->flags, sizeof (uint32_t), 1, f);
	fwrite (&net->num_layers, sizeof (uint32_t), 1, f);

	/* Write network setup. */
	for (i = 0; i < net->num_layers; i++) {
		fwrite (&net->layers[i].num_neurons, sizeof (uint32_t), 1, f);
	}

	for (i = 0; i < net->num_layers; i++) {
		for (j = 0; j < net->layers[i].num_neurons; j++) {
			struct neuron* neuron =
				&net->layers[i].neurons[j];

			/* Write neuron. */
			fwrite (&neuron->x, sizeof (double), 1, f);
			fwrite (&neuron->delta, sizeof (double), 1, f);
		}
	}

	for (i = 0; i < net->num_axons; i++) {
		/* Write axon. */
		fwrite (&net->axons[i].weight, sizeof (double), 1, f);
		fwrite (&net->axons[i].weight_d, sizeof (double), 1, f);
	}

	return (!ferror (f) );
}