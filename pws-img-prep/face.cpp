#include "face.h"

namespace ImgPrep {

FaceImage::FaceImage (const std::string& path)
{
	path_ = path;

	// load the on-disk image file
	cv::Mat_<unsigned char> tmp (
		cv::imread (path_, CV_LOAD_IMAGE_GRAYSCALE)
	);

	if (!tmp.data) {
		throw NoImageException ();
	}

	if ( (int) tmp.rows * ( (double) width_ / height_) != (int) tmp.cols) {
		// ratio between width and height MUST be
		// 0.8 to maintain face borders
		throw ImageSizeInvalidException ();
	}

	// create grayscale image on pre-determined
	// dimensions
	img_ = cv::Mat (width_, height_, CV_8UC1);

	// resize loaded image to fit into img_
	cv::resize (tmp,
		        img_,
				cv::Size (width_, height_) );
}

bool FaceImage::analyze (std::vector<FaceLandmark>* landmarks, bool redrawImage)
{
	float resLandmarks[2 * stasm_NLANDMARKS];
	int resFound = 0;

	// Look for facial features, landmarks will be written to
	// resLandmarks
	if (!stasm_search_single ( (int*) &resFound,
			                    resLandmarks,
								(char*) img_.data,
								img_.cols,
								img_.rows,
								path_.c_str (), "data")) {
		throw StasmFailedException ();
	}

	if (!resFound) {
		// No face was found.
		return false;
	}

	if (landmarks) {
		for (int i = 0; i < stasm_NLANDMARKS; i++)
			// add point to output landmarks
			landmarks->push_back ( { (double) resLandmarks[i*2],
			                         (double) resLandmarks[i*2+1] } );
	}

	if (redrawImage)
	{
		// Redraw the image with landmarks in white

		stasm_force_points_into_image (resLandmarks,
										img_.cols,
										img_.rows);

		for (int i = 0; i < stasm_NLANDMARKS; i++) {
			img_ (cvRound (resLandmarks[i*2+1]),
				  cvRound (resLandmarks[i*2]) ) = 255;
		}
	}

	return true;
}

void FaceImage::showWindow ()
{
	cv::imshow ("Result image", img_);
	cv::waitKey ();
}

bool FaceImage::save (const std::string& path)
{
	return cv::imwrite (path.c_str(), img_);
}

}