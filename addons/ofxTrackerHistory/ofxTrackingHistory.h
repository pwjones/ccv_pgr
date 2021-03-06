
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
		void updatePosition(cv::Point posUpdate); // to be called at each position update
		bool isFollowingPath(int path);
		double followingProportion(int path); // the current proportion of trail followed
		double continuousFollowingProp(int path); // the proportion of the trail covered in current following episode
		void reset(); 
		
protected:
		ofxEdgeDetector *edgeDetector; //the object responsible for keeping track of paths
		vector<cv::Point> pos; // tracked position
		vector<vector<bool>> followed; // booleans for the following of each trail point
		vector<bool> close; // the rewarded path, boolean vector for closeness
		vector<bool> prevClose; // closeness on the prev update
		vector<bool> diffV; // difference between close and prevClose
		double followingThresh; // distance threshold for following
		int followingPath; // the path on which we track following
		bool useSkel; // are the paths skeletonized
		bool isFollowing; // current following state of the animal
		double continuousProp, initialProp;
		
		void updateFollowing(double newProp, bool currFollowing);
		double findAdjacentTrailPoints(cv::Point pos, vector<bool>& closeTrail);
		double vectorDiffProp(vector<bool> v1, vector<bool> v2);
};

#endif //OFXTRACKERHISTORY_H