/*
 *  Tracking.h
 *
 *  Created by Ramsin Khoshabeh on 5/4/08.
 *  Copyright 2008 risenparadigm. All rights reserved.
 *
 * Changelog:
 * 08/15/08 -- Fixed a major bug in the track algorithm
 */

#ifndef _TRACKING_H
#define _TRACKING_H

#include <list>
#include <map>

#include "ofxFiducial.h"
#include "ContourFinder.h"
#include "ofxFiducialTracker.h"
#include "../Events/TouchMessenger.h"
#include "../Calibration/CalibrationUtils.h"

class BlobTracker : public TouchListener
{
public:
	BlobTracker();
	~BlobTracker();

	std::vector<Blob>		trackedBlobs; //tracked blobs
	bool isCalibrating;
	int MOVEMENT_FILTERING;
	ofxFiducialTracker*	fidfinder;
	CalibrationUtils* calibrate;
	
	void track(ContourFinder* newBlobs); //assigns IDs to each blob in the contourFinder
	void passInFiducialInfo(ofxFiducialTracker*	_fidfinder);
	void doFiducialCalculation();
	void setCameraSize(int width,int height);
	std::map<int, Blob>	getTrackedBlobs();
	std::map<int, Blob>	getTrackedObjects();
	void getBlobsCenterOfMass(float& x, float& y);
	float getCenterOfMassVelocity();

private:
	int				camWidth,camHeight;
	int				IDCounter;	  //counter of last blob
	int				fightMongrel;
	float			vel, lastVel;	// The center of mass velocity
	float			comX, comY;
	int				t, lastTimeTimeWasChecked; //this is the global time for computing COM velocity
	std::map<int, Blob>     calibratedBlobs;
	std::map<int,Blob>		trackedObjects;
	std::map<int, Blob>		calibratedObjects;

	int trackKnn(ContourFinder *newBlobs, Blob *track, int k, double thresh);
};

#endif
