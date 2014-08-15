/*
*  Blob.cpp
*
*  Created by Peter Jones on 8/4/2014.
*  Copyright 2014 me, use as you wish.
*
*/

#include "Blob.h"

//----------------------------------------
Blob::Blob() 
{
    area 		= 0.0f;
    length 		= 0.0f;
    hole 		= false;
    nPts        = 0;
	simulated	= false;
	age			= 0.0f;
	sitting		= 0.0f;
	color		= 0xFFFFFF;
	isObject	= false;
	raccel		= 0.0f;
	A			= 0.0f;
	//freakishly long variable name (ala Apple)
	lastTimeTimeWasChecked = ofGetElapsedTimeMillis(); //get current time as of creation
}

Blob::~Blob()
{
}

string Blob::print()
{
	string ptStr = string("Points: ");
	char ptChar[40], posStr[40], areaStr[40];
	string wholeStr = string();
	float height = ofGetHeight();
	float width = ofGetWidth();

	sprintf(posStr, "Pos:[%.2f,%.2f] ", centroid.x, centroid.y);
	sprintf(areaStr, "Area:%d ", nPts); 
	for (int i = 0; i < nPts; i++) {
		sprintf(ptChar, "[%d,%d]", (int)pts[i].x, (int)pts[i].y);
		ptStr.append(ptChar);
	}
	wholeStr.append(posStr);
	wholeStr.append(areaStr);
	wholeStr.append(ptStr);
	return(wholeStr);
}

//----------------------------------------
void Blob::drawContours(float x = 0, float y = 0, float inputWidth = ofGetWidth(), float inputHeight = ofGetHeight(), float outputWidth = ofGetWidth(), float outputHeight = ofGetHeight()) 
{
	glPushMatrix();
	glTranslatef(x + angleBoundingRect.x/inputWidth * outputWidth, y + angleBoundingRect.y/inputHeight * outputHeight, 0.0f);
	glRotatef(-angle, 0.0f, 0.0f, 1.0f);
	glTranslatef(-(x + angleBoundingRect.x/inputWidth * outputWidth), -(y + angleBoundingRect.y/inputHeight * outputHeight), 0.0f);                

	//ofSetColor(0xFFFFFF);
	//ofRect(x + (angleBoundingRect.x - angleBoundingRect.width/2)/inputWidth * outputWidth, y + (angleBoundingRect.y - angleBoundingRect.height/2)/inputHeight * outputHeight, angleBoundingRect.width/inputWidth * outputWidth, angleBoundingRect.height/inputHeight * outputHeight);

	ofSetColor(0x0099FF);
	ofRect(x + (angleBoundingRect.x/inputWidth) * outputWidth, y + ((angleBoundingRect.y - angleBoundingRect.height)/inputHeight) * outputHeight, 1, (angleBoundingRect.height * 2)/inputHeight * outputHeight); //Vertical Plus
	ofRect(x + ((angleBoundingRect.x - angleBoundingRect.width)/inputWidth) * outputWidth, y + (angleBoundingRect.y/inputHeight) * outputHeight, (angleBoundingRect.width * 2)/inputWidth * outputWidth, 1); //Horizontal Plus

	glPopMatrix();

	//draw contours
	ofNoFill();
	ofSetColor(0xFF0099);
	ofBeginShape();
	for (int i = 0; i < nPts; i++)
		   ofVertex(x + pts[i].x/inputWidth * outputWidth, y + pts[i].y/(inputHeight) * outputHeight);
	ofEndShape(true);	
}
// ------------------------------------------------
void Blob::drawBox(float x = 0, float y = 0, float inputWidth = ofGetWidth(), float inputHeight = ofGetHeight(), float outputWidth = ofGetWidth(), float outputHeight = ofGetHeight())
{		
	 glPushMatrix();
	 glTranslatef(x + angleBoundingRect.x/inputWidth * outputWidth, y + angleBoundingRect.y/inputHeight * outputHeight, 0.0f);
	 glRotatef(-angle, 0.0f, 0.0f, 1.0f);
	 glTranslatef(-(x + angleBoundingRect.x/inputWidth * outputWidth), -(y + angleBoundingRect.y/inputHeight * outputHeight), 0.0f);                
	 ofNoFill();
	 
	 ofSetColor(0xFFFFFF);
	 ofRect(x + (angleBoundingRect.x - angleBoundingRect.width/2)/inputWidth * outputWidth, y + (angleBoundingRect.y - angleBoundingRect.height/2)/inputHeight * outputHeight, angleBoundingRect.width/inputWidth * outputWidth, angleBoundingRect.height/inputHeight * outputHeight);

	 glPopMatrix();
}