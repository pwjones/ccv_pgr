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
	
protected:
	//ofxCvGrayscaleImage bgImg;
	Mat bgImg, edgeImg;
	Mat paths;
	Mat seedPts;

};

#endif // OFXEDGEDETECTOR_H