
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

void ofxEdgeDetector::detectEdges()
{
	IplImage* img = cvLoadImage( "my3.jpg", 0);
	cvNamedWindow( "example-input" );
	cvNamedWindow( "example-output" );

	// Show the original image
	cvShowImage("example-input", img);

	// Make sure image is divisible by 2
	assert( img->width%2 == 0 && img->height%2 == 0);

	// Create an image for the output
	IplImage* out = cvCreateImage( cvSize(img->width/2,img->height/2), img->depth, img->nChannels );

	// Reduce the image by 2
	cvPyrDown( img, out );

	// Perform canny edge detection
	cvCanny( out, out, 10, 100, 3 );

	// Show the processed image
	cvShowImage("example-output", out);

	cvWaitKey(0);
	cvReleaseImage( &img );
	cvReleaseImage( &out );
	cvDestroyWindow( "example-input" );
	cvDestroyWindow( "example-output" );
 
 }

bool testFunc()
{
	return(true);
}