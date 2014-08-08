
#include "ofxEdgeDetector.h"
#include "opencv2/opencv.hpp"

using namespace cv;

bool testFunc();


ofxEdgeDetector::ofxEdgeDetector() {
	printf("Hi, what's up? I'm edge detector\n");
}

ofxEdgeDetector::~ofxEdgeDetector()
{
}

void ofxEdgeDetector::updateImage(ofxCvGrayscaleImage& src_img)
{
	//Mat temp;
	IplImage *cvIm;
	cvIm = src_img.getCvImage();
	
	Mat newMat = cvarrToMat(cvIm);
	bgImg = newMat.clone();
	//namedWindow( "Update Window", WINDOW_AUTOSIZE ); // Create a window for display.
    //imshow( "Update Window", bgImg );                // Show our image inside it.
	//cvCopy( cvIm , &newMat, 0 );
	//bgImg = newMat;
	//bgImg = &temp; 
}

void ofxEdgeDetector::detectEdges()
{
	//IplImage im = bgImg;
	//IplImage *img = &im;
	//= bgImg.getCvImage();
	string origWind = "Original Image";
	string edgeWind = "Detected Edges";
	//Mat image = bgImg;
	IplImage im = bgImg;
	IplImage *img = &im;
    //image = imread("savedBg.jpg", CV_LOAD_IMAGE_GRAYSCALE); // Read the file

    if(! bgImg.data )                      // Check for invalid input
    {
        cout <<  "Could not open or find the image" << std::endl ;
        return;
    }

	//assert( img->width%2 == 0 && img->height%2 == 0);
	cv::Size s = bgImg.size();
	namedWindow( origWind, WINDOW_NORMAL ); // Create our windows
	resizeWindow(origWind, s.width/2, s.height/2);
	namedWindow( edgeWind, WINDOW_NORMAL );
	resizeWindow(edgeWind, s.width/2, s.height/2);
	imshow(origWind, bgImg); // Show the original image

	// Create an image for the output
	bgImg.copyTo(edgeImg);
	//IplImage* out = cvCreateImage( cvSize(img->width/2,img->height/2), img->depth, img->nChannels );
	
	// Perform canny edge detection
	Canny( edgeImg, edgeImg, 10, 100, 3 );

	// Show the processed image
	imshow(edgeWind, edgeImg);
	waitKey(0); //wait for keyboard input over window, forever
	destroyWindow(origWind);
	destroyWindow(edgeWind);
 
 }

bool testFunc()
{
	return(true);
}