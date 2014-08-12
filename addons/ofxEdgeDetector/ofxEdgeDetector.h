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
	void cannyThreshold(int newThresh, void *);
	void drawSelectedContours(vector<int>& seli);
	void findNearbyContours(int x, int y, vector<int>& neari);
	void keyResponder(int c);
	vector<int> selContours;

protected:
	//ofxCvGrayscaleImage bgImg;
	Mat bgImg, edgeImg, contourImg;
	Mat paths, seedPts;
	cv::Size imSize;
	int lowerThresh, ratio;
	string origWind, edgeWind, contourWind;
	vector<vector<cv::Point> > contours;
	vector<Vec4i> hierarchy;
	
	
	void computeContours();
	void selectPaths();
	void refinePaths();

	

};

#endif // OFXEDGEDETECTOR_H