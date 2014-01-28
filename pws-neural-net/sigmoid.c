#include "sigmoid.h"

double sigmoid_func (double x)
{
	/*	Sigmoid function:
	 *
	 *	         1
	 *	O = ____________
	 *      ( 1 + e^-x )
	 *
	 */
	return ( (double) 1.0 / (1.0 + exp (-x) ) );
}

double sigmoid_deriv_func (double y)
{
	/*	Afgeleide van de Sigmoid function:
	 *
	 *	O' = y * (1 - y)
	 *
	 */
	return (y * (1.0 - y) );
}