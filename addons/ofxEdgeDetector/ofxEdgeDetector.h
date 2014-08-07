#ifndef OFXEDGEDETECTOR_H
#define OFXEDGEDETECTOR_H

#include "ofxOpenCv.h"
#include "ofxFC2MovieWriter.h"
#include "opencv2/opencv.hpp"

using namespace cv;

class ofxEdgeDetector
{
public:
	ofxEdgeDetector();
	~ofxEdgeDetector();

	void detectEdges();
	
protected:
	ofxCvColorImage bgImage;
	Mat paths;
	Mat seedPts;

};

#endif // OFXEDGEDETECTOR_H