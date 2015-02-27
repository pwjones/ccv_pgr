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
void vectorSetFalse(vector<bool>& v);
void vectorCopy(vector<bool>& src, vector<bool>& dst);

ofxTrackingHistory::ofxTrackingHistory(ofxEdgeDetector *detector, double thresh)
{
	edgeDetector = detector;
	followingThresh = thresh;
	useSkel = detector->pathsThinned();
	followingPath = 0;
	// All of the paths have different sizes, so initialize the followed vector to match that.
	vector<int> numPathPts = detector->numPathPoints(useSkel);
	printf("useSkel = %d  Num Path Pts:", useSkel);
	for (int ii = 0; ii<numPathPts.size(); ii++) {
		printf("numPathPts[%d]=%d    ", ii, numPathPts[ii]);
	}
	printf("\n");
	followed = vector<vector<bool>>(numPathPts.size());
	for (int i=0; i < numPathPts.size(); i++) {  
		followed[i] = vector<bool>(numPathPts[i], false);
	}
	// Vectors for tracking continuous following
	close = vector<bool>(numPathPts[followingPath], false);
	//prevClose = vector<bool>(numPathPts[followingPath], false);
	diffV = vector<bool>(numPathPts[followingPath], false);
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
	
	//vectorCopy(close, prevClose); // copies the close vector
	//vectorSetFalse(close);
	prevClose = close;
	close = vector<bool>(followed[followingPath].size(), false); //boolean of trail points within distance threshold
	
	double prop = findAdjacentTrailPoints(posUpdate, close); // marks the close trail points true
	double newProp = vectorDiffProp(close, prevClose); // look 1 update back for new points
	
	// check if any of them are true.  
	/*bool follow = false;
	for (int ii = 0; ii<close.size(); ii++) {
	    if (close[ii]) {
	        follow = true;
	        break;
	    }
	}*/
	followed[followingPath] = vectorOR(followed[followingPath], close); // bools for individual path points
	pos.push_back(posUpdate); // save the position to the history
	updateFollowing(newProp, prop>0); // update some other stuff
}

//-----------------------------------------------------------------
// Updates the variables regarding current tracking state.
void ofxTrackingHistory::updateFollowing(double newProp, bool currFollowing)
{
    //double followingProp = followingProportion(followingPath);
	if (isFollowing) {// previously on the trail
        if (!currFollowing) { // has left the trail
            isFollowing = false;
            continuousProp = 0;
        } else {  // stayed on trail
            isFollowing = true;
            continuousProp += newProp;
        }
    } else { // wasn't on the trail
        if (currFollowing) { // started to be on trail
            isFollowing = true;
            //initialProp = followingProp;
            continuousProp = newProp;
        } // nothing needs to be done if stays off
    }
}

//-----------------------------------------------------------------------------------------
double ofxTrackingHistory::findAdjacentTrailPoints(cv::Point pos, vector<bool>& closeTrail)
{
	//for(jj=0; jj<edgeDetector->numPaths;i++) {
	int followCount = 0;
	int jj = followingPath; // until I know what performance is like, stick to a single path
	vector<double> dists = edgeDetector->pathDist(pos, jj, useSkel); //gets the distances to path points
	double prop;

	for (int ii = 0; ii<dists.size(); ii++) {
		closeTrail[ii] = (dists[ii] <= followingThresh);
		followCount += closeTrail[ii];
	}
	prop = static_cast<double>(followCount)/closeTrail.size();
	//printf("followCount = %d  Dists = %d Prop = %f\n", followCount, closeTrail.size(), prop);
	return(prop);
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
	useSkel = edgeDetector->pathsThinned();
	vector<int> numPathPts = edgeDetector->numPathPoints(useSkel);
	
	printf("useSkel = %d  Num Path Pts:", useSkel);
	for (int ii = 0; ii<numPathPts.size(); ii++) {
		printf("numPathPts[%d]=%d    ", ii, numPathPts[ii]);
	}
	printf("\n");
	
	followed = vector<vector<bool>>(numPathPts.size());
	for (int i=0; i < numPathPts.size(); i++) {  
		followed[i] = vector<bool>(numPathPts[i], false);
	}
	close = vector<bool>(numPathPts[followingPath], false);
	prevClose = vector<bool>(numPathPts[followingPath], false);
	diffV = vector<bool>(numPathPts[followingPath], false);
}

// ------------Vector manipulation utility functions --------------------
vector<bool> vectorOR(vector<bool>& v1, vector<bool>& v2)
{
	if (v1.size() != v2.size()) {
		printf("In VectorOR, vectors are not the same size!!!!\n");
	}
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
	//printf("nTrue: %d trues, %d size vector\n", nTrue, v.size());
	return(nTrue);
}
//-----------------------------------------------------------------
void vectorSetFalse(vector<bool>& v)
{
	for (int ii = 0; ii < v.size(); ii++) {
		v[ii] = false;
	}
}
//-----------------------------------------------------------------
void vectorCopy(vector<bool>& src, vector<bool>& dst)
{
	if (src.size() != dst.size()) {
		printf("Bad News: Called vectorCopy with different sized vectors! Returning without copying\n"); 
		return;
	}

	for (int ii = 0; ii < src.size(); ii++) {
		dst[ii] = src[ii];
	}
}
//-----------------------------------------------------------------
double ofxTrackingHistory::vectorDiffProp(vector<bool> v1, vector<bool> v2)
{
	if (v1.size() != v2.size() || (v1.size() != diffV.size())) {
		printf("Bad News: Called vectorDiffProp with different sized vectors! Returning -1\n"); 
		return (-1);
	}

	//vector<bool> diffV (v1.size(), false);
	int count = 0;
	double prop;

	int diff;
	for (int ii=0; ii < v1.size(); ii++) {
			diff = v1[ii] - v2[ii];
			if (diff > 0) {
				count++;
				diffV[ii] = true;
			} else {
				diffV[ii] = false;
			}
	}
	prop = static_cast<double>(count)/diffV.size(); // Need to cast one of the ints to avoid truncation
	return(prop);
}
