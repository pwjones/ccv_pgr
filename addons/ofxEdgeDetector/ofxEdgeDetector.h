#ifndef OFXEDGEDETECTOR_H
#define OFXEDGEDETECTOR_H

#include "ofxOpenCv.h"
#include "ofxCvGrayscaleImage.h"
#include "CPUImageFilter.h"
//#include "ofxFC2MovieWriter.h"
//#include "opencv2/opencv.hpp"

using namespace cv;

//class CPUImageFilter();
class ofxEdgeDetector
{
public:

	ofxEdgeDetector();
	~ofxEdgeDetector();
	void updateImage(ofxCvGrayscaleImage& src_img);
	void detectEdges();
	void thinPaths();
	double minPathDist(cv::Point p);
	
	// These need to be public to be accessed by GUI routines. Don't use
	void cannyThreshold(int newThresh, void *);
	void keyResponder(int c);
	void selectContours(int x, int y);
	vector<int> selContours;

	vector<vector<cv::Point> > pathPts;
	vector<vector<cv::Point> > skelPathPts;

protected:
	// Internally, we handle all of the image processing in OpenCV 2.x style C++
	// function calls.  The Mat image stucture is much more convenient than the 
	// former style

	Mat bgImg, edgeImg, contourImg;
	Mat paths, seedPts;
	cv::Size imSize;
	int lowerThresh, ratio;
	string origWind, edgeWind, contourWind;
	vector<vector<cv::Point> > contours;
	vector<Vec4i> hierarchy;
	int activePath;
	vector<vector<int> > pathContours;
	

	
	void initWindows();
	void computeContours();
	void selectPaths();
	void findNearbyContours(int x, int y, vector<int>& neari);
	void refinePaths();
	Mat drawSelectedContours(vector<int>& seli); //draws contours in selContours, also returns the image
	Mat drawSelectedPaths(int pathi[], int ni); //draws the contours in pathContours, also returns the image
	Mat closePathImg(int pathi[], int ni);
	Mat& closePathImg(Mat& im);
	Mat drawPathOverlay(int pathi[], int ni);
	Mat drawPathOverlay(vector<vector<cv::Point> > pts);
	Mat drawContourPoints(const vector<cv::Point>& contourPts, int matrixType, Vec3b *newColor = 0, bool blend = 0);
	Mat drawContourPoints(const vector<cv::Point>& contourPts, Mat img, int matrixType, Vec3b *color = 0, bool blend = 0);

};

#endif // OFXEDGEDETECTOR_H