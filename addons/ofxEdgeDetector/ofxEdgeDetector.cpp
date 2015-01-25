
#include "ofxEdgeDetector.h"
#include "opencv2/opencv.hpp"
#include "thinning.h"

using namespace cv;

// Function Prototypes
static void mouseSelect( int event, int x, int y, int, void* );
static void mouseDoNothing( int event, int x, int y, int, void* );
void cannyThreshWrapper(int x, void *data);
void listPointsInImage(Mat& I, vector<cv::Point>& pts);
float euclidianDist(const cv::Point& p1, const cv::Point& p2);
void keyboardInput();

// GLOBALS
ofxEdgeDetector *edgeObj;

// ------------------------------ Class Functions --------------------------------
ofxEdgeDetector::ofxEdgeDetector() {
	// Default values for some variables
	lowerThresh = 25;
	ratio = 3;
	origWind = "Original Image";
	edgeWind = "Detected Edges";
	contourWind = "Contours";
	activePath = 1;
	pathPts.reserve(2); // make sure that there are at least 2 spots in vector.
	skelPathPts.reserve(2);
	detected = 0;
	skelDetected = 0;
	edgeObj = this;
}

ofxEdgeDetector::~ofxEdgeDetector()
{
}

bool ofxEdgeDetector::pathsDetected()
{
	return(detected);
}

// ----------------------------------------------------------
vector<int> ofxEdgeDetector::numPathPoints(bool useSkel)
{
	if (useSkel && skelDetected)
		vector<vector<cv::Point>>& pts = (skelPathPts.size() > 0) ? skelPathPts: pathPts;
	else 
		vector<vector<cv::Point>>& pts = pathPts;
	
	vector<int> npts(pathPts.size(),-1);
	for(int i = 0; i<npts.size(); i++) {
		npts[i] = pathPts[i].size();
	}
}
		
// ------------------------------------------------------------
void ofxEdgeDetector::updateImage(ofxCvGrayscaleImage& src_img)
{
	//Mat temp;
	IplImage *cvIm;
	cvIm = src_img.getCvImage();
	
	Mat newMat = cvarrToMat(cvIm);
	bgImg = newMat.clone();
	imSize = bgImg.size();
}
// ---------------------------------
void ofxEdgeDetector::detectEdges()
{	
	// Read in an image - useful for developing in the dark
    /*
	bgImg = imread("savedBg.jpg", CV_LOAD_IMAGE_GRAYSCALE); // Read the file
	if( ! bgImg.data ) {  // Check for invalid input
        cout <<  "Could not open or find the image" << std::endl ;
        return;
    }
	*/

	initWindows(); // open up the display windows
	bgImg.copyTo(edgeImg); // Make a copy of bgImg
	imshow(origWind, bgImg); // Show the original image
	cannyThreshold(lowerThresh, 0); //detect Edges, display

	keyboardInput(); // get user interaction

	destroyWindow(origWind);
	destroyWindow(edgeWind);
	destroyWindow(contourWind);
 }

double ofxEdgeDetector::minPathDist(cv::Point p, int pathNum)
{
	cv::Size s = bgImg.size();
	// use the skeleton path points if they are there, the full path if not
	vector<vector<cv::Point> >& pts = (skelPathPts.size() > 0) ? skelPathPts : pathPts;
		
	double minDist = euclidianDist(cv::Point(0,0), cv::Point(s.width, s.height)); 
	if (pts.size() > 0) {
		if (pathNum+1 > pts.size()) //check to not overindex the paths
			pathNum = 0;

		int len = pts[pathNum].size();
		for (int i=0; i< len; i++ ) { //check each point in the 1st path
			double dist = euclidianDist(p, pts[0][i]);
			if (dist < minDist)
				minDist = dist;
		}
	}
	return(minDist);
}

vector<double> ofxEdgeDetector::pathDist(cv::Point p, int pathNum, bool useSkel)
{
	cv::Size s = bgImg.size();
	vector<cv::Point> *pts;
	// use the skeleton path points if they are there, the full path if not
	if (useSkel) {
		pts = (skelPathPts.size() > 0) ? &skelPathPts[pathNum]: &pathPts[pathNum];
	} else {
		pts = &pathPts[pathNum];
	}
		
	vector<double> dist(pts->size(), -1); 
	for (int i=0; i< pts->size(); i++ ) { //check each point in the 1st path
		dist[i] = euclidianDist(p, (*pts)[i]);
	}
	return(dist);
}


void ofxEdgeDetector::initWindows()
{
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

	createTrackbar( "Min Threshold:", edgeWind, &lowerThresh, 100, cannyThreshWrapper );
}
// ----------------------------------------------------------
void ofxEdgeDetector::cannyThreshold(int newThresh, void *)
{
	lowerThresh = newThresh;
	blur(bgImg, edgeImg, cv::Size(3,3) );
	Canny(edgeImg, edgeImg, lowerThresh, lowerThresh*ratio, 3 ); // Perform canny edge detection
	imshow(edgeWind, edgeImg);
	computeContours();
}
// ----------------------------------
void ofxEdgeDetector::selectPaths()
{
	setMouseCallback(origWind, mouseSelect, 0 );
	setMouseCallback(contourWind, mouseSelect, 0 );
	
	pathContours.clear(); //reset the selected contours
	pathContours.reserve(2); // We need room for this many

	string prompt = "Click to select rewarded path, press enter when path is complete";
	//string text = "Funny text inside the box";
	int fontFace = FONT_HERSHEY_SIMPLEX;
	double fontScale = 1;
	int thickness = 3;
	int baseline=0;
	cv::Size textSize = getTextSize(prompt, fontFace, fontScale, thickness, &baseline);
	baseline += thickness;
	Mat txtImg = bgImg.clone();
	cv::Point textOrg((txtImg.cols - textSize.width)/2,(txtImg.rows + textSize.height - 50));
	putText(txtImg, prompt, textOrg, fontFace, fontScale, Scalar(255, 0, 0), thickness, 8);
	imshow(origWind, txtImg);

	for ( int i=0; i<2 ; i++ ) { // select the rewarded and distrator paths 
		cout << "Detecting path number " << i << "\n"; 
		activePath = i;
		selContours.clear();
		char c = waitKey(0);
		pathContours.push_back(selContours);
		vector<cv::Point> pv;
		pv.push_back(cv::Point(0,0));
		pathPts.push_back(pv);
	}
	int sp[] = {0, 1};
	//pathPts.clear();
	vector<cv::Point> tempPts;
	for ( int i = 0; i < 2; i++ ) { // going to fill the paths by closing
		//Mat pathIm = drawSelectedPaths(sp[i], 1);
		Mat pathIm = closePathImg(&sp[i], 1);
		//imshow(contourWind, pathIm);
		//waitKey(0);
		//pathPts[i].clear();
		//cout << "Before listPnts:" << pathPts[i].size() << "\n";
		tempPts.clear();
		listPointsInImage(pathIm, tempPts);
		pathPts[i] = tempPts;
		//listPointsInImage(pathIm, pathPts[i]);
		//cout << "After listPnts:" << pathPts[i].size() << "\n"; 
		//pathIm = drawPathOverlay(&sp[i],1);
	}

	drawPathOverlay(pathPts);
	setMouseCallback(origWind, mouseDoNothing, 0 );
	setMouseCallback(contourWind, mouseDoNothing, 0 );
	detected = 1;
}

void ofxEdgeDetector::thinPaths()
{
	Mat pIm, skelIm;
	vector<cv::Point> pts;

	skelPathPts.clear();
	for (int i = 0; i<pathPts.size(); i++) {
		pIm = drawContourPoints(pathPts[i], CV_8UC1);
		thinning(pIm, skelIm);
		
		listPointsInImage(skelIm, pts);
		skelPathPts.push_back(pts);
		
		//namedWindow( "skeleton", WINDOW_NORMAL ); 
		//resizeWindow( "skeleton", skelIm.size().width/3, skelIm.size().height/3);
		//imshow("skeleton", skelIm);
		//imshow(contourWind, pIm);
		//waitKey(0);
		pts.clear();
	}
	pIm = drawPathOverlay(pathPts);
	skelIm = drawContourPoints(skelPathPts[0], CV_8UC3);
	skelIm = drawContourPoints(skelPathPts[1], skelIm, CV_8UC3);
	Mat combo = skelIm.clone();
	addWeighted(pIm, .5, skelIm, .5, 0, combo);
	//namedWindow( "skeleton", WINDOW_NORMAL ); 
	//resizeWindow( "skeleton", skelIm.size().width/3, skelIm.size().height/3);
	imshow(contourWind, combo);
	// checking the path dist
	cout << "Distance from 0: " << minPathDist(cv::Point(0,0), 0) << "\n";
	cout << "Distance from (1280,1024): " << minPathDist(cv::Point(1280,1024), 0) << "\n";
	skelDetected = 1;
}

// -----------------------------------
void ofxEdgeDetector::refinePaths()
{
	setMouseCallback(origWind, mouseSelect, 0 );
}
// -----------------------------------------------------
Mat ofxEdgeDetector::closePathImg(int pathi[], int ni)
{
	Mat pathIm = drawSelectedPaths(pathi, ni);
	pathIm = closePathImg(pathIm);
	return (pathIm);
}
// ------------------------------------------
Mat& ofxEdgeDetector::closePathImg(Mat& im)
{
	int DIAM_X = 4, DIAM_Y=4;
	Mat element = getStructuringElement(MORPH_RECT, cv::Size(DIAM_X, DIAM_Y));
	morphologyEx(im, im, MORPH_CLOSE, element, cv::Point(-1,-1), 1);
	return(im);
}
// --------------------------------------
void ofxEdgeDetector::computeContours()
{
	RNG rng(12345);
	vector<vector<cv::Point> > contourTemp;
	vector<Vec4i> hier;

	Mat edgeTemp = edgeImg.clone();
	findContours( edgeTemp, contourTemp, hier, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);
	
	// Drawing the contours actually fills in the gaps really well - leverage that
	Mat contourImg = Mat::zeros( edgeImg.size(), CV_8UC1 ); //color version
	uchar color = 255;
	for( int i = 0; i< contourTemp.size(); i++ ) {
	   drawContours( contourImg, contourTemp, i, color, 2, 8, hier, 0, cv::Point() );
	}
	// Since the contours returned don't really give a dense enough output, we go through
	// the image and pick out the individual drawn points as the contour
	vector<cv::Point> contourPts;
	listPointsInImage( contourImg, contourPts );
	Mat contourImg2 = drawContourPoints(contourPts, CV_8UC1);
	
	/// Find contours, again - this time for the grouping.
	edgeTemp = contourImg2.clone();
	findContours( edgeTemp, contours, hierarchy, CV_RETR_LIST, CV_CHAIN_APPROX_NONE, cv::Point(0, 0) );
	
	/// Draw the new contours
	Mat drawing = Mat::zeros( edgeImg.size(), CV_8UC3 );
	for( int i = 0; i< contours.size(); i++ ) {
       Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
	   drawContours( drawing, contours, i, color, 2, 8, hierarchy, 0, cv::Point() );
	}
	imshow(contourWind, drawing);	
}

Mat ofxEdgeDetector::drawContourPoints(const vector<cv::Point>& contourPts, int matrixType, Vec3b *newColor, bool blend)
{
	Mat img;
	if (matrixType == CV_8UC1) {
		img = Mat::zeros( edgeImg.size(), CV_8UC1 );
	} else {
		img = Mat::zeros( edgeImg.size(), CV_8UC3 );
	}
	return ( drawContourPoints(contourPts, img, matrixType, newColor, blend) );
}

Mat ofxEdgeDetector::drawContourPoints(const vector<cv::Point>& contourPts, Mat img, int matrixType, Vec3b *newColor, bool blend)
{
	Vec3b color; 
	if (newColor == 0) {	
		color[0] = 255; color[1] = 255; color[2] = 255;
	} else
		color = *newColor;
	//cout << "drawContourPoints: newColor:" << (int)color[0] << "  " << (int)color[1] << "  " << (int)color[2] << "\n";
	//cout << "blend = " << blend << "\n"; 
	if (matrixType == CV_8UC1) {
		//Mat img = Mat::zeros( edgeImg.size(), CV_8UC1 );
		for( int i = 0; i< contourPts.size(); i++ ) { //now draw it point by point
			img.at<uchar>(contourPts[i]) = 255;
		}
		return (img);
	} else if (matrixType == CV_8UC3) { 
		//Mat img = Mat::zeros( edgeImg.size(), CV_8UC3 );
		cv::Size s = img.size();
		for( int i= 0; i< contourPts.size(); i++ ) { //for each path
			if (contourPts[i].x < s.width &&  contourPts[i].y < s.height) {
				Vec3b val = img.at<Vec3b>(contourPts[i]);
				if (blend)
					img.at<Vec3b>(contourPts[i]) = (val + color)/2;
				else
					img.at<Vec3b>(contourPts[i]) = color;
			} else {
				//cout << "Point (" << contourPts[i].x << ", " << contourPts[i].y << ") out of bounds\n";
			}
			//cout << "Point: (" << contourPts[i].x << ", " << contourPts[i].y << ")\n";
			//cout << "Color: " << 
			//cout << "Value: " << (int)newVal[0] << "  " << (int)newVal[1] << "  " << (int)newVal[2] << "\n";
			//waitKey(0);
		}
		return (img);
	} else {
		Mat img = Mat::zeros( edgeImg.size(), CV_8UC3 );
		return (img);
	}	
}


// keyResponder
// ---------------
// For processing the keystrokes over the contour window.
void ofxEdgeDetector::keyResponder(int c)
{
        while( c != 27 && c != 113) { //while the user does not hit 'Esc' or 'q'
			switch( (char)c ) {
			case 't':
				thinPaths();
				break;
			case 's':
				selectPaths();
				break; 
			}
			cout << "Hit 'r' to refine paths, 's' to reThreshold or select, and 'Esc' or 'q' to exit\n";	
			c = waitKey(0);
		}
}
// ---------------------------------------------------------------------------
Mat ofxEdgeDetector::drawSelectedContours(vector<int>& seli)
{
	RNG rng(12345);
	Mat drawing = Mat::zeros( edgeImg.size(), CV_8UC3 );
	for( int i = 0; i< seli.size(); i++ ) {
       Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
	   drawContours( drawing, contours, seli[i], color, 2, 8, hierarchy, 0, cv::Point() );
	   //cout << "Drawing contour " << seli[i] << "\n";
	}
	imshow(contourWind, drawing);
	return(drawing);
}
// ---------------------------------------------------------------------------
Mat ofxEdgeDetector::drawSelectedPaths(int pathi[], int ni)
{
	int ncolor = 3;
	uchar colors[] = {1, 255, 1, 1, 1, 255, 255, 255, 255};
	cv::Size s = edgeImg.size();
	Mat drawing = Mat::zeros( s, CV_8UC3 );
	for( int i = 0; i< ni; i++ ) {
		int pi = pathi[i];
		int ci = (pi % ncolor) * 3;
		Scalar color = Scalar( colors[ci], colors[ci+1], colors[ci+2] );
		//cout << "drawSelectedPaths: loop i = " << pi << " ci = " << ci << "\n";
		//cout << "drawSelectedPaths: contours is " << contours.size() << " elements long\n";
		for ( int j=0; j < pathContours[pi].size(); j++) {
			//cout << "In drawSelectedPaths drawing contour " << pathContours[pi][j] << "\n";
			drawContours( drawing, contours, pathContours[pi][j], color, 2, 8, hierarchy, 0, cv::Point() );
		}
	}
	namedWindow( contourWind, WINDOW_NORMAL ); // in case the window doesn't exist already
	resizeWindow(contourWind, s.width/2, s.height/2);
	imshow(contourWind, drawing);
	return (drawing);
}

// ---------------------------------------------------------------------------
Mat ofxEdgeDetector::drawPathOverlay(int pathi[], int ni)
{
	Vec3b color;
	int ncolor = 3;
	uchar colors[] = {1, 128, 1, 1, 1, 128, 255, 255, 255};
	cv::Size s = bgImg.size();
	Mat drawing = Mat::zeros( s, CV_8UC3 );
	cvtColor(bgImg, drawing, CV_GRAY2RGB,0);  //writing the background image
	for( int i= 0; i< ni; i++ ) { //for each path
		int pi = pathi[i];
		int ci = (pi % ncolor) * 3;
		color[0] = colors[ci]; color[1] = colors[ci+1]; color[2] = colors[ci+2]; //set the tint color
		for ( int j=0; j < pathContours[pi].size(); j++) { // for each contour in the path
			int cont_i = pathContours[pi][j];
			for ( int k=0; k < contours[cont_i].size(); k++) { //for each point in the contour
				cv::Point2i pnt = contours[cont_i][k];
				Vec3b val = drawing.at<Vec3b>(pnt);
				drawing.at<Vec3b>( contours[cont_i][k] ) = (val + color)/2;
			}
		}
	}

	namedWindow( "overlay", WINDOW_NORMAL ); // in case the window doesn't exist already
	resizeWindow("overlay", s.width/2, s.height/2);
	imshow("overlay", drawing);
	return (drawing);
}

// ---------------------------------------------------------------------------
Mat ofxEdgeDetector::drawPathOverlay(vector<vector<cv::Point> > pts)
{
	Vec3b color;
	int ncolor = 3;
	uchar colors[] = {1, 128, 1, 1, 1, 128, 255, 255, 255};
	cv::Size s = bgImg.size();
	Mat drawing = Mat::zeros( s, CV_8UC3 );
	cvtColor(bgImg, drawing, CV_GRAY2RGB);  //writing the background image
	for( int i= 0; i< pts.size(); i++ ) { //for each path
		int ci = (i % ncolor) * 3;
		color[0] = colors[ci]; color[1] = colors[ci+1]; color[2] = colors[ci+2]; //set the tint color
		drawing = drawContourPoints(pts[i], drawing, CV_8UC3, &color, 1);
		imshow(contourWind, drawing);
	}
	return (drawing);
}

// ---------------------------------------------------------------------------
static void mouseSelect( int event, int x, int y, int, void* )
{
	if( event != EVENT_LBUTTONDOWN )
        return;
	edgeObj->selectContours(x, y);
}

// ---------------------------------------------------------------------------
static void mouseDoNothing( int event, int x, int y, int, void* )
{
}

// ---------------------------------------------------------------------------
void ofxEdgeDetector::selectContours(int x, int y) 
{
	findNearbyContours(x, y, selContours);
	drawSelectedContours(selContours);
}

// ---------------------------------------------------------------------------
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

// --------------------------------
string ofxEdgeDetector::print()
{
	std::stringstream pathstr;
	char ptstr[50];

	//Print the formatted points for the paths, skeletonized and full
	pathstr << "Paths:" << pathPts.size() << "\n";
	for (int i=0; i< pathPts.size(); i++) {
		pathstr << "Path " << i << ":";
		for ( int j=0; j < pathPts[i].size(); j++ ) {
			sprintf(ptstr, "[%d,%d]", pathPts[i][j].x, pathPts[i][j].y);
			pathstr << ptstr;
		}
		pathstr << "\n";
	}

	//Print the formatted points for the paths, skeletonized
	pathstr << "skeletonPaths:" << skelPathPts.size() << "\n";
	for (int i=0; i< skelPathPts.size(); i++) {
		pathstr << "Path " << i << ":";
		for ( int j=0; j < skelPathPts[i].size(); j++ ) {
			sprintf(ptstr, "[%d,%d]", skelPathPts[i][j].x, skelPathPts[i][j].y);
			pathstr << ptstr;
		}
		pathstr << "\n";
	}
	return(pathstr.str());
}

// ------------------------  Local Functions   -----------------                 

// listPointsInImage: returns the set of points that aren't black in the first channel
// -----------------------------------------------------------------------------------
static void listPointsInImage(Mat& I, vector<cv::Point>& pts)
{  
	// accept only char type matrices
    CV_Assert(I.depth() != sizeof(uchar));
	//CV_Assert(I.channels() == 1);
    int channels = I.channels();
    int nRows = I.rows;
    int nCols = I.cols * channels;

	int width = I.cols;
	int height = I.rows;
	bool cont;
    if ( cont = I.isContinuous()) {
        nCols *= nRows;
        nRows = 1;
    }

    int i,j;
    uchar* p;
    for( i = 0; i < nRows; ++i) {
        p = I.ptr<uchar>(i); //points to the beginning of each row
        for ( j = 0; j < nCols; ++j) { //work across the columns
			if (p[j] > 0) {
				if (cont) { //realized that for continuous data, need to extract point from single index
					pts.push_back(cv::Point((j/channels)%width,(j/channels)/width));
					//cout << "Detected point: x " << j%width << "  y  " << j/width << "\n";
				} else {
					pts.push_back(cv::Point(j/channels,i));
					//cout << "Detected point: x " << j << "  y  " << i << "\n";
				}
			}
		}
    }
}

float euclidianDist(const cv::Point& p1, const cv::Point& p2)
{
	cv::Point diff = p1 - p2;
	float dist = std::sqrt((float)(diff.x*diff.x + diff.y*diff.y));
    return (dist);
}

// Necessary to pass callback of ofcEdgeDetector::cannyThreshold
void cannyThreshWrapper(int x, void *data)
{
	edgeObj->cannyThreshold(x,data);
}

void keyboardInput()
{
	cout << "Hit 's' select paths, 't' to thin them, or 'Esc' or 'q' to exit\n";
	int c = waitKey(0);
	edgeObj->keyResponder(c);
}
