#ifndef _TRANSFORM_H
#define _TRANSFORM_H

#include <math.h>

#include "face.h"

namespace ImgPrep {

class MalformedFaceException : public std::exception {};
class FatFaceException : public MalformedFaceException {};

class Transform
{
public:

	static const int resolution	= 200;

	Transform (std::vector<FaceLandmark>& landmarks);

	void normalize ();

private:

	static const int landmark_top_		= 14;
	static const int landmark_bottom_	= 6;

	double calcTargetAngle ();
	
	void rotateLandmark (FaceLandmark& lm, double angle);
	void normalizeBounds ();

	std::vector<FaceLandmark>& landmarks_;

};

}

#endif // _TRANSFORM_h