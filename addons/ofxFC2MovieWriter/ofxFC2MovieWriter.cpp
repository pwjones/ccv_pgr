
#include "ofxFC2MovieWriter.h"
#include <cstdio>
#include <iostream>
#include <string>
#include <vector>
#include <stdlib.h>
#include <io.h>
#include <time.h>

using namespace FlyCapture2;

// Some functions that are to just be used here
bool fileExists(const char *filename);
//char *makeFormattedTimestamp();

ofxFC2MovieWriter::ofxFC2MovieWriter()
{
	fileOpen = 0;
	movRecorder = new AVIRecorder();
}

void ofxFC2MovieWriter::init(string fileNameBase, AviType aviType, float frameRate, int width, int height)
{
	char *timeStr;
	int version = 0;
	size_t strLen = fileNameBase.length();
	string ext = ".avi";
	string endstr, tempName;
	
	// Generate a filename consisting of the fileNameBase and a timestamp
	timeStr = makeFormattedTimestamp();
	tempName = fileNameBase;
	tempName.append("_");
	tempName.append(timeStr);
	tempName.append(ext);
	aviFileName = tempName;
	printf("Trying to open file: %s\n", aviFileName.c_str());
	switch (aviType)
    {
    case UNCOMPRESSED:
        {
            AVIOption option;     
            option.frameRate = frameRate;
            error = movRecorder->AVIOpen(aviFileName.c_str(), &option);
        }
        break;
    case MJPG:
        {	
			MJPGOption option;
			option.frameRate = frameRate;
            option.quality = 75;
            error = movRecorder->AVIOpen(aviFileName.c_str(), &option);
        }
        break;
    case H264:
        {
            H264Option option;
            option.frameRate = frameRate;
            option.bitrate = 1000000;
            option.height = height;
            option.width = width;
            error = movRecorder->AVIOpen(aviFileName.c_str(), &option);
        }
        break;
	}
	if (error != PGRERROR_OK)
    {
		error.PrintErrorTrace();
        return;
    }
	fileOpen = 1;
}

void ofxFC2MovieWriter::openMovieFile(string fileNameBase, AviType aviType, float frameRate, int width, int height)
{

}

void ofxFC2MovieWriter::writeGrayMovieFrame(unsigned char *frame_data, int width, int height)
{
	Image movImage = Image(height, width, width, frame_data, sizeof(width*height*sizeof(unsigned char)), PIXEL_FORMAT_MONO8);
	error = movRecorder->AVIAppend(&movImage);
        if (error != PGRERROR_OK)
        {
            error.PrintErrorTrace();
            //continue;
        }
}

void ofxFC2MovieWriter::closeMovieFile()
{
	if(fileOpen) {
		movRecorder->AVIClose();
		fileOpen = 0;
	}
}


ofxFC2MovieWriter::~ofxFC2MovieWriter() 
{
	if(fileOpen) {
		movRecorder->AVIClose();
		delete movRecorder;
	}
}

char *ofxFC2MovieWriter::makeFormattedTimestamp()
{
char *timestr = (char *)malloc(sizeof(char)*80);

rawtime = time(NULL);
timeinfo = localtime(&rawtime);
strftime(timestr,80,"%Y-%m-%d-%H%M%S",timeinfo);
return(timestr);
}

bool fileExists(const char *filename)
{
	/* Check for existence */
   if( (_access( filename, 0 )) != -1 ) {
	   printf( "File %s exists\n", filename );
	   /* Check for write permission */
	   if( (_access( filename, 2 )) != -1 )
			printf( "File %s has write permission\n", filename );
	   return (1);
   } else {
	   return 0;
   }
}

