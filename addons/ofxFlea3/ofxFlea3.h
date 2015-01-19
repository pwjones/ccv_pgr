#ifndef OFXFLEA3_H_INCLUDED
#define OFXFLEA3_H_INCLUDED

#include "stdio.h"
#include "ofxCameraBase.h"
#include "FlyCapture2.h"
#include "Image.h"

using namespace FlyCapture2;
//
// The maximum number of cameras on the bus.
//
#define _MAX_CAMS   10

class ofxFlea3 : ofxCameraBase
{
    public:
        ofxFlea3();
		~ofxFlea3();
        void initFlea3(int wid,int hei,int startX=0,int startY=0);
        void setDeviceID(int id);
        int getDeviceID();
        void listDevices();
        int getDeviceCount();
		GUID* getBaseCameraGuids(int* camCount);
		void setCameraFeature(CAMERA_BASE_FEATURE featureCode,int firstValue,int secondValue,bool isAuto,bool isEnabled);
		void getCameraFeature(CAMERA_BASE_FEATURE featureCode,int* firstValue,int* secondValue, bool* isAuto, bool* isEnabled,int* minValue,int* maxValue);
		int getCameraBaseCount();
		CAMERA_BASE_FEATURE* getSupportedFeatures(int* featuresCount);
		void callSettingsDialog();
		void setTriggerMode(bool isEnabled, unsigned int polarity=1, unsigned int source=0, float delayms = 0);
		// These are supposed to be protected
		void getNewFrame(Image *newFrame);
		void getNewFrame(Image *newFrame, TimeStamp *ts);
	protected:
		void setCameraType();
		void cameraInitializationLogic();
		void cameraDeinitializationLogic();
		void getNewFrame(unsigned char* newFrame);
    private:
		Camera cams[_MAX_CAMS];
		int fcCameraID;
		int camNum ;
		PGRGuid pgrGuid[_MAX_CAMS];
		Image fcImage[_MAX_CAMS]; //This Image object is from the FlyCapture2 lib
		// Private functions
		void PrintCameraInfo( CameraInfo* pCamInfo );
		double getRequestedFrameRate();
};
#endif // OFXFLEA3_H_INCLUDED
