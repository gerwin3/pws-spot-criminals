#ifdef _DEBUG

#include <stdlib.h>
#include <vector>

#include "face.h"
#include "transform.h"

void debugDrawPoint (std::vector<ImgPrep::FaceLandmark>& lms, int p, cv::Mat& imgLms)
{
	imgLms.at<char>(cv::Point ((int)lms[p].x, (int)lms[p].y)) = 255;
}

void debugDrawLine (std::vector<ImgPrep::FaceLandmark>& lms, int p1, int p2, cv::Mat& imgLms)
{
	cv::line (imgLms,
		cv::Point ((int) lms[p1].x, (int) lms[p1].y),
		cv::Point ((int) lms[p2].x, (int) lms[p2].y),
		cv::Scalar (255,255,255),
		1, 8);
}

void debugDrawMultiLine (std::vector<ImgPrep::FaceLandmark>& lms, int begin, int end, cv::Mat& imgLms)
{
	for (int i = begin; i <= (end - 1); i++) {
		cv::line (imgLms,
			cv::Point ((int) lms[i].x, (int) lms[i].y),
			cv::Point ((int) lms[i+1].x, (int) lms[i+1].y),
			cv::Scalar (255,255,255),
			1, 8);
	}	
}

void debugDrawShape (std::vector<ImgPrep::FaceLandmark>& lms, int begin, int end, cv::Mat& imgLms)
{
	debugDrawMultiLine (lms, begin, end, imgLms);

	cv::line (imgLms,
	cv::Point ((int) lms[end].x, (int) lms[end].y),
	cv::Point ((int) lms[begin].x, (int) lms[begin].y),
	cv::Scalar (255,255,255),
	1, 8);
}

void debugWindow (std::vector<ImgPrep::FaceLandmark>& lms)
{
	// grayscale img matching transform resolution
	cv::Mat imgLms =
		cv::Mat::zeros (ImgPrep::Transform::resolution+1,
		                ImgPrep::Transform::resolution+1,
						CV_8UC1);

	debugDrawShape (lms, 0, 15, imgLms);		// Head
	debugDrawShape (lms, 16, 21, imgLms);		// Left eyebrow
	debugDrawLine (lms, 21, 50, imgLms);		// Left eyebrow to nose
	debugDrawShape (lms, 22, 27, imgLms);		// Right eyebrow
	debugDrawLine (lms, 22, 48, imgLms);		// Right eyebrow to nose

												// 28 and 29; point between eye and eyebrow

	debugDrawShape (lms, 30, 37, imgLms);		// Left eye
	debugDrawPoint (lms, 38, imgLms);			// Left eye middle
	debugDrawPoint (lms, 39, imgLms);			// Right eye middle
	debugDrawShape (lms, 40, 47, imgLms);		// Right eye

	debugDrawPoint (lms, 49, imgLms);			// Nose 1
	debugDrawLine (lms, 48, 54, imgLms);		// Nose 2
	debugDrawLine (lms, 50, 58, imgLms);		// Nose 3
	debugDrawMultiLine (lms, 54, 58, imgLms);	// Nose 4

	debugDrawPoint (lms, 51, imgLms);			// Left Nostril
	debugDrawPoint (lms, 52, imgLms);			// Nose Middle
	debugDrawPoint (lms, 53, imgLms);			// Rigth Nostril

	debugDrawShape (lms, 59, 68, imgLms);		// Upper lip
	debugDrawShape (lms, 69, 76, imgLms);		// Bottom lip

	cv::imshow ("Debug Window", imgLms);
	cv::waitKey ();
}

#endif