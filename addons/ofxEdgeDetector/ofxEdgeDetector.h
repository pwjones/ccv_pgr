#ifndef OFXEDGEDETECTOR_H
#define OFXEDGEDETECTOR_H

#include "ofxOpenCv.h"
#include "ofxFC2MovieWriter.h"

class ofxEdgeDetector
{
public:
	ofxEdgeDetector();
	~ofxEdgeDetector();

	void detectEdges();
};

#endif // OFXEDGEDETECTOR_H