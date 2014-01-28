#ifndef _FACE_H
#define _FACE_H

#include <vector>

#include "opencv/cv.h"
#include "opencv/highgui.h"

#include "stasm/stasm_lib.h"

namespace ImgPrep {

class NoImageException : public std::exception {};
class StasmFailedException : public std::exception {};
class ImageSizeInvalidException : public std::exception {};

struct FaceLandmark
{
	double x;
	double y;
};

class FaceImage
{
public:

	// Only excepts images which width-height ration equals
	// width_ / height_ ( = 288 / 360) = 0.8
	FaceImage (const std::string& path);

	bool analyze (std::vector<FaceLandmark>* landmarks = NULL, bool redrawImage = true);

	void showWindow ();
	bool save (const std::string& path);

	bool okay () const { return (img_.data != NULL); }

private:

	static const int		width_ = 288;
	static const int		height_ = 360;

	std::string				path_;
	cv::Mat_<unsigned char> img_;

};

}

#endif // _FACE_H