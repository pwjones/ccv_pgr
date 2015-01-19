
#include "ofxFlea3.h"
#include <cstdio>
#include <iostream>
#include <string>
#include <vector>

using namespace FlyCapture2;

ofxFlea3::ofxFlea3()
{
    fcCameraID=0;
    //First assume there are MAX_CAMS.Then this variable is modified to the
    //true camera number after listDevices() is called.
    camNum=_MAX_CAMS;
}

void PrintError( Error error)                                                                                                                     
{
    error.PrintErrorTrace();
}

void ofxFlea3::listDevices()
{
    printf("The following are Flea3 devices\n");
    // This function enumerates all the cameras found on the machine, across
    // all 1394 buses and cards. 
	BusManager busMgr;
	Error error;
    unsigned int numCameras;
    error = busMgr.GetNumOfCameras(&numCameras);
    if (error != PGRERROR_OK)
    {
        PrintError(error);
        return;
    }

	if ( numCameras < 1 )
    {
       printf( "No camera detected.\n" );
       return;
    } else {
       printf( "Number of cameras detected: %u\n", numCameras );
    }
	camNum = numCameras;
	printf( "Getting GUIDs for the camera(s)\n");
	for (unsigned int i=0; i<numCameras; i++) {
		error = busMgr.GetCameraFromIndex(0, &pgrGuid[i]);
		if (error != PGRERROR_OK) {
			PrintError(error);
			return;
		}
	}

}

void ofxFlea3::setDeviceID(int id)
{
    fcCameraID=id;
}
int ofxFlea3::getDeviceID()
{
    return fcCameraID;
}
void ofxFlea3::initFlea3(int wid,int hei,int startX,int startY)
{
	Error error;
	FC2Config config;
	//If you want full frame, this should be MODE_0.  If you want full coverage but a smaller image,
	//it should be MODE_1
	const Mode k_fmt7Mode = MODE_0;
    const PixelFormat k_fmt7PixFmt = PIXEL_FORMAT_MONO8;
    //Camera *cam;
	listDevices();
    for(int i=0;i<camNum;i++) {
		//
		// Create the cams. This call must be made before any other calls to
		// to the cams are made, as a valid cams is needed for all
		// camera-specific FlyCapture library calls. This call sets the cams
		// to the default settings, but flycaptureInitialize() must be called
		// below to fully initialize the camera for use.
		
		// Newer OO way to construct a camera object and initialize   
		cams[i].Connect( &pgrGuid[i] ); 
		
		// Get and display the camera information
		CameraInfo camInfo;
		error = cams[i].GetCameraInfo(&camInfo);
		if (error != PGRERROR_OK) {
			PrintError(error);
			return;
		}
		PrintCameraInfo(&camInfo);
		
		guid.Data1 = pgrGuid[1].value[1]; // This assigns the first part of the point grey specific GUID to the OF one.

		// Initialize the camera. This call initializes one of the cameras on the
		// bus with the FlyCapturecams that is passed in. This should generally
		// be called right after creating the cams but before doing anything
		// else.
		//
		// This call performs several functions. It sets the camera to communicate
		// at the proper bus speed, turns on color processing (if available) and
		// sets the Bayer orientation to the correct setting. Finally, it also
		// initializes the white balance tables for cameras that support that
		// function.
		//
		// Start grabbing images in the current videomode and framerate. Driver
		// level image buffer allocation is performed at this point. After this
		// point, images will be constantly grabbed by the camera and stored
		// in the buffers on the PC.
		// Start capturing images
		printf("Getting camera configuration\n");
		error = cams[i].GetConfiguration(&config);
		if (error != PGRERROR_OK) {
			printf("There is a config error: ofxFlea3.cpp:111\n");
			PrintError(error);

			return;
		}
		
		// Configuring embedded information to include a timestamp
		//Get embedded information
		EmbeddedImageInfo info;
		error = cams[i].GetEmbeddedImageInfo(&info);
		info.timestamp.onOff = true; //Set embedded timestamp
		error = cams[i].SetEmbeddedImageInfo(&info); //Set embedded information
		
		// Format 7 is the custom video mode that allows "non-standard" resolutions and frame rates.
		// Must use format 7 specific methods
		// Query for available Format 7 modes
		Format7Info fmt7Info;
		bool supported;
		fmt7Info.mode = k_fmt7Mode;
		error = cams[i].GetFormat7Info( &fmt7Info, &supported );
		if (error != PGRERROR_OK) {
			PrintError( error );
			return;
		}

		//PrintFormat7Capabilities( fmt7Info );

		Format7ImageSettings fmt7ImageSettings;
		fmt7ImageSettings.mode = k_fmt7Mode;
		fmt7ImageSettings.offsetX = startX;
		fmt7ImageSettings.offsetY = startY;
		fmt7ImageSettings.width = wid;
		fmt7ImageSettings.height = hei;
		fmt7ImageSettings.pixelFormat = k_fmt7PixFmt;

		bool valid;
		Format7PacketInfo fmt7PacketInfo;
		// Validate the settings to make sure that they are valid
		error = cams[i].ValidateFormat7Settings(&fmt7ImageSettings, &valid, &fmt7PacketInfo );
		if (error != PGRERROR_OK) {
			PrintError( error );
			return;
		}
		if ( !valid ) {
			// Settings are not valid
			printf("Format7 settings are not valid\n");
			return;
		}
		// Set the settings to the camera
		error = cams[i].SetFormat7Configuration(&fmt7ImageSettings, fmt7PacketInfo.recommendedBytesPerPacket);
		if (error != PGRERROR_OK) { PrintError( error ); return; }
		
		printf( "Starting capture... \n" );
		error = cams[i].StartCapture();
		if (error != PGRERROR_OK) {
			printf("There is a capture error: ofxFlea3.cpp:112\n");
			PrintError(error);
			return;
		} else {
			printf("Capture started successfully\n");
		}
		// Retrieve frame rate property
		Property frmRate;
		frmRate.type = FRAME_RATE;
		error = cams[i].GetProperty( &frmRate );
		if (error != PGRERROR_OK)
		{
			PrintError( error );
			return;
		}
		printf("Attempting to set camera framerate to %d fps\n", framerate); 
		frmRate.valueA = framerate;
		frmRate.absValue = framerate;
		error = cams[i].SetProperty( &frmRate );
		if (error != PGRERROR_OK)
		{
			PrintError( error );
			return;
		}
		printf( "Frame rate is %3.2f fps\n", frmRate.absValue );

		Image rawImage;
		for (int ii = 0; ii<5; ii++) {
			error = cams[i].RetrieveBuffer(&rawImage);
			if (error != PGRERROR_OK){
				printf("Error grabbing image: ofxFlea3.cpp:122 %u\n", ii);
				continue;
			} else {
				printf( "Grabbed image %d\n", ii );
			}
			printf("Copying image\n");
			fcImage[i].DeepCopy(&rawImage);
			printf("Converting Image");
			Image convIm;
			error = rawImage.Convert( PIXEL_FORMAT_MONO8, &convIm);
			convIm.Save("FleaImage_init.bmp");
		}

    }//end of for loop
}
void ofxFlea3::cameraInitializationLogic()
{
	initFlea3(width,height,left,top);
}
void ofxFlea3::cameraDeinitializationLogic()
{
	Error error;

	for(int i=0;i<camNum;i++){
		printf("Stopping capture\n");
		cams[i].StopCapture();
		// Disconnect the camera
		error = cams[i].Disconnect();
		if (error != PGRERROR_OK) {
			PrintError(error);
		}
	}
}
CAMERA_BASE_FEATURE* ofxFlea3::getSupportedFeatures(int* featuresCount)
{
	*featuresCount = 7;
	CAMERA_BASE_FEATURE* features = (CAMERA_BASE_FEATURE*)malloc(*featuresCount * sizeof(CAMERA_BASE_FEATURE));
	features[0] = BASE_BRIGHTNESS;
	features[1] = BASE_EXPOSURE;
	features[2] = BASE_SATURATION;
	features[3] = BASE_GAMMA;
	features[4] = BASE_SHUTTER;
	features[5] = BASE_GAIN;
	features[6] = BASE_FRAMERATE;
	return features;
}
void ofxFlea3::setCameraFeature(CAMERA_BASE_FEATURE featureCode,int firstValue,int secondValue,bool isAuto,bool isEnabled)
{
	Property deviceProperty;
	deviceProperty.type = UNSPECIFIED_PROPERTY_TYPE;
	switch (featureCode)
	{
	case BASE_BRIGHTNESS:
		deviceProperty.type = BRIGHTNESS;
		break;
	case BASE_EXPOSURE:
		deviceProperty.type = AUTO_EXPOSURE;
		break;
	case BASE_SATURATION:
		deviceProperty.type = SATURATION;
		break;
	case BASE_GAMMA:
		deviceProperty.type = GAMMA;
		break;
	case BASE_SHUTTER:
		deviceProperty.type = SHUTTER;
		break;
	case BASE_GAIN:
		deviceProperty.type = GAIN;
		break;
	case BASE_FRAMERATE:
		deviceProperty.type = FRAME_RATE;
		framerate = firstValue;
		break;
	}
	if (deviceProperty.type != UNSPECIFIED_PROPERTY_TYPE) {
		deviceProperty.onOff = isEnabled;
		deviceProperty.valueA = firstValue;
		deviceProperty.valueB = secondValue;
		deviceProperty.autoManualMode = isAuto;
		for (int i=0; i<camNum; i++) {
			Error error = cams[i].SetProperty(&deviceProperty);
			if (error != PGRERROR_OK) {
				std::cout << "There was an error setting camera properties\n";
			}
		}
	}
}

void ofxFlea3::getCameraFeature(CAMERA_BASE_FEATURE featureCode,int* firstValue,int* secondValue, bool* isAuto, bool* isEnabled,int* minValue,int* maxValue)
{
	Property deviceProperty;
	deviceProperty.type = UNSPECIFIED_PROPERTY_TYPE;
	*firstValue = 0;
	*secondValue = 0;
	*isAuto = false;
	*isEnabled = false;
	*minValue = 0;
	*maxValue = 0;
	switch (featureCode) {
		case BASE_BRIGHTNESS:
			deviceProperty.type = BRIGHTNESS;
			break;
		case BASE_EXPOSURE:
			deviceProperty = AUTO_EXPOSURE;
			break;
		case BASE_SATURATION:
			deviceProperty = SATURATION;
			break;
		case BASE_GAMMA:
			deviceProperty = GAMMA;
			break;
		case BASE_SHUTTER:
			deviceProperty = SHUTTER;
			break;
		case BASE_GAIN:
			deviceProperty = GAIN;
			break;
		case BASE_FRAMERATE:
			deviceProperty = FRAME_RATE;
			break;
	}
	if (deviceProperty.type != UNSPECIFIED_PROPERTY_TYPE) {
		Error error = cams[1].GetProperty(&deviceProperty); //This is obviously only working for the first camera, if multiple are connected.
		if (error != PGRERROR_OK) {
			std::cout << "There was an error getting camera properties\n";
		} else {
			*firstValue = (int) deviceProperty.valueA;
			*secondValue = (int) deviceProperty.valueB;
			*isAuto = deviceProperty.autoManualMode;
			*isEnabled = deviceProperty.onOff;
			*minValue = 0;
			*maxValue = (int) deviceProperty.valueA;
		}
	}
}

void ofxFlea3::setTriggerMode(bool isEnabled, unsigned int polarity, unsigned int source, float delayms)
{
	TriggerMode trig;
	trig.onOff = isEnabled;
	trig.polarity = polarity;
	trig.source = source;
	//delayms = 0;

	for (int i = 0; i<camNum; i++) {
		printf("ofxFlea3::setTriggerMode, cam:%d - isEnabled = %u polarity = %u source = %u delay=%f\n", i, isEnabled, polarity, source, delayms);
		Error error = cams[i].SetTriggerMode(&trig);
		if (error != PGRERROR_OK) {
			printf("Error setting camera trigger modes %d\n", i);
		}
	}
}

void ofxFlea3::getNewFrame(unsigned char* newFrame)
{
	Image rawImage;
	Error error;
	unsigned int dsize;
	error = cams[fcCameraID].RetrieveBuffer(&rawImage);
    if (error != PGRERROR_OK)
    {
		printf("Error grabbing image %u\n", fcCameraID);
    }
	// Get some image information
	PixelFormat pixFormat;
    unsigned int rows=1024, cols=1280, stride;
    rawImage.GetDimensions( &rows, &cols, &stride, &pixFormat );
	dsize = rawImage.GetDataSize();
	memcpy((void *)newFrame, (const unsigned char*)rawImage.GetData(), dsize);
	TimeStamp raw_ts = rawImage.GetTimeStamp(); 
	timestamp = ofxTimeStamp(raw_ts); //construct the timestamp using the PRG TimeStamp
	
	// Create a converted image
    //Image convertedImage;
	// Convert the raw image
	//error = rawImage.Convert( PIXEL_FORMAT_MONO8, &convertedImage );
    //if (error != PGRERROR_OK) {
	//	PrintError( error );
	//	cout << "Error in getNewFrame, conversion\n";
    //}
	rawImage.ReleaseBuffer();
	//dsize = convertedImage.GetDataSize();
	//convertedImage.Save("camImage_ofxFlea3.bmp");
	//unsigned char *tempIm = (unsigned char *)malloc(dsize);
	//memcpy((void *)newFrame, (const unsigned char*)convertedImage.GetData(), dsize);
	
	
	// Image saving to debug
	//Image newIm(height, width, width, tempIm, dsize, FlyCapture2::PIXEL_FORMAT_MONO8);
	//newIm.Save("FleaTemp_image.bmp");
	//memcpy((void *)newFrame, (const void*)tempIm, dsize);
	//convertedImage.ReleaseBuffer();
	//free(tempIm);
}

void ofxFlea3::getNewFrame(Image *newFrame)
{
	// This is the version that includes all of the niceties of the Image object provided
	// by the FlyCapture2 library. Always returns the image in 8-bit Grayscale format. The 
	// function expects the newFrame image to exist.
	Image imCopy;
	Error error;
	Image rawImage;
	//Image *rawImage = fcImage[fcCameraID];
	error = cams[fcCameraID].RetrieveBuffer(&rawImage);
    if (error != PGRERROR_OK)
    {
		printf("Error grabbing image %u\n", fcCameraID);
    }
	// Get some image information
	PixelFormat pixFormat;
    unsigned int rows, cols, stride, dsize;
    TimeStamp raw_ts;
	rawImage.GetDimensions( &rows, &cols, &stride, &pixFormat );
	raw_ts = rawImage.GetTimeStamp();

	// Create a converted image
    Image convertedImage;
	error = fcImage[fcCameraID].Convert( PIXEL_FORMAT_MONO8, newFrame );
    if (error != PGRERROR_OK) {
		PrintError( error );
    }
	//Now check the timestamp of the original image and the converted.  Note, the 
	// converted images are no longer timestamped.
	TimeStamp conv_ts;
	conv_ts = convertedImage.GetTimeStamp();
	printf("Raw TS: %d sec ", raw_ts.seconds);
	printf("%d usec\n", raw_ts.microSeconds);
	//printf("--- Conv TS: %d sec %d usec\n", conv_ts.seconds, conv_ts.microSeconds);
	//rawImage.DeepCopy(&imCopy);
	//dsize = convertedImage.GetDataSize();
	//memcpy(newFrame, convertedImage.GetData(), dsize);
	//newFrame = convertedImage.GetData();
	newFrame->Save("camImage_ofxFlea3_imgv.bmp");
	rawImage.ReleaseBuffer();
}

void ofxFlea3::getNewFrame(Image *newFrame, TimeStamp *ts)
{
	// This is the version that includes all of the niceties of the Image object provided
	// by the FlyCapture2 library. Always returns the image in 8-bit Grayscale format. The 
	// function expects the newFrame image to exist.  The timestamp input is there in order
	// for the function to be able to return that information, which is lost after calling
	// Image.Convert
	Image rawImage, imCopy;
	Error error;
	error = cams[fcCameraID].RetrieveBuffer(&rawImage);
    if (error != PGRERROR_OK)
    {
		printf("Error grabbing image %u\n", fcCameraID);
    }
	// Get some image information
	PixelFormat pixFormat;
    unsigned int rows, cols, stride, dsize;
    TimeStamp raw_ts;
	rawImage.GetDimensions( &rows, &cols, &stride, &pixFormat );
	raw_ts = rawImage.GetTimeStamp();
	*ts = raw_ts;
	// Create a converted image
    Image convertedImage;
	error = rawImage.Convert( PIXEL_FORMAT_MONO8, newFrame );
    if (error != PGRERROR_OK) {
		PrintError( error );
    }
	//Now check the timestamp of the original image and the converted
	TimeStamp conv_ts;
	conv_ts = convertedImage.GetTimeStamp();
	printf("Raw TS: %d sec ", raw_ts.seconds);
	printf("%d usec\n", raw_ts.microSeconds);
	printf("--- Conv TS: %d sec %d usec\n", conv_ts.seconds, conv_ts.microSeconds);
	//rawImage.DeepCopy(&imCopy);
	//dsize = convertedImage.GetDataSize();
	//memcpy(newFrame, convertedImage.GetData(), dsize);
	//newFrame = convertedImage.GetData();
	newFrame->Save("camImage_ofxFlea3_imgv.bmp");
	rawImage.ReleaseBuffer();

}
int ofxFlea3::getDeviceCount()
{
	// Get number of PGRCameras
	int numCams;
	numCams = camNum;
	return numCams;
}
void ofxFlea3::setCameraType()
{
	cameraType = FLEA;
	cameraTypeName = "FLEA";
}

double ofxFlea3::getRequestedFrameRate()
{
	// Set up the frame rate data
    Property prop;
    prop.type = FRAME_RATE;
    if ( &cams[1] == NULL) {
        return 0.0;
    }
    else {
	    Error error = cams[1].GetProperty( &prop );
        return (error == PGRERROR_OK) ? prop.absValue : 0.0;
    }
}

void ofxFlea3::PrintCameraInfo( CameraInfo* pCamInfo )
{
	std::printf(
        "\n*** CAMERA INFORMATION ***\n"
        "Serial number - %u\n"
        "Camera model - %s\n"
        "Camera vendor - %s\n"
        "Sensor - %s\n"
        "Resolution - %s\n"
        "Firmware version - %s\n"
        "Firmware build time - %s\n\n",
        pCamInfo->serialNumber,
        pCamInfo->modelName,
        pCamInfo->vendorName,
        pCamInfo->sensorInfo,
        pCamInfo->sensorResolution,
        pCamInfo->firmwareVersion,
        pCamInfo->firmwareBuildTime );
}

//void ofxFlea3::setCameraFeature(CAMERA_BASE_FEATURE featureCode,int firstValue,int secondValue,bool isAuto,bool isEnabled)
//{
//}
//void ofxFlea3::getCameraFeature(CAMERA_BASE_FEATURE featureCode,int* firstValue,int* secondValue, bool* isAuto, bool* isEnabled,int* minValue,int* maxValue)
//{
//}
int ofxFlea3::getCameraBaseCount()
{
	listDevices();
	return camNum;
}
GUID* ofxFlea3::getBaseCameraGuids(int* camCount)
{
	listDevices();
	*camCount = camNum;
	GUID* guids = (GUID*)malloc(*camCount * sizeof(GUID));
	for (int i=0;i<*camCount;i++)
	{
		GUID tempGuid;

		tempGuid.Data1 = i;
		tempGuid.Data2 = tempGuid.Data3 = 0;
		memset(tempGuid.Data4,0,8*sizeof(unsigned char));
		guids[i] = tempGuid;
	}
	return guids;
}
//CAMERA_BASE_FEATURE* ofxFlea3::getSupportedFeatures(int* featuresCount)
//{
//	CAMERA_BASE_FEATURE *features = NULL;
//	return features;
//}
void ofxFlea3::callSettingsDialog()
{
}


//Clean up
ofxFlea3::~ofxFlea3()
{
	
}

