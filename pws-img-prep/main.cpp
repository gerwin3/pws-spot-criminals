#include <iostream>
#include <sstream>
#include <fstream>

#include "face.h"
#include "transform.h"

void writeLandmarksToFile (const std::string& path, const std::vector<ImgPrep::FaceLandmark>& lms)
{
	static const int fDecimalPrec = 15; // max safe precision for doubles

	std::ofstream fOut (path, std::ios::out | std::ios::binary);

	if (fOut.is_open ()) {
		// write all landmarks seperated by newlines, x and
		// y are seperated by a space
		for (int i = 0; i < lms.size (); i++) {
			fOut << std::setprecision (9) << lms[i].x << " " << lms[i].y << std::endl;
		}

		fOut.close ();
	} else {
		throw std::exception ("Failed to open file.");
	}
}

void usage ()
{
	std::cout << "usage : pws-img-prep <in> <out>" << std::endl
			  << "          in -> (optional) Input image file" << std::endl
			  << "          out -> (optional) Output LMS-file" << std::endl
			  << std::endl
			  << "typing 'pws-img-prep ?' prints this message." << std::endl;
}

int main (int argc, char* argv[])
{
	std::string in = "data/gb6.jpg";
	std::string out = "out/gb6.lms";

	/* Parse input and output file names if available,
	   print usage () on error. */
	if (argc > 1) {
		if (argv[1] == "?") {
			usage ();
			return 0;
		}
		in = argv[1];
		if (argc > 2) {
			out = argv[2];
			if (argc > 3) {
				usage ();
				return 0;
			}
		}
	}

	std::vector<ImgPrep::FaceLandmark> lms;

	try {
		ImgPrep::FaceImage (in).analyze (&lms);	/* 1. Locate facial features */
		ImgPrep::Transform (lms).normalize ();	/* 2. Normalize locations */

		writeLandmarksToFile (out, lms);		/* 3. Store output in file */
	} catch (ImgPrep::ImageSizeInvalidException e) {
		std::cout << " [ fail ] >> Image width-height-ratio must be 0.8!" << std::endl;
	} catch (std::exception e) {
		std::cout << " [ fail ] >> " << e.what () << std::endl;
		return -1;
	}

	return 0;
}

//
//	analyzeThatsMyFace; Used for bulk converting all
//		images of the ThatMyFace dataset to LandMarkS
//		files.
//
void analyzeThatsMyFace ()
{
	std::cout << " [ Info ] Analyzing THATSMYFACE dataset ... " << std::endl;

	static const int idStart	= 00001,
					 idEnd		= 22512;

	int numParsed = 0,
		numFailed = 0,
		numSkipped = 0;

	clock_t t = clock ();

	for (int i = idStart; i < idEnd; i++) {
		// compile path of current image, all images from
		// <idStart>.jpg to <idEnd>.jpg are valid.
		std::stringstream imgPath;
		imgPath << "J:\\Datasets\\dataset_faces\\"
			    << std::setfill ('0') << std::setw (5) << i
				<< ".jpg";

		// path for the LandMarkS-file.
		std::stringstream lmsPath;
		lmsPath << "J:\\Datasets\\dataset_faces_lms\\"
			    << i
				<< ".lms";

		try {
			// load the image
			ImgPrep::FaceImage imgInmate (imgPath.str() );

			std::vector<ImgPrep::FaceLandmark> lms;

			if (imgInmate.analyze (&lms) ) {
				// the image has been analyzed successfully, a
				// face has been detected, transform the land-
				// marks so they aren't influencted by tilt,
				// and size anymore.
				ImgPrep::Transform transform (lms);
				transform.normalize ();

				// write landmarks to file
				writeLandmarksToFile (lmsPath.str (), lms);

				numParsed++;
				std::cout << " [ Info ] >> Successfully parsed '" << i << ".jpg'" << std::endl;
			} else {
				numFailed++;
				std::cout << " [ Info ] >> No face found in '" << i << ".jpg'" << std::endl;
			}
		} catch (ImgPrep::NoImageException e) {
			numSkipped++;
		} catch (ImgPrep::StasmFailedException e) {
			numFailed++;
			std::cout << " [ Fail ] >> Stasm failed (#" << i << ")." << std::endl;
		} catch (ImgPrep::MalformedFaceException e) {
			numFailed++;
			std::cout << " [ Fail ] >> Malformed face (#" << i << ")." << std::endl;
		} catch (std::exception e) {
			numFailed++;
			std::cout << " [ Fail ] >> Error: " << e.what () << " (#" << i << ")." << std::endl;
		}
	}

	std::cout << " [ Info ] >> All entries have been processed. Results: " << std::endl
		      << "          >> Success    : " << numParsed << std::endl
			  << "          >> Failed     : " << numFailed << std::endl
			  << "          >> Skipped    : " << numSkipped << std::endl
			  << "          >> Time taken : " << ( (clock () - t) / CLOCKS_PER_SEC) << std::endl;

	std::cin.get ();
}

//
//	analyzeMugshots; Used for bulk converting all
//		images of the Mugshots dataset to LandMarkS
//		files.
//
void analyzeMugshots ()
{
	std::cout << " [ Info ] Analyzing MUGSHOTS dataset ... " << std::endl;

	static const int idStart	= 0001000000,
					 idEnd		= 0001030000;

	int numParsed = 0,
		numFailed = 0,
		numSkipped = 0;

	clock_t t = clock ();

	for (int i = idStart; i < idEnd; i++) {
		// compile path of current image, all images from
		// <idStart>.jpg to <idEnd>.jpg are valid.
		std::stringstream imgPath;
		imgPath << "J:\\Datasets\\dataset_inmates\\t000"
			    << i
				<< ".jpg";

		// path for the LandMarkS-file.
		std::stringstream lmsPath;
		lmsPath << "J:\\Datasets\\dataset_inmates_lms\\t000"
			    << i
				<< ".lms";

		try {
			// load the image
			ImgPrep::FaceImage imgInmate (imgPath.str() );

			std::vector<ImgPrep::FaceLandmark> lms;

			if (imgInmate.analyze (&lms) ) {
				// the image has been analyzed successfully, a
				// face has been detected, transform the land-
				// marks so they aren't influencted by tilt,
				// and size anymore.
				ImgPrep::Transform transform (lms);
				transform.normalize ();

				// write landmarks to file
				writeLandmarksToFile (lmsPath.str (), lms);

				numParsed++;
				std::cout << " [ Info ] >> Successfully parsed '" << i << ".jpg'" << std::endl;
			} else {
				numFailed++;
				std::cout << " [ Info ] >> No face found in '" << i << ".jpg'" << std::endl;
			}
		} catch (ImgPrep::NoImageException e) {
			numSkipped++;
		} catch (ImgPrep::StasmFailedException e) {
			numFailed++;
			std::cout << " [ Fail ] >> Stasm failed (#" << i << ")." << std::endl;
		} catch (ImgPrep::MalformedFaceException e) {
			numFailed++;
			std::cout << " [ Fail ] >> Malformed face (#" << i << ")." << std::endl;
		} catch (std::exception e) {
			numFailed++;
			std::cout << " [ Fail ] >> Error: " << e.what () << " (#" << i << ")." << std::endl;
		}
	}

	std::cout << " [ Info ] >> All entries have been processed. Results: " << std::endl
		      << "          >> Success    : " << numParsed << std::endl
			  << "          >> Failed     : " << numFailed << std::endl
			  << "          >> Skipped    : " << numSkipped << std::endl
			  << "          >> Time taken : " << ( (clock () - t) / CLOCKS_PER_SEC) << std::endl;

	std::cin.get ();
}