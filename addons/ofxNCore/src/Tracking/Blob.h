/*
* Blob.h
*
*
* A blob is a homogenous patch represented by a polygonal contour.
* Typically a blob tracker uses the contour to figure out the blob's
* persistence and "upgrades" it with ids and other temporal
* information.
*
*/

#ifndef BLOB_H
#define BLOB_H

#include <vector>
#include "ofTypes.h"
#include "ofxOpenCV.h"

class Blob 
{
    public: 
        vector <ofPoint>    pts;     // the contour of the blob
        int                 nPts;    // number of pts;
		int					id;
	    float               area;
        float               length;
		float               angle;
		float               aspectRatio;
		float				A;		 //rotation velocity
		float               raccel;  //rotation acceleration
		float				maccel;  //distance traveled since last frame
		float				age;     //how long the blob has been at war
		float				sitting; //how long hes been sitting in the same place
		float				downTime;
		float				lastTimeTimeWasChecked;
	    ofRectangle         boundingRect;
		ofRectangle         angleBoundingRect;
		CvBox2D				angleBoundingBox;
		ofPoint             centroid, lastCentroid, D;
		bool				simulated;
		bool				isObject;
		bool                hole;
		int					color;				
		// Methods
		Blob();
		~Blob();
		string print();
		void drawContours(float x, float y, float inputWidth, float inputHeight, float outputWidth, float outputHeight);
		void drawBox(float x, float y, float inputWidth, float inputHeight, float outputWidth, float outputHeight);
};
#endif


