#ifndef OFXFC2MOVIEWRITER_H_INCLUDED
#define OFXFC2MOVIEWRITER_H_INCLUDED

#include "FlyCapture2.h"
#include <string>
#include <vector>

using namespace std;
using namespace FlyCapture2;

enum AviType
{
    UNCOMPRESSED,
    MJPG,
    H264
};

class ofxFC2MovieWriter
{
	public:
		ofxFC2MovieWriter();
		~ofxFC2MovieWriter();
		void init(string fileNameBase, AviType aviType, float frameRate, int width, int height);
		void openMovieFile(string fileNameBase, AviType aviType, float frameRate, int width, int height);
		void writeGrayMovieFrame(unsigned char *frame_data, int width, int height);
		void closeMovieFile();
	protected:
		AviType aviType;
		string aviFileName;
		Error error;
		AVIRecorder *movRecorder;
		bool fileOpen;
		time_t rawtime;
		struct tm *timeinfo;
		char *makeFormattedTimestamp();

};


#endif // OFXFC2MOVIEWRITER_H_INCLUDED