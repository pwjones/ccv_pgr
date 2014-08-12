
#include "ofxEdgeDetector.h"
#include "opencv2/opencv.hpp"

using namespace cv;

bool testFunc();
static void onMouseSelect( int event, int x, int y, int, void* );
void cannyThreshWrapper();
void listPointsInImg(Mat& I, vector<cv::Point>& pts);
float euclidianDist(const cv::Point& p1, const cv::Point& p2);
void keyboardInput();

ofxEdgeDetector *edgeObj;

// Necessary to pass callback of ofcEdgeDetector::cannyThreshold
void cannyThreshWrapper(int x, void *data)
{
	edgeObj->cannyThreshold(x,data);
}
//Constructor!
ofxEdgeDetector::ofxEdgeDetector() {
	// Default values for some variables
	lowerThresh = 25;
	ratio = 3;
	origWind = "Original Image";
	edgeWind = "Detected Edges";
	contourWind = "Contours";
	edgeObj = this;
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
	imSize = bgImg.size();
}

void ofxEdgeDetector::detectEdges()
{
    bgImg = imread("savedBg.jpg", CV_LOAD_IMAGE_GRAYSCALE); // Read the file
	if( ! bgImg.data ) {  // Check for invalid input
        cout <<  "Could not open or find the image" << std::endl ;
        return;
    }

	// Create our windows
	cv::Size s = bgImg.size();
	int offx = 10, offy = 20;
	namedWindow( origWind, WINDOW_NORMAL ); 
	resizeWindow(origWind, s.width/3, s.height/3);
	moveWindow(origWind, offx, offy);
	namedWindow( edgeWind, WINDOW_NORMAL );
	resizeWindow(edgeWind, s.width/3, s.height/3);
	moveWindow(edgeWind, offx+s.width/3, offy);
	namedWindow( contourWind, WINDOW_NORMAL ); 
	resizeWindow(contourWind, s.width/2, s.height/2);
	moveWindow(contourWind, offx+(s.width/3*2), offy);
	
	edgeObj = this;
	createTrackbar( "Min Threshold:", edgeWind, &lowerThresh, 100, cannyThreshWrapper );
	bgImg.copyTo(edgeImg); // Make a copy of bgImg
	imshow(origWind, bgImg); // Show the original image
	

	cannyThreshold(lowerThresh, 0); //detect Edges, display

	/// Show in a window
	// get some mouse clicks
	// get an enter key
	// get some more mouse clicks
	keyboardInput();

	//waitKey(0); //wait for keyboard input over window, forever
	destroyWindow(origWind);
	destroyWindow(edgeWind);
	destroyWindow(contourWind);
 }

void ofxEdgeDetector::cannyThreshold(int newThresh, void *)
{
	lowerThresh = newThresh;
	blur(bgImg, edgeImg, cv::Size(3,3) );
	Canny(edgeImg, edgeImg, lowerThresh, lowerThresh*ratio, 3 ); // Perform canny edge detection
	imshow(edgeWind, edgeImg);
	computeContours();
}

void ofxEdgeDetector::selectPaths()
{
	setMouseCallback(origWind, onMouseSelect, 0 );
	setMouseCallback(contourWind, onMouseSelect, 0 );
	
	string prompt = "Click to select rewarded path, press enter when path is complete";
	string text = "Funny text inside the box";
	int fontFace = FONT_HERSHEY_SCRIPT_SIMPLEX;
	double fontScale = 2;
	int thickness = 3;
	int baseline=0;
	cv::Size textSize = getTextSize(text, fontFace, fontScale, thickness, &baseline);
	baseline += thickness;
	Mat txtImg = bgImg.clone();
	//Mat img(600, 800, CV_8UC3, Scalar::all(0));
	// center the text
	cv::Point textOrg((txtImg.cols - textSize.width)/2,(txtImg.rows + textSize.height)/2);
	// draw the box
	//rectangle(txtImg, textOrg + Point(0, baseline), textOrg + Point(textSize.width, -textSize.height),
	//		  Scalar(0,0,255));
	// ... and the baseline first
	//line(txtImg, textOrg + Point(0, thickness), textOrg + Point(textSize.width, thickness),
	//	 Scalar(0, 0, 255));
	// then put the text itself
	putText(txtImg, text, textOrg, fontFace, fontScale, Scalar::all(255), thickness, 8);

}
void ofxEdgeDetector::refinePaths()
{
	setMouseCallback(origWind, onMouseSelect, 0 );
}

void ofxEdgeDetector::computeContours()
{
	RNG rng(12345);
	vector<vector<cv::Point> > contourTemp;
	vector<Vec4i> hier;

	Mat edgeTemp = edgeImg.clone();
	
	imshow(contourWind, edgeTemp);

	findContours( edgeTemp, contourTemp, hier, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);
	
	// Drawing the contours actually fills in the gaps really well - leverage that
	Mat contourImg = Mat::zeros( edgeImg.size(), CV_8UC1 ); //color version
	Scalar color(255, 255, 255);
	for( int i = 0; i< contourTemp.size(); i++ ) {
       //Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
	   drawContours( contourImg, contourTemp, i, color, 2, 8, hier, 0, cv::Point() );
	}
	imshow(contourWind, contourImg);
	// Doing findContours a second time doesn't work to get the drawn contours.  So, instead
	// going through the image point by point and adding to a vector 
	vector<cv::Point> contourPts;
	listPointsInImg( contourImg, contourPts );
	Mat contourImg2 = Mat::zeros( edgeImg.size(), CV_8UC1 ); //color version
	for( int i = 0; i< contourPts.size(); i++ ) {
	   contourImg2.at<uchar>(contourPts[i]) = 255;
	}
	// Close the contours - morphological closing operation
	cv::Size s = bgImg.size();
	int DIAM_X = 4, DIAM_Y=4;
	Mat closed;
	Mat element = getStructuringElement(MORPH_RECT, cv::Size(DIAM_X, DIAM_Y));
	morphologyEx(contourImg2, closed, MORPH_CLOSE, element, cv::Point(-1,-1), 1);
	namedWindow("closed", WINDOW_NORMAL);
	resizeWindow("closed", s.width/2, s.height/2);
	imshow("closed", closed);
	imshow(contourWind, contourImg);
	setMouseCallback(contourWind, onMouseSelect, 0 );

	/// Find contours, again
	edgeTemp = contourImg2.clone();
	findContours( edgeTemp, contours, hierarchy, CV_RETR_LIST, CV_CHAIN_APPROX_NONE, cv::Point(0, 0) );
	/// Draw the new contours
	Mat drawing = Mat::zeros( edgeImg.size(), CV_8UC3 );
	for( int i = 0; i< contours.size(); i++ ) {
       Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
	   drawContours( drawing, contours, i, color, 2, 8, hierarchy, 0, cv::Point() );
	}
	imshow(contourWind, drawing);
	cout << "Exiting Path Detection\n";
}

void keyboardInput()
{
	int c = waitKey(0);
	edgeObj->keyResponder(c);
}

// keyResponder
// ---------------
// For processing the keystrokes over the contour window.
void ofxEdgeDetector::keyResponder(int c)
{
        while( c != 27 && c != 113) { //while the user does not hit 'Esc' or 'q'
            cout << "Hit 'r' to refine paths, 's' to reThreshold or select, and 'Esc' or 'q' to exit\n";
			switch( (char)c ) {
			case 'r':
				refinePaths();
				break;
			case 's':
				cannyThreshold(lowerThresh, 0);
				break; 
			}
			c = waitKey(0);
		}
}

void ofxEdgeDetector::drawSelectedContours(vector<int>& seli)
{
	RNG rng(12345);
	Mat drawing = Mat::zeros( edgeImg.size(), CV_8UC3 );
	for( int i = 0; i< seli.size(); i++ ) {
       Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
	   drawContours( drawing, contours, seli[i], color, 2, 8, hierarchy, 0, cv::Point() );
	}
	//namedWindow(contourWind, WINDOW_NORMAL);
	imshow(contourWind, drawing);
}

static void onMouseSelect( int event, int x, int y, int, void* )
{
	if( event != EVENT_LBUTTONDOWN )
        return;

	cv::Point selP = cv::Point(x,y);
	edgeObj->findNearbyContours(x, y, edgeObj->selContours);
	edgeObj->drawSelectedContours(edgeObj->selContours);
}

bool testFunc()
{
	return(true);
}

void ofxEdgeDetector::findNearbyContours(int x, int y, vector<int>& neari)
{
	std::vector<float> dists;
	float distThresh = 10;
	float dist;

	for (int i = 0; i < contours.size(); i++ ) {
		for (int j = 0; j < contours[i].size(); j++) {
			cv::Point2i contPt = contours[i][j];
			cv::Point2i clickPt = cv::Point(x,y);
			dist = euclidianDist(contPt, clickPt);
			if (dist < distThresh) {
				neari.push_back(i);
				dists.push_back(dist);
			}
		}
	}
	//return(neari);
}

static void listPointsInImg(Mat& I, vector<cv::Point>& pts)
{  
	// accept only char type matrices
    CV_Assert(I.depth() != sizeof(uchar));

    int channels = I.channels();
    int nRows = I.rows;
    int nCols = I.cols * channels;

    if (I.isContinuous()) {
        nCols *= nRows;
        nRows = 1;
    }

    int i,j;
    uchar* p;
    for( i = 0; i < nRows; ++i) {
        p = I.ptr<uchar>(i); //points to the beginning of each row
        for ( j = 0; j < nCols; ++j) { //work across the columns
			if (p[j] > 0)
				pts.push_back(cv::Point(j,i));
        }
    }
}



float euclidianDist(const cv::Point& p1, const cv::Point& p2)
{
	cv::Point diff = p1 - p2;
	float dist = std::sqrt((float)(diff.x*diff.x + diff.y*diff.y));
    return (dist);
}