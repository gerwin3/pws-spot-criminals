#ifndef _SIGMOID_H
#define _SIGMOID_H

#include <stdlib.h>
#include <math.h>

#include "net.h"

double sigmoid_func (double x);
double sigmoid_deriv_func (double x);

static struct net_mode net_mode_sigmoid =
	{ &sigmoid_func, &sigmoid_deriv_func };

#endif