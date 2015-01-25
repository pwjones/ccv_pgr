
#ifndef OFXTRACKERHISTORY_H
#define OFXTRACKERHISTORY_H

#include "ofxOpenCv.h"
#include "ofxEdgeDetector.h"

using namespace cv;

class ofxTrackerHistory
{
public:
		ofxTrackerHistory(ofxEdgeDetector *detector, double thresh);
		~ofxTrackerHistory();
		void updatePosition(cv::Point posUpdate);
		double followingProportion(int path);
		
protected:
		ofxEdgeDetector *edgeDetector;
		vector<cv::Point> pos;
		vector<vector<bool>> followed;
		double followingThresh;
		int followingPath;
		bool useSkel;
};

#endif //OFXTRACKERHISTORY_H