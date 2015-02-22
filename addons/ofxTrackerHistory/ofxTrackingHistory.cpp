/* ofxTrackingHistory.cpp
 * --------------------------
 * Written by Peter Jones, 2015.  
 *
 * Currently has a huge limitation - I've take the shortcut of permanently setting the FOLLOWINGPATH = 0.
 * Other functions and structures rely on this.  Not a huge deal.  Means that it will only track the history
 * relative to a single path, and when making changes to this, must consider that some things will need to 
 * become vectors to accomodate changes.
 */

#include "ofxTrackingHistory.h"
#include "opencv2/opencv.hpp"
#include <algorithm>

// Function prototypes
vector<bool> vectorOR(vector<bool>& v1, vector<bool>& v2);
double trueCount(vector<bool>& v);

ofxTrackingHistory::ofxTrackingHistory(ofxEdgeDetector *detector, double thresh)
{
	edgeDetector = detector;
	followingThresh = thresh;
	useSkel = 1;
	followingPath = 0;
	// All of the paths have different sizes, so initialize the followed vector to match that.
	vector<int> numPathPts = detector->numPathPoints(useSkel);
	followed = vector<vector<bool>>(numPathPts.size());
	for (int i=0; i < numPathPts.size(); i++) {  
		followed[i] = vector<bool>(numPathPts[i], false);
	}
}
//-----------------------------------------------------------------
ofxTrackingHistory::~ofxTrackingHistory()
{
}

//-----------------------------------------------------------------
bool ofxTrackingHistory::isFollowingPath(int path)
{
    if (path != followingPath) 
        return (false);
    else
        return (isFollowing);
}

//-----------------------------------------------------------------
double ofxTrackingHistory::continuousFollowingProp(int path)
{
    if (path != followingPath) 
        return (0.0);
    else
        return (continuousProp);
}

//-----------------------------------------------------------------
void ofxTrackingHistory::updatePosition(cv::Point posUpdate)
{
	//for(jj=0; jj<edgeDetector->numPaths;i++) {
	int jj = followingPath; // until I know what performance is like, stick to a single path
	vector<double> dists = edgeDetector->pathDist(posUpdate, jj, useSkel); //gets the distances to path points
	vector<bool> close(dists.size(), false);
	
	
	for (int ii = 0; ii<dists.size(); ii++) {
		close[ii] = (dists[ii] <= followingThresh);
	}
	bool follow = false;
	for (int ii = 0; ii<close.size(); ii++) {
	    if (close[ii]) {
	        follow = true;
	        break;
	    }
	}
	
	followed[jj] = vectorOR(followed[jj], close); // bools for individual path points
	pos.push_back(posUpdate);
	
	updateFollowing(follow);
}

//-----------------------------------------------------------------
// Updates the variables regarding current tracking state.
void ofxTrackingHistory::updateFollowing(bool currFollowing)
{
    double followingProp = followingProportion(followingPath);
	if (isFollowing) {// previously on the trail
        if (!currFollowing) { // has left the trail
            isFollowing = false;
            continuousProp = 0;
        } else {  // stayed on trail
            isFollowing = true;
            continuousProp = followingProp - initialProp;
        }
    } else { // wasn't on the trail
        if (currFollowing) { // started to be on trail
            isFollowing = true;
            initialProp = followingProp;
            continuousProp = 0;
        } // nothing needs to be done if stays off
    }
}

//-----------------------------------------------------------------
double ofxTrackingHistory::followingProportion(int path)
{
	if (path != followingPath) // this should eventually be eliminated.  Should track all paths
		return(0);
	else 
		return( trueCount(followed[path]) / followed[path].size() );
}

//-----------------------------------------------------------------
void ofxTrackingHistory::reset()
{
	// All of the paths have different sizes, so initialize the followed vector to match that.
	vector<int> numPathPts = edgeDetector->numPathPoints(useSkel);
	followed = vector<vector<bool>>(numPathPts.size());
	for (int i=0; i < numPathPts.size(); i++) {  
		followed[i] = vector<bool>(numPathPts[i], false);
	}
}

// ------------Vector manipulation utility functions --------------------
vector<bool> vectorOR(vector<bool>& v1, vector<bool>& v2)
{
	int minval = min(v1.size(), v2.size());
	int i = 0;
	vector<bool> orv(minval, false);
	while (i < minval) {
		orv[i] = v1[i] || v2[i];
		i++;
	}
	return(orv);
}
//-----------------------------------------------------------------
double trueCount(vector<bool>& v)
{
	int nTrue = 0;

	for (int ii=0; ii < v.size(); ii++) {
		if (v[ii] == true)
			nTrue++;
	}
	return(nTrue);
}