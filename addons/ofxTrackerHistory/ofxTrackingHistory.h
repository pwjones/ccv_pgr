
#ifndef OFXTRACKINGHISTORY_H
#define OFXTRACKINGHISTORY_H

#include "ofxOpenCv.h"
#include "ofxEdgeDetector.h"

using namespace cv;

class ofxTrackingHistory
{
public:
		ofxTrackingHistory(ofxEdgeDetector *detector, double thresh);
		~ofxTrackingHistory();
		void updatePosition(cv::Point posUpdate);
		double followingProportion(int path);
		void reset();
protected:
		ofxEdgeDetector *edgeDetector;
		vector<cv::Point> pos;
		vector<vector<bool>> followed;
		double followingThresh;
		int followingPath;
		bool useSkel;
};

#endif //OFXTRACKERHISTORY_H