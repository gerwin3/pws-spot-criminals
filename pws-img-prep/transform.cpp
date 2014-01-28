#include "transform.h"

namespace ImgPrep {

Transform::Transform (std::vector<FaceLandmark>& landmarks)
	: landmarks_ (landmarks) {}

void Transform::normalize ()
{
	// acquire angle for normalization
	double targetAngle = calcTargetAngle ();

	for (unsigned int i = 0; i < landmarks_.size (); i++) {
		// rotate the current landmark
		rotateLandmark (landmarks_[i], targetAngle);
	}

	// narrow crop
	normalizeBounds ();
}

double Transform::calcTargetAngle ()
{
	FaceLandmark top	= landmarks_[landmark_top_],
				 bottom = landmarks_[landmark_bottom_];

	enum Direction {
		Left, Right
	} direction = Left;

	if (top.x == bottom.x) {
		// no normalizaiton required
		return 0;
	} else if (top.x < bottom.x) {
		direction = Right;
	} else {
		direction = Left;
	}

	double diffHorizontal = fabs (top.x - bottom.x);
	double diffVertical = bottom.y - top.y;

	if (diffVertical <= 0) {
		// top was beneath bottom
		throw MalformedFaceException ();
	}

	double angle = atan (diffHorizontal / diffVertical);

	// rotation to the left yields an negative value
	return (direction == Right) ? angle : -angle;
}

void Transform::rotateLandmark (FaceLandmark& p, double angle)
{
	double ox = landmarks_[landmark_bottom_].x,
		   oy = landmarks_[landmark_bottom_].y;

	double s = sin (angle),
		   c = cos (angle);

	FaceLandmark p2 = {
		c * (p.x - ox) - s * (p.y - oy) + ox,
		s * (p.x - ox) + c * (p.y - oy) + oy
	};

	p = p2;
}

void Transform::normalizeBounds ()
{
	//       <----- a ----->
	//   _______________________
	//	|   |				|   |
	//	|   |				|   |
	//	|   |				|   |
	//	|   |				|   |
	//	| x |		LMS 	| x | height: y
	//	|   |				|   |
	//	|   |				|   |
	//	|   |				|   |
	//	|___|_______________|___|
	//
	//	y = 200
	//  2x+a = 200

	double top = resolution,
		   bottom = 0,
		   left = resolution,
		   right = 0;

	// find outer bounds
	for (unsigned int i = 0; i < landmarks_.size (); i++) {
		if (landmarks_[i].y < top)
			top = landmarks_[i].y;

		if (landmarks_[i].y > bottom)
			bottom = landmarks_[i].y;

		if (landmarks_[i].x < left)
			left = landmarks_[i].x;

		if (landmarks_[i].x > right)
			right = landmarks_[i].x;
	}

	// check for fatfaces
	if ( (right - left) > (bottom - top) ) {
		throw FatFaceException ();
	}

	double factor =
		(double) (resolution - 1) / (bottom - top); // factor needed to fit height in

	 // horizontal margins needed for canvas size
	double margin =
		( (double) resolution - ( (right - left) * factor) ) / 2;

	// adjust the points
	for (unsigned int i = 0; i < landmarks_.size (); i++) {
		// remapping point
		landmarks_[i].x =
			(landmarks_[i].x - left) * factor + margin;
		landmarks_[i].y =
			(landmarks_[i].y - top) * factor;
	}
}

}