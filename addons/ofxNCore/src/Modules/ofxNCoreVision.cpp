/*
*  ofxNCoreVision.cpp
*  NUI Group Community Core Vision
*
*  Created by NUI Group Dev Team A on 2/1/09.
*  Updated by Anatoly Churikov on 12/03/11
*  Copyright 2011 NUI Group. All rights reserved.
*
*/

#include "ofxNCoreVision.h"
#include "../Controls/gui.h"
#include "FlyCapture2.h"
// #include "API/gpu_filter_api.h"

//#include "ofxFC2MovieWriter.h"

/******************************************************************************
* The setup function is run once to perform initializations in the application
*****************************************************************************/
void ofxNCoreVision::_setup(ofEventArgs &e)
{
	//set the title
	ofSetWindowTitle("Community Core Vision - 1.5");
	//create filter
	if(filter == NULL)	filter = new ProcessFilters();
//	if ( filter_fiducial == NULL ){filter_fiducial = new ProcessFilters();}
	if ( filter_fiducial == NULL ){filter_fiducial = new ProcessFiducialFilters();}

	parallelAd = 0x378;
	//Load Settings from config file
	loadXMLSettings();

	if(debugMode) {
		printf("DEBUG MODE : Printing to File\n");
		/*****************************************************************************************************
		* LOGGING
		******************************************************************************************************/
		/* alright first we need to get time and date so our logs can be ordered */
		time ( &rawtime );
		timeinfo = localtime ( &rawtime );
		strftime (fileName,80,"../logs/log_%B_%d_%y_%H_%M_%S.txt",timeinfo);
		FILE *stream ;
		sprintf(fileName, ofToDataPath(fileName).c_str());
		if((stream = freopen(fileName, "w", stdout)) == NULL){}
		/******************************************************************************************************/
	}

	cameraInited = false;

	//Setup Window Properties
	ofSetWindowShape(winWidth,winHeight);
	ofSetVerticalSync(false);	            //Set vertical sync to false for better performance?

	//printf("Application Loaded...\n?");

	//load camera/video
	initDevice();
	printf("Camera(s)/Video Initialized...\n");
	//set framerate
	ofSetFrameRate(camRate * 1.3);			//This will be based on camera fps in the future

	/*****************************************************************************************************
	* Allocate images (needed for drawing/processing images)
	******************************************************************************************************/
	processedImg.allocate(camWidth, camHeight); //main Image that'll be processed.
	processedImg.setUseTexture(false);			//We don't need to draw this so don't create a texture
	sourceImg.allocate(camWidth, camHeight);    //Source Image
	sourceImg.setUseTexture(false);				//We don't need to draw this so don't create a texture
	sourceGrayImg.allocate(camWidth, camHeight); //Image that'll be used for video.
	sourceGrayImg.setUseTexture(false);			//We don't need to draw this so don't create a texture

	//Fiducial Images
	processedImg_fiducial.allocate(camWidth, camHeight); //main Image that'll be processed.
	processedImg_fiducial.setUseTexture(false);                        //We don't need to draw this so don't create a texture
	/******************************************************************************************************/

	//Fonts - Is there a way to dynamically change font size?
	verdana.loadFont("fonts/verdana.ttf", 8, true, true);	   //Font used for small images

	//Static Images
	background.loadImage("images/background.jpg"); //Main Background
	//GUI Controls
	controls = ofxGui::Instance(this);
	setupControls();
	/*
	if ((bcamera) && ( multiCams == NULL ))
	{
		multiCams = new ofxNCoreMulticamera();
		multiCams->passInCoreVision( this );
		multiCams->passInMultiplexerManager( multiplexerManager );
		multiCams->setup();
		printf("Multicamera settings GUI initialised...\n");
	}
	*/
	//printf("Controls Loaded...\n");
	tracker.setCameraSize(camWidth,camHeight);
	//Setup Calibration

	calib.SetTracker(&tracker);
	//Allocate Filters
	filter->allocate( camWidth, camHeight );
	filter_fiducial->allocate( camWidth, camHeight );
	
	//Fiducial Initialisation

	// factor for Fiducial Drawing. The ImageSize specified in multiple #define statements (change all)!(Look at ProcessFilters.h at the draw() Method
	fiducialDrawFactor_Width = MAIN_WINDOW_WIDTH / static_cast<float>(filter->camWidth);//camWidth;
	fiducialDrawFactor_Height = MAIN_WINDOW_HEIGHT / static_cast<float>(filter->camHeight);//camHeight;


	/*****************************************************************************************************
	* Startup Modes
	******************************************************************************************************/
	//If Standalone Mode (not an addon)
	if (bStandaloneMode)
	{
		printf("Starting in standalone mode...\n\n");
		showConfiguration = true;
	}
	if (bMiniMode)
	{
		showConfiguration = true;
		bShowInterface = false;
		printf("Starting in Mini Mode...\n\n");
		ofSetWindowShape(190, 200); //minimized size
		filter->bMiniMode = bMiniMode;
	}
	else{
		bShowInterface = true;
		printf("Starting in full mode...\n\n");
	}

	//If Object tracking activated
	if(contourFinder.bTrackObjects)
	{
		templates.loadTemplateXml();
	}

	contourFinder.setTemplateUtils(&templates);
	tracker.passInFiducialInfo(&fidfinder);

	//Setup the movieWriter to save the incoming video
	movieWriter = new ofxFC2MovieWriter();

	/*
	//CUDA context initialization
	if ( gpu_context_create(&ctx) != GPU_OK )
	{
		GPU_ERROR("Unable to create GPU context");
		//return 0;
	}
	if ( gpu_context_init( ctx, camHeight, camWidth, 1, GPU_MEMORY_HOST) != GPU_OK ){
		GPU_ERROR("Unable to initialize GPU context");
		//break;
	}
	*/

	#ifdef TARGET_WIN32
		//get rid of the console window
		//FreeConsole();
	#endif



	printf("CCV is Initialized!\n\n");

}

/****************************************************************
*	Load/Save config file Settings
****************************************************************/
void ofxNCoreVision::loadXMLSettings()
{
	// TODO: a seperate XML to map keyboard commands to action
	message = "Loading Settings...";
	// Can this load via http?
	if ( XML.loadFile("xml/app_settings.xml"))
		message = "Settings Loaded!\n\n";
	else
		message = "No Settings Found...\n\n"; //FAIL

	//--------------------------------------------------------------
	//  START BINDING XML TO VARS
	//--------------------------------------------------------------
	winWidth					= XML.getValue("CONFIG:WINDOW:WIDTH", 927);
	winHeight					= XML.getValue("CONFIG:WINDOW:HEIGHT", 600);
	camWidth					= XML.getValue("CONFIG:MULTIPLEXER:WIDTH", 320);
	camHeight					= XML.getValue("CONFIG:MULTIPLEXER:HEIGHT", 240);
	myTUIO.width = camWidth;
	myTUIO.height = camHeight;
	camRate						= XML.getValue("CONFIG:MULTIPLEXER:FRAMERATE", 0);
	if (XML.getValue("CONFIG:MULTIPLEXER:CAMERATYPES:PS3", 0))
		supportedCameraTypes.push_back(PS3);
	if (XML.getValue("CONFIG:MULTIPLEXER:CAMERATYPES:CMU", 0))
		supportedCameraTypes.push_back(CMU);
	if (XML.getValue("CONFIG:MULTIPLEXER:CAMERATYPES:FFMV", 0))
		supportedCameraTypes.push_back(FFMV);
	if (XML.getValue("CONFIG:MULTIPLEXER:CAMERATYPES:KINECT", 0))
		supportedCameraTypes.push_back(KINECT);
	if (XML.getValue("CONFIG:MULTIPLEXER:CAMERATYPES:DIRECTSHOW", 0))
		supportedCameraTypes.push_back(DIRECTSHOW);
	if (XML.getValue("CONFIG:MULTIPLEXER:CAMERATYPES:FLEA", 0))
		supportedCameraTypes.push_back(FLEA);
	videoFileName				= XML.getValue("CONFIG:VIDEO:FILENAME", "test_videos/RearDI.m4v");
	savedMovieFileName			= XML.getValue("CONFIG:VIDEO:SAVEDFILENAME", "data/output/video");
	logFileName					= XML.getValue("CONFIG:VIDEO:LOGFILENAME", "data/output/trackingLog");
	saveMovieSubsample          = XML.getValue("CONFIG:VIDEO:SUBSAMPLE", 3);
	bcamera						= XML.getValue("CONFIG:SOURCE","VIDEO") == "MULTIPLEXER";
	maxBlobs					= XML.getValue("CONFIG:BLOBS:MAXNUMBER", 20);
	bShowLabels					= XML.getValue("CONFIG:BOOLEAN:LABELS",0);
	bDrawOutlines				= XML.getValue("CONFIG:BOOLEAN:OUTLINES",0);
	filter->bLearnBakground		= XML.getValue("CONFIG:BOOLEAN:LEARNBG",0);
	filter->bVerticalMirror		= XML.getValue("CONFIG:BOOLEAN:VMIRROR",0);
	filter->bHorizontalMirror	= XML.getValue("CONFIG:BOOLEAN:HMIRROR",0);

	//Filters
	filter->bTrackDark			= XML.getValue("CONFIG:BOOLEAN:TRACKDARK", 0);
	filter->bHighpass			= XML.getValue("CONFIG:BOOLEAN:HIGHPASS",1);
	filter->bAmplify			= XML.getValue("CONFIG:BOOLEAN:AMPLIFY", 1);
	filter->bSmooth				= XML.getValue("CONFIG:BOOLEAN:SMOOTH", 1);
	filter->bDynamicBG			= XML.getValue("CONFIG:BOOLEAN:DYNAMICBG", 1);
	filter->bDynamicTH			= XML.getValue("CONFIG:BOOLEAN:DYNAMICTH",0);
	//MODES
	bGPUMode					= XML.getValue("CONFIG:BOOLEAN:GPU", 0);
	bMiniMode                   = XML.getValue("CONFIG:BOOLEAN:MINIMODE",0);
	//CONTROLS
	tracker.MOVEMENT_FILTERING	= XML.getValue("CONFIG:INT:MINMOVEMENT",0);
	MIN_BLOB_SIZE				= XML.getValue("CONFIG:INT:MINBLOBSIZE",2);
	MAX_BLOB_SIZE				= XML.getValue("CONFIG:INT:MAXBLOBSIZE",100);
	backgroundLearnRate			= XML.getValue("CONFIG:INT:BGLEARNRATE", 0.01f);
	filter->threshSize			= XML.getValue("CONFIG:INT:THRESHOLDSIZE", 7.0f);

	//Filter Settings
	filter->threshold			= XML.getValue("CONFIG:INT:THRESHOLD",0);
	filter->highpassBlur		= XML.getValue("CONFIG:INT:HIGHPASSBLUR",0);
	filter->highpassNoise		= XML.getValue("CONFIG:INT:HIGHPASSNOISE",0);
	filter->highpassAmp			= XML.getValue("CONFIG:INT:HIGHPASSAMP",0);
	filter->smooth				= XML.getValue("CONFIG:INT:SMOOTH",0);
	filter->normalizingLowLevel = XML.getValue("CONFIG:INT:LOWNORMALIZING",0);
	filter->normalizingHighLevel= XML.getValue("CONFIG:INT:HIGHNORMALIZING",255);
	filter->isNormalizingOn		= XML.getValue("CONFIG:BOOLEAN:ISNORMALIZING",0);
	minTempArea					= XML.getValue("CONFIG:INT:MINTEMPAREA",0);
	maxTempArea					= XML.getValue("CONFIG:INT:MAXTEMPAREA",0);
	bWinTouch					= XML.getValue("CONFIG:BOOLEAN:WINTOUCH",0);

	//Tracking Options
	contourFinder.bTrackFingers				= XML.getValue("CONFIG:BOOLEAN:TRACKFINGERS",0);
	contourFinder.bTrackObjects				= XML.getValue("CONFIG:BOOLEAN:TRACKOBJECTS",0);
	contourFinder.bTrackFiducials			= XML.getValue("CONFIG:BOOLEAN:TRACKFIDUCIALS",0);

	//NETWORK SETTINGS
	bTUIOMode					= XML.getValue("CONFIG:BOOLEAN:TUIO",0);
	myTUIO.bOSCMode				= XML.getValue("CONFIG:BOOLEAN:OSCMODE",1);
	myTUIO.bTCPMode				= XML.getValue("CONFIG:BOOLEAN:TCPMODE",1);
	myTUIO.bBinaryMode			= XML.getValue("CONFIG:BOOLEAN:BINMODE",1);
	myTUIO.bHeightWidth			= XML.getValue("CONFIG:BOOLEAN:HEIGHTWIDTH",0);
	tmpLocalHost				= XML.getValue("CONFIG:NETWORK:LOCALHOST", "localhost");
	tmpPort						= XML.getValue("CONFIG:NETWORK:TUIOPORT_OUT", 3333);
	tmpFlashPort				= XML.getValue("CONFIG:NETWORK:TUIOFLASHPORT_OUT", 3000);
	myTUIO.version              = XML.getValue("CONFIG:NETWORK:VERSION",0);
	myTUIO.setup(tmpLocalHost.c_str(), tmpPort, tmpFlashPort); //have to convert tmpLocalHost to a const char*
	serialPortName				= XML.getValue("CONFIG:NETWORK:SERIAL", "COM3");
	parallelPortName			= XML.getValue("CONFIG:NETWORK:PARALLEL", "LPT1");
	//--------------------------------------------------------------
	//  END XML SETUP

	//Filter for Fiducial setup
	filter_fiducial->bLearnBackground         = false;
	filter_fiducial->bVerticalMirror		  = filter->bVerticalMirror;
	filter_fiducial->bHorizontalMirror        = filter->bHorizontalMirror;
	filter_fiducial->bTrackDark               = filter->bTrackDark;
//^^ did not want to hardcode this , but these will not be any use of this.
	filter_fiducial->fiducialThreshold			= XML.getValue("CONFIG:FIDUCIAL:THRESHOLD", 16);
	filter_fiducial->fiducial_tile_size			= XML.getValue("CONFIG:FIDUCIAL:TILE", 16);
}

void ofxNCoreVision::saveSettings()
{
	XML.setValue("CONFIG:MULTIPLEXER:WIDTH", (int)camWidth);
	XML.setValue("CONFIG:MULTIPLEXER:HEIGHT", (int)camHeight);
	XML.setValue("CONFIG:MULTIPLEXER:FRAMERATE", camRate);
	int isSupported = 0;
	for (int i=0;i<supportedCameraTypes.size();i++)
		if (supportedCameraTypes[i] == PS3)
		{
			isSupported = 1;
			break;
		}
	XML.setValue("CONFIG:MULTIPLEXER:CAMERATYPES:PS3", isSupported);
	isSupported = 0;
	for (int i=0;i<supportedCameraTypes.size();i++)
		if (supportedCameraTypes[i] == CMU)
		{
			isSupported = 1;
			break;
		}
	XML.setValue("CONFIG:MULTIPLEXER:CAMERATYPES:CMU", isSupported);
	isSupported = 0;
	for (int i=0;i<supportedCameraTypes.size();i++)
		if (supportedCameraTypes[i] == FFMV)
		{
			isSupported = 1;
			break;
		}
	XML.setValue("CONFIG:MULTIPLEXER:CAMERATYPES:FFMV", isSupported);
	isSupported = 0;
	for (int i=0;i<supportedCameraTypes.size();i++)
		if (supportedCameraTypes[i] == KINECT)
		{
			isSupported = 1;
			break;
		}
	XML.setValue("CONFIG:MULTIPLEXER:CAMERATYPES:KINECT", isSupported);
	isSupported = 0;
	for (int i=0;i<supportedCameraTypes.size();i++)
		if (supportedCameraTypes[i] == DIRECTSHOW)
		{
			isSupported = 1;
			break;
		}
	XML.setValue("CONFIG:MULTIPLEXER:CAMERATYPES:DIRECTSHOW", isSupported);
	isSupported = 0;
	for (int i=0;i<supportedCameraTypes.size();i++)
		if (supportedCameraTypes[i] == FLEA)
		{
			isSupported = 1;
			break;
		}
	XML.setValue("CONFIG:MULTIPLEXER:CAMERATYPES:FLEA", isSupported);

	XML.setValue("CONFIG:SOURCE", bcamera ? "MULTIPLEXER" : "VIDEO");
	XML.setValue("CONFIG:BOOLEAN:PRESSURE",bShowPressure);
	XML.setValue("CONFIG:BOOLEAN:LABELS",bShowLabels);
	XML.setValue("CONFIG:BOOLEAN:OUTLINES",bDrawOutlines);
	XML.setValue("CONFIG:BOOLEAN:LEARNBG", filter->bLearnBakground);
	XML.setValue("CONFIG:BOOLEAN:VMIRROR", filter->bVerticalMirror);
	XML.setValue("CONFIG:BOOLEAN:HMIRROR", filter->bHorizontalMirror);
	XML.setValue("CONFIG:BOOLEAN:TRACKDARK", filter->bTrackDark);
	XML.setValue("CONFIG:BOOLEAN:HIGHPASS", filter->bHighpass);
	XML.setValue("CONFIG:BOOLEAN:AMPLIFY", filter->bAmplify);
	XML.setValue("CONFIG:BOOLEAN:SMOOTH", filter->bSmooth);
	XML.setValue("CONFIG:BOOLEAN:DYNAMICBG", filter->bDynamicBG);
	XML.setValue("CONFIG:BOOLEAN:DYNAMICTH", filter->bDynamicTH);
	XML.setValue("CONFIG:BOOLEAN:GPU", bGPUMode);
	XML.setValue("CONFIG:INT:MINMOVEMENT", tracker.MOVEMENT_FILTERING);
	XML.setValue("CONFIG:INT:MINBLOBSIZE", MIN_BLOB_SIZE);
	XML.setValue("CONFIG:INT:MAXBLOBSIZE", MAX_BLOB_SIZE);
	XML.setValue("CONFIG:INT:BGLEARNRATE", backgroundLearnRate);
	XML.setValue("CONFIG:INT:THRESHOLD", filter->threshold);
	XML.setValue("CONFIG:INT:HIGHPASSBLUR", filter->highpassBlur);
	XML.setValue("CONFIG:INT:HIGHPASSNOISE", filter->highpassNoise);
	XML.setValue("CONFIG:INT:HIGHPASSAMP", filter->highpassAmp);
	XML.setValue("CONFIG:INT:SMOOTH", filter->smooth);
	XML.setValue("CONFIG:INT:MINTEMPAREA", minTempArea);
	XML.setValue("CONFIG:INT:MAXTEMPAREA", maxTempArea);
	XML.setValue("CONFIG:INT:THRESHOLDSIZE", filter->threshSize);
	XML.setValue("CONFIG:BOOLEAN:MINIMODE", bMiniMode);
	XML.setValue("CONFIG:BOOLEAN:TUIO",bTUIOMode);
	XML.setValue("CONFIG:BOOLEAN:WINTOUCH",bWinTouch);
	XML.setValue("CONFIG:BOOLEAN:TRACKFINGERS",contourFinder.bTrackFingers);
	XML.setValue("CONFIG:BOOLEAN:TRACKOBJECTS",contourFinder.bTrackObjects);
	XML.setValue("CONFIG:BOOLEAN:TRACKFIDUCIALS",contourFinder.bTrackFiducials);
	XML.setValue("CONFIG:BOOLEAN:HEIGHTWIDTH", myTUIO.bHeightWidth);
	XML.setValue("CONFIG:BOOLEAN:OSCMODE", myTUIO.bOSCMode);
	XML.setValue("CONFIG:BOOLEAN:TCPMODE", myTUIO.bTCPMode);
	XML.setValue("CONFIG:NETWORK:VERSION",myTUIO.version );
	XML.setValue("CONFIG:BOOLEAN:BINMODE", myTUIO.bBinaryMode);
	XML.setValue("CONFIG:FIDUCIAL:THRESHOLD", filter_fiducial->fiducialThreshold);
	XML.setValue("CONFIG:FIDUCIAL:TILE", filter_fiducial->fiducial_tile_size);
	XML.saveFile("xml/app_settings.xml");
}

/************************************************
*				Init Device
************************************************/
//Init Device (camera/video)
void ofxNCoreVision::initDevice()
{
	//save/update log file
	if(debugMode) if((stream = freopen(fileName, "a", stdout)) == NULL){}
	if (bcamera)
	{
		#ifdef TARGET_WIN32
		multiplexerManager = new ofxMultiplexerManager();
		for (int i=0;i<supportedCameraTypes.size();i++)
		{
			multiplexerManager->addAllowdedCameraType(supportedCameraTypes[i]);
		}
		multiplexer = new ofxMultiplexer();
		multiplexerManager->setMultiplexerSize(camWidth,camHeight);
		multiplexerManager->setCalibrator(&calib);
		multiplexerManager->setProcessFilter(filter);
		multiplexerManager->setMultiplexer(multiplexer);
		multiplexerManager->startMulticamManager();
		interleaveMode = multiplexerManager->getInterleaveMode();
		#endif
	}
	else
	{
		if( vidPlayer == NULL )
		{
            vidPlayer = new ofVideoPlayer();
            vidPlayer->loadMovie( videoFileName );
            vidPlayer->play();
            vidPlayer->setLoopState(OF_LOOP_NORMAL);
			printf("Video Mode\n\n");
			camHeight = vidPlayer->height;
			camWidth = vidPlayer->width;
			return;
        }
	}
}
/******************************************************************************
* The update function runs continuously. Use it to update states and variables
*****************************************************************************/
void ofxNCoreVision::_update(ofEventArgs &e)
{
	int ii;
	if(debugMode) if((stream = freopen(fileName, "a", stdout)) == NULL){}

	bNewFrame = false;
	if (bcamera)
	{
	#ifdef TARGET_WIN32
		if (calib.calibrating)
		{
			if (calib.shouldStart)
			{
				multiplexerManager->startCalibration();
				calib.shouldStart = false;
			}
			else
				multiplexerManager->updateCalibrationStatus();
		}
		multiplexer->updateStitchedFrame();
		bNewFrame = true;
	#endif	
	}
	else //if video
	{
		vidPlayer->idleMovie();
		bNewFrame = vidPlayer->isFrameNew();
	}
	//if no new frame, return
	if(!bNewFrame)
	{
		return;
	}
	else//else process camera frame
	{
		
		// Calculate FPS of Camera
		frames++;
		float etime = ofGetElapsedTimeMillis();
		if (etime > (lastFPSlog + 1000))
		{
			fps = frames;
			frames = 0;
			lastFPSlog = etime;
		}//End calculation

		float beforeTime = ofGetElapsedTimeMillis();

		if (bGPUMode)
		{
			grabFrameToGPU(filter->gpuSourceTex);
			checkDAQ(); // Stop/start the DAQ,check inputs/outputs if it's on
			filter->applyGPUFilters();
			contourFinder.findContours(filter->gpuReadBackImageGS,  (MIN_BLOB_SIZE * 2) + 1, ((camWidth * camHeight) * .4) * (MAX_BLOB_SIZE * .001), maxBlobs, false);
			if(contourFinder.bTrackFiducials || bFidtrackInterface)
			{
				grabFrameToGPU(filter_fiducial->gpuSourceTex);
				filter_fiducial->applyGPUFilters();
				if (contourFinder.bTrackFiducials)
					fidfinder.findFiducials( filter_fiducial->gpuReadBackImageGS );
			}
		}
		else { // not GPUmode
			grabFrameToCPU();
			checkDAQ(); // Stop/start the DAQ,check inputs/outputs if it's on
			//sourceImg.setFromGrayscalePlanarImages(processedImg, processedImg, processedImg);
			sourceGrayImg.setFromPixels(processedImg.getPixels(), processedImg.getWidth(), processedImg.getHeight()); //copy that image
			filter->applyCPUFilters( processedImg );
			//filter->applyCUDAFilters(ctx, processedImg );
			contourFinder.findContours(processedImg,  (MIN_BLOB_SIZE * 2) + 1, ((camWidth * camHeight) * .4) * (MAX_BLOB_SIZE * .001), maxBlobs, false);
			if(contourFinder.bTrackFiducials || bFidtrackInterface)
			{
				filter_fiducial->applyCPUFilters( processedImg_fiducial );
				if (contourFinder.bTrackFiducials)
					fidfinder.findFiducials( processedImg_fiducial );
			}
		}

		//If Object tracking or Finger tracking is enabled
		if(contourFinder.bTrackFingers || contourFinder.bTrackObjects)
		{
			tracker.track(&contourFinder); // this actually does the assignment of IDs, etc.
		}

		//Map Fiducials from camera to screen position
		if(contourFinder.bTrackFiducials)
		{
			tracker.doFiducialCalculation();
		}

		//Dynamic Background subtraction LearnRate
		if (filter->bDynamicBG)
		{
			filter->fLearnRate = backgroundLearnRate * .0001; //If there are no blobs, add the background faster.
			if (contourFinder.nBlobs > 0) //If there ARE blobs, add the background slower.
			{
				filter->fLearnRate = backgroundLearnRate * .0001;
			}
		}//End Background Learning rate

		//Sending TUIO messages
		if (myTUIO.bOSCMode || myTUIO.bTCPMode || myTUIO.bBinaryMode)
		{
			//printf("sending data osc : %d TCP : %d binary : %d\n", myTUIO.bOSCMode, myTUIO.bTCPMode, myTUIO.bBinaryMode);
			myTUIO.setMode(contourFinder.bTrackFingers , contourFinder.bTrackObjects, contourFinder.bTrackFiducials);
			myTUIO.sendTUIO(&getBlobs(),&getObjects(),&fidfinder.fiducialsList);
		}
		// ---------------------- MOUSE TRACKING SPECIFIC ACTIONS ---------------------// 
		
		// Edge/Trail Detection 
		if (bDetectEdges) {
			pathDetector.updateImage(filter->grayImg);
			pathDetector.detectEdges(); // Will block this image acquisition loop temporarily, which is ok
			if (trackingHist != NULL)
				trackingHist->reset();
			bDetectEdges = 0;	
		}

		//float vel = tracker.getCenterOfMassVelocity() * (camWidth+camHeight)/2; // velocity expressed as pixels rather than percentage
		/*if ((frames % 5) == 0 && vel > .05) {
			printf("DEBUG: Vel = %f\n", vel);
		}*/

		// Track the progress relative to trail/paths
		int rewardedPath = 0;
		if (pathDetector.pathsDetected() && contourFinder.bTrackFingers) {
			if (trackingHist == NULL) {
				trackingHist = new ofxTrackingHistory(&pathDetector, distThresh);
			}
			float x,y;
			contourFinder.getHeadPosition(x,y);
			trackingHist->updatePosition(cv::Point(x,y));
			int trailPx = pathDetector.numPathPoints(1, rewardedPath );
			double prop = trackingHist->followingProportion( rewardedPath ) * trailPx;
			double contProp = trackingHist->continuousFollowingProp( rewardedPath ) * trailPx;
			if (frames == 0) {
				printf("Has followed: %f  Goal: %f  Current prop: %f\n", prop, followingPropThresh, contProp);
			}
			if (prop >= followingPropThresh && contProp >= continuousFollowingThresh) // the animal has earned a reward
				bRewardEarned = 1;
		}
		// ---------------------- END MOUSE TRACKING SPECIFIC FUNCTIONALITY ------------------------- //

		
		// Check if serial comm channel is open - send outputs if so
		checkSerial();

		// This implementation works, but is not ideal because it needs the Flea3 camera, and the
		// getNewImage method to be public, which it shouldn't be.
		if (bSaveBgImage) { 
			printf("Printing bgImage\n");
			unsigned char *saveImg = sourceImg.getPixels();
			ofxFlea3 *cam = (ofxFlea3 *) multiplexerManager->getCameraBase(0);
			FlyCapture2::Image fcImage = FlyCapture2::Image();
			cam->getNewFrame(&fcImage);
			//float h = sourceImg.getHeight(); float w =  sourceImg.getWidth();
			//FlyCapture2::Image fcImage = FlyCapture2::Image(h, w, w, saveImg, sizeof(w*h*sizeof(unsigned char)), FlyCapture2::PIXEL_FORMAT_MONO8);
			fcImage.Save("savedBg.jpg");
			bSaveBgImage = 0;
		}

		if (bSaveMovie) { // Do movie saving things - this functionality also needs the FlyCapture2 library
			if (bSavingMovie) {//then already saving and just want to append
				if (framesSinceSave % saveMovieSubsample == 0) {
					movieWriter->writeGrayMovieFrame(sourceGrayImg.getPixels(), sourceGrayImg.getWidth(), sourceGrayImg.getHeight());
					framesSinceSave = 0;
				}
				framesSinceSave++;
			} else { //start saving the movie
				printf("Starting to save movie on frame: %d\n", frames);
				double fps_offset = -15;
				movieWriter->init(savedMovieFileName.c_str(), MJPG, (fps+fps_offset)/saveMovieSubsample, sourceGrayImg.getWidth(), sourceGrayImg.getHeight());
				bSavingMovie = 1;
				framesSinceSave = 1;
			}
		} else {
			if (bSavingMovie) {
				movieWriter->closeMovieFile();
				bSavingMovie = 0;
			}
		}

		// Write the tracked positions directly to the specified log file
		writeLogFile();
		
		//get DSP time
		differenceTime = ofGetElapsedTimeMillis() - beforeTime;
	}
}

void ofxNCoreVision::checkDAQ()
{
	int32 err; // used for the error codes from the Ni-DAQ
	if (bUseDaq) {
		if (!bDaqOpen) { // open it!
			printf("Opening the NI-DAQ session\n");
			// Create the channels
			err = DAQmxCreateTask("",&nidaqInHandle); DAQmxErrorCheck(err, nidaqInHandle);
			err = DAQmxCreateDIChan(nidaqInHandle,inputLine.c_str(),"",DAQmx_Val_ChanPerLine); DAQmxErrorCheck(err, nidaqInHandle);
			err = DAQmxStartTask(nidaqInHandle); DAQmxErrorCheck(err, nidaqInHandle);// Now start the task
			err = DAQmxCreateTask("",&nidaqOutHandle); DAQmxErrorCheck(err, nidaqOutHandle);
			err = DAQmxCreateDOChan(nidaqOutHandle,outputLine.c_str(),"",DAQmx_Val_ChanPerLine); DAQmxErrorCheck(err, nidaqOutHandle);
			err = DAQmxStartTask(nidaqOutHandle); DAQmxErrorCheck(err, nidaqOutHandle);// Now start the task
			if (!err) { //success
				printf("DAQ successfully started\n");
				bDaqOpen = 1;
			} else { //if we error, don't use the DAQ
				bUseDaq = 0;
			}
		} else {
			// Give a high output whenever there is a reward earned by trail following
			daqOut = bRewardEarned;
			// Flip the frame to high, and the reward status to be what it is, then output that on the NIDAQ 
			frameCountBool = !frameCountBool;
			bool frameEven = (frames % 2) == 0;
			bool bsaving = bSavingLog || bSavingMovie;
			//if (bRewardEarned)
				//printf("DEBUG: TRIGGERING REWARD\n");
			uInt8 data[4] = {frameCountBool, bsaving, daqOut, daqOut};
			err = DAQmxWriteDigitalLines(nidaqOutHandle,1,1,10.0,DAQmx_Val_GroupByChannel,data,NULL,NULL); DAQmxErrorCheck(err, nidaqOutHandle);
			if (err)
				printf("NIDAQ WriteDigitialLines Error Code returned %d", err);
			// Flip frame back down to low, and output again.
			frameCountBool = !frameCountBool;
			data[0] = frameCountBool;
			err = DAQmxWriteDigitalLines(nidaqOutHandle,1,1,10.0,DAQmx_Val_GroupByChannel,data,NULL,NULL); DAQmxErrorCheck(err, nidaqOutHandle);
			if (err)
				printf("NIDAQ WriteDigitialLines Error Code returned %d", err);
			// Read the input channel.  If high, then reset things.
			uInt32 readData;
			int32  read;
			err = DAQmxReadDigitalU32(nidaqInHandle,1,10.0,DAQmx_Val_GroupByChannel,&readData,1,&read,NULL); DAQmxErrorCheck(err, nidaqOutHandle);
			if (err)
				printf("NIDAQ ReadDigitalU32 Error Code returned %d", err);
			if (readData) { //will be 0 most of the time - if not, reset the tracking
				bRewardEarned = false;
				if (trackingHist != NULL) trackingHist->reset();
			}
			//if (frames == 0) { //Every 1000 frames
				//printf("Data acquired: 0x%X\n",readData);
			//}
		}
	} else if (bDaqOpen) {
			//DAQmxErrorCheck (DAQmxStopTask(&nidaqInHandle), nidaqInHandle);
			//DAQmxErrorCheck (DAQmxStopTask(&nidaqOutHandle), nidaqOutHandle);
	}
}

void ofxNCoreVision::checkSerial()
{
	// Serial port input/output
	if (bSerial) {
		if (!bSerialOpen) {
			printf("Opening a serial port connection\n");
			char tmpName[28];
			strcpy(tmpName, serialPortName.c_str());
			//bool serialSuccess = OpenSerialPort(SerialPortName, dcb, hPort);
			bool serialSuccess = serialOut.Open(tmpName, 115200);
			if (serialSuccess)
				bSerialOpen = 1;
		}
		if (contourFinder.bTrackFingers && contourFinder.nBlobs > 0) {
			float comx, comy;
			char tmp[80];
			string serialMsg;

			if (pathDetector.pathsDetected()) { //send distance to trail and CenterOfMass
				contourFinder.getHeadPosition(comx, comy);
				float minDist = pathDetector.minPathDist(cv::Point(comx, comy), 0);
				//printf("  COM: ( %f, %f )     Dist to path: %f\n", comx, comy, minDist);
				sprintf(tmp, "xpos=%d\nypos=%d\ndistToTrail=%.d;\n", (int)comx, (int)comy, (int)minDist);
				serialMsg = tmp;
				serialOut.SendData(serialMsg.c_str(), serialMsg.length());
			} else { // just send CenterOfMass
				contourFinder.getBlobsCenterOfMass(comx, comy);
				sprintf(tmp, "xpos=%d\nypos=%d;\n", (int)comx, (int)comy);
				serialMsg = tmp;
				serialOut.SendData(serialMsg.c_str(), serialMsg.length());
			}
		}
	} else {
		if (bSerialOpen) { //closing the serial port
			bool closeSuccess = serialOut.Close();
			if (closeSuccess)
				bSerialOpen = 0;
		}
	}
}

void ofxNCoreVision::writeLogFile()
{
	if (bSavingLog) { 
		if (logFile.is_open()) {
			//printf("The log file is open\n");
			string blobStr;
			char tstr[50], headStr[100];
			// Do file saving stuff
			sprintf(headStr, "Areas: %d\n", tracker.trackedBlobs.size());
			logFile << headStr;
			for (int ii=0; ii < contourFinder.nBlobs; ii++) {
				blobStr = contourFinder.blobs[ii].print();
				sprintf(tstr, "%0.2f", contourFinder.blobs[ii].lastTimeTimeWasChecked);
				//printf(blobStr.c_str());
				logFile << "t:" << tstr << " " << blobStr << "\n";
			}
			
		} else { // open up the file and start saving 
			const char *logfn_base = logFileName.c_str();
			char logfn[100];
			char *timestr;
			//time(&rawtime);
			rawtime = time(NULL);
			timeinfo = localtime(&rawtime);
			//strftime(timestr,80,"%Y-%m-%d-%H%M%S",timeinfo);
			timestr = ofxNCoreVision::makeFormattedTimestamp();
			sprintf(logfn, "%s_%s.txt", logfn_base, timestr);
			logFile.open(logfn, ios::out);
			if (logFile.fail()) {
				printf("Frame: %d     Failed opening the log file: %s\n", frames, logfn);
			} else {
				printf("Frame  %d     Succeeded opening the log file: %s\n", frames, logfn);
			}
			free(timestr);
			// print a header with the path information
			logFile << "MovieSubsample:" << saveMovieSubsample << "\n";
			logFile << pathDetector.print();
		}
	} else {
		if (logFile.is_open()) {
			printf("Closing the log file.\n");
			logFile.close();
		}
	}
}

/************************************************
*				Input Device Stuff
************************************************/
//get pixels from camera
void ofxNCoreVision::getPixels()
{
	if (bcamera)
	{
		#ifdef TARGET_WIN32
			if (multiplexer!=NULL)
			{
				int w,h;
				if (capturedData == NULL)
				{
					multiplexer->getStitchedFrameSize(&w,&h);
					camWidth = w;
					camHeight = h;
					capturedData = (unsigned char*)malloc(w*h*sizeof(unsigned char));
				}
				multiplexer->getStitchedFrame(&w,&h,capturedData);
				processedImg.setFromPixels(capturedData, camWidth, camHeight);
				//sourceImg.setFromPixels(capturedData, camWidth, camHeight);
				if(contourFinder.bTrackFiducials || bFidtrackInterface){processedImg_fiducial.setFromPixels(capturedData, camWidth, camHeight);}
				// CUDA setup for the frame
				//if(gpu_set_input( ctx, (unsigned char *)capturedData) != GPU_OK) {
				//	GPU_ERROR("Unable to set context buffer");
				//	return;
				//}
				
			}
		#endif
	}
	else
	{
		if(vidPlayer != NULL )
		{
			sourceImg.setFromPixels(vidPlayer->getPixels(), camWidth, camHeight);
			processedImg = sourceImg;
			if(contourFinder.bTrackFiducials || bFidtrackInterface){processedImg_fiducial = sourceImg;}
		}	
	}
}

//Grab frame from CPU
void ofxNCoreVision::grabFrameToCPU()
{
	getPixels();
}

//Grab frame from GPU
void ofxNCoreVision::grabFrameToGPU(GLuint target)
{
	if (bcamera)
	{
		
		glEnable(GL_TEXTURE_2D);
		//glPixelStorei(1);
		glBindTexture(GL_TEXTURE_2D, target);
		#ifdef TARGET_WIN32
		if(multiplexer!=NULL)
		{
			int w,h;
			if (capturedData == NULL)
			{
				multiplexer->getStitchedFrameSize(&w,&h);
				camWidth = w;
				camHeight = h;
				capturedData = (unsigned char*)malloc(w*h*sizeof(unsigned char));
			}	 
			multiplexer->getStitchedFrame(&w,&h,capturedData);
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, camWidth, camHeight, GL_LUMINANCE, GL_UNSIGNED_BYTE, capturedData);
		}
		#endif
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glBindTexture(GL_TEXTURE_2D,0);
	}
	else
	{
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, target);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, camWidth, camHeight, GL_RGB, GL_UNSIGNED_BYTE, vidPlayer->getPixels());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glBindTexture(GL_TEXTURE_2D,0);
	}
}


/******************************************************************************
* The draw function paints the textures onto the screen. It runs after update.
*****************************************************************************/
void ofxNCoreVision::_draw(ofEventArgs &e)
{
	if (showConfiguration)
	{
		//if calibration
		if (bCalibration)
		{
			//Don't draw main interface
			calib.passInContourFinder(contourFinder.nBlobs, contourFinder.blobs);
			calib.doCalibration();
			//filter->draw();
		}
		//if mini mode
		else if (bMiniMode)
		{
			drawMiniMode();
		}
		//if full mode
		else if (bShowInterface)
		{
			drawFullMode();
			if (!bMultiCamsInterface)
			{
				if(bDrawOutlines || bShowLabels) drawFingerOutlines();
				if(contourFinder.bTrackFiducials)
				{
					drawFiducials();
				}
			
				if(contourFinder.bTrackObjects && isSelecting)
				{
					ofNoFill();
					ofSetColor(255, 0, 0);
					ofRect(rect.x,rect.y,rect.width,rect.height);
					ofSetColor(0, 255, 0);
					ofRect(minRect.x,minRect.y,minRect.width, minRect.height);
					ofSetColor(0, 0, 255);
					ofRect(maxRect.x, maxRect.y, maxRect.width, maxRect.height);
				}
			}
			if (!bCalibration && !bMiniMode) {controls->draw();}
		
		}
	}
	if (bMultiCamsInterface && bDraggingImage ) 
		draggingImage->draw();
}

void ofxNCoreVision::drawFullMode()
{
	ofBackground(255, 255, 255);
	ofSetColor(0xFFFFFF);
	//Draw Background Image
	background.draw(0,0);

	if (!bMultiCamsInterface)
	{
		//Draw Image Filters To Screen
		if (bGPUMode) filter->drawGPU();
		else
		{
			filter->drawAllData = !bFidtrackInterface;
			filter->draw();
			if (bFidtrackInterface)
				filter_fiducial->draw();
		}

		ofSetColor(0xFFFFFF);
		string str1 = "Mode: Camera\n";
		string str2 = "Size: ";
		str2+= ofToString(camWidth, 0) + "x" + ofToString(camHeight, 0)  + "\n";
		string str3 = "FPS: ";
		str3+= ofToString(fps, 0)+"\n";
		string str4 = "Processing:";
		string str4a = " ";
		str4a+= ofToString(differenceTime, 0)+" ms \n";
		string str5 = "Tracking: ";
		if(!bFidMode)
		{
			str5+= "Fingers/Objects\n";
		}
		else
		{
			str5+= "Fiducials\n";
		}
		string str6 = "Blobs: ";
		str6+= ofToString(contourFinder.nBlobs,0)+", "+ofToString(contourFinder.nObjects,0)+", "+ofToString(fidfinder.fiducialsList.size(),0)+"\n";
		string str7 = "";
		string str8 = "";
		if (bcamera)
		{
			int tWidth,tHeight;
			str7 = "Camera Grid:\n";
			multiplexer->getCameraGridSize(&tWidth,&tHeight);
			str7+= ofToString(tWidth,0)+"x"+ofToString(tHeight,0)+"\n";
			str8 = "Calibration Grid:\n";
			multiplexer->getCalibrationGridSize(&tWidth,&tHeight);
			str8+= ofToString(tWidth,0)+"x"+ofToString(tHeight,0)+"\n";
		}
		
		ofSetColor(0x555555);
		verdana.drawString(  str3 + str4+str4a+str6 + str1 +  str2 + "\n\n\n" + str7 + str8 , DEBUG_TEXT_OFFSET_X1, DEBUG_TEXT_OFFSET_Y1);

		// REMOVED + str5 
 
		//TUIO data drawing
		char buf[256]="";
		if(myTUIO.bOSCMode && myTUIO.bTCPMode)
		{
			sprintf(buf, "Dual Mode");
		}
		else if(myTUIO.bOSCMode)
		{
			sprintf(buf, "Host: %s\nProtocol: UDP\nPort: %i", myTUIO.localHost, myTUIO.TUIOPort);
		}
		else if(myTUIO.bTCPMode)
		{
			if(myTUIO.bIsConnected)
				sprintf(buf, "Host: %s\nProtocol: TCP\nPort: %i", myTUIO.localHost, myTUIO.TUIOFlashPort);
			else
				sprintf(buf, "Binding Error\nHost: %s\nProtocol: TCP [XML]\nPort: %i", myTUIO.localHost, myTUIO.TUIOFlashPort);
		}
		else if(myTUIO.bBinaryMode)
		{
			if(myTUIO.bIsConnected)
				sprintf(buf, "Host: %s\nProtocol: Binary\nPort: %i", myTUIO.localHost, myTUIO.TUIOFlashPort);
			else
				sprintf(buf, "Binding Error\nHost: %s\nProtocol: Binary\nPort: %i", myTUIO.localHost, myTUIO.TUIOFlashPort);
		}


		ofSetColor(0x555555);
		verdana.drawString(buf, DEBUG_TEXT_OFFSET_X2, DEBUG_TEXT_OFFSET_Y2);
	}
}

void ofxNCoreVision::drawMiniMode()
{
	ofBackground(0, 0, 0);

	////// ------- cut from the full draw
	if (bGPUMode) filter->drawGPU();
	else {
		filter->drawAllData = !bFidtrackInterface;
		filter->draw();
		if (bFidtrackInterface)
			filter_fiducial->draw();
	}
	///// -------- end cut from full draw 
	
	//black background
	ofSetColor(0,0,0);
	ofRect(0,0,ofGetWidth(), ofGetHeight());
	//draw outlines
	if (bDrawOutlines)
	{
		/* REMOVE? (WHY DO WE EVEN SHOW THIS?)*/
		for (int i=0; i<contourFinder.nBlobs; i++)
		{
			contourFinder.blobs[i].drawContours(0,0, camWidth, camHeight, ofGetWidth(), ofGetHeight()-96);
		}
		for (int i=0;i<contourFinder.nObjects; i++)
		{
			contourFinder.objects[i].drawBox(0,0, camWidth, camHeight, ofGetWidth(), ofGetHeight()-96);
		}
		
	}

	//draw grey rectagles for text information
	ofSetColor(128,128,128);
	ofFill();
	ofRect(0,ofGetHeight() - 83, ofGetWidth(), 20);
	ofRect(0,ofGetHeight() - 62, ofGetWidth(), 20);
	ofRect(0,ofGetHeight() - 41, ofGetWidth(), 20);
	ofRect(0,ofGetHeight() - 20, ofGetWidth(), 20);

	//draw text
	ofSetColor(250,250,250);
	verdana.drawString("Calculation  [ms]:        " + ofToString(differenceTime,0),15, ofGetHeight() - 70 );
	verdana.drawString("Camera [FPS]:            " + ofToString(fps,0),15, ofGetHeight() - 50 );
	verdana.drawString("Blob Count:               " + ofToString(contourFinder.nBlobs,0),15, ofGetHeight() - 29 );
	verdana.drawString("Communication:  " ,10, ofGetHeight() - 9 );
	//draw green tuio circle
	if((myTUIO.bIsConnected || myTUIO.bOSCMode) && bTUIOMode)
		ofSetColor(0x00FF00);	//green = connected
	else
		ofSetColor(0xFF0000);	//red = not connected
	ofFill();
	ofCircle(ofGetWidth() - 17 , ofGetHeight() - 10, 5);
	ofNoFill();
	ofSetColor(255,255,255);
}

void ofxNCoreVision::drawFingerOutlines()
{
	//Find the blobs for drawing
	if(contourFinder.bTrackFingers)
	{
		float xscalef = MAIN_WINDOW_WIDTH/camWidth;
		float yscalef = MAIN_WINDOW_HEIGHT/camHeight;

		for (int i=0; i<contourFinder.nBlobs; i++)
		{
			if (bDrawOutlines)
			{
				//Draw contours (outlines) on the source image
				contourFinder.blobs[i].drawContours(MAIN_FILTERS_X, MAIN_TOP_OFFSET, camWidth, camHeight, MAIN_WINDOW_WIDTH, MAIN_WINDOW_HEIGHT);
			}
			if (bShowLabels) //Show ID label;
			{
				float xpos = contourFinder.blobs[i].centroid.x * (MAIN_WINDOW_WIDTH/camWidth);
				float ypos = contourFinder.blobs[i].centroid.y * (MAIN_WINDOW_HEIGHT/camHeight);
				//if (i == 0)
				//	cout << "drawFingerOutlines: " << xpos << " , " << ypos << "\n";
				
				ofSetColor(0xCCFFCC);
				char idStr[1024];

				sprintf(idStr, "id: %i", contourFinder.blobs[i].id);

				verdana.drawString(idStr, xpos + MAIN_FILTERS_X+335, ypos + contourFinder.blobs[i].boundingRect.height/2 + 45);
			}
		}
		if (bDrawOutlines) { //also draw the center of mass of the tracked blobs
			float xpos, ypos;
			float xoff = MAIN_FILTERS_X;
			float yoff = MAIN_TOP_OFFSET;

			contourFinder.getHeadPosition(xpos, ypos);
			//contourFinder.getBlobsCenterOfMass(xpos, ypos);
			xpos = xpos * xscalef;
			ypos = ypos * yscalef;

			//ofSetColor(0xCCFFCC);
			glPushMatrix();
			ofSetColor(0x00CC00);
			ofRect(xoff + xpos, yoff+ypos-2.5, 1, 5); //Vertical Plus
			ofRect(xoff + xpos-2.5, yoff+ypos, 5, 1); //Horizontal Plus
			glPopMatrix();
			//char idStr[] = "x";
			//cout << "COM: " << xpos << " , " << ypos << "\n";
			//verdana.drawString(idStr, xpos + MAIN_FILTERS_X, ypos + MAIN_TOP_OFFSET);
		}
	}
	//Object Drawing
	if(contourFinder.bTrackObjects)
	{
		for (int i=0; i<contourFinder.nObjects; i++)
		{
			if (bDrawOutlines)
			{
				//Draw contours (outlines) on the source image
				contourFinder.objects[i].drawBox(MAIN_FILTERS_X, MAIN_TOP_OFFSET, camWidth, camHeight, MAIN_WINDOW_WIDTH, MAIN_WINDOW_HEIGHT);
			}
			if (bShowLabels) //Show ID label;
			{
				float xpos = contourFinder.objects[i].centroid.x * (MAIN_WINDOW_WIDTH/camWidth);
				float ypos = contourFinder.objects[i].centroid.y * (MAIN_WINDOW_HEIGHT/camHeight);

				ofSetColor(0xCCFFCC);
				char idStr[1024];

				sprintf(idStr, "id: %i", contourFinder.objects[i].id);

				verdana.drawString(idStr, xpos + MAIN_FILTERS_X+335, ypos + contourFinder.objects[i].boundingRect.height/2 + 45);
			}
		}
	}

	ofSetColor(0xFFFFFF);
}

void ofxNCoreVision::drawFiducials()
{
	for (list<ofxFiducial>::iterator fiducial = fidfinder.fiducialsList.begin(); fiducial != fidfinder.fiducialsList.end(); fiducial++)
	{
		fiducial->drawScaled(MAIN_FILTERS_X,MAIN_TOP_OFFSET,fiducialDrawFactor_Width,fiducialDrawFactor_Height);

		fiducial->drawCornersScaled( MAIN_FILTERS_X, MAIN_TOP_OFFSET ,fiducialDrawFactor_Width,fiducialDrawFactor_Height);

		ofSetColor(0,0,255);
		ofSetColor(255,255,255);

	}
}

/*****************************************************************************
* KEY EVENTS
*****************************************************************************/
void ofxNCoreVision::_keyPressed(ofKeyEventArgs &e)
{
 	if (showConfiguration)
	{
		switch (e.key)
		{
		case 9:
		case 11:
			if (bcamera && !bMiniMode && !bCalibration)
				switchMultiCamsGUI(!bMultiCamsInterface);
			break;
		case 'b':
			//multiplexer->setCalibrationCamera(1);
			//if(!bFidMode)
			{
				filter->bLearnBakground = true;
			}
			//else
			{
				filter_fiducial->bLearnBakground = true;
			}
			break;
		case 'o':
			bDrawOutlines ? bDrawOutlines = false : bDrawOutlines = true;
			controls->update(appPtr->trackedPanel_outlines, kofxGui_Set_Bool, &appPtr->bDrawOutlines, sizeof(bool));
			break;
		case 'h':
			filter->bHorizontalMirror ? filter->bHorizontalMirror = false : filter->bHorizontalMirror = true;
			filter_fiducial->bHorizontalMirror ? filter_fiducial->bHorizontalMirror = false : filter_fiducial->bHorizontalMirror = true;
			controls->update(appPtr->propertiesPanel_flipH, kofxGui_Set_Bool, &appPtr->filter->bHorizontalMirror, sizeof(bool));
			break;
		case 't':
			myTUIO.bOSCMode = !myTUIO.bOSCMode;
			myTUIO.bTCPMode = false;
			myTUIO.bBinaryMode = false;
			bTUIOMode = myTUIO.bOSCMode;
			controls->update(appPtr->optionPanel_tuio_tcp, kofxGui_Set_Bool, &appPtr->myTUIO.bTCPMode, sizeof(bool));
			controls->update(appPtr->optionPanel_tuio_osc, kofxGui_Set_Bool, &appPtr->myTUIO.bOSCMode, sizeof(bool));
			controls->update(appPtr->optionPanel_bin_tcp, kofxGui_Set_Bool, &appPtr->myTUIO.bBinaryMode, sizeof(bool));
			//clear blobs
//			myTUIO.blobs.clear();
			break;
		case 'f':
			myTUIO.bOSCMode = false;
			myTUIO.bTCPMode = !myTUIO.bTCPMode;
			myTUIO.bBinaryMode = false;
			bTUIOMode = myTUIO.bTCPMode;
			controls->update(appPtr->optionPanel_tuio_tcp, kofxGui_Set_Bool, &appPtr->myTUIO.bTCPMode, sizeof(bool));
			controls->update(appPtr->optionPanel_tuio_osc, kofxGui_Set_Bool, &appPtr->myTUIO.bOSCMode, sizeof(bool));
			controls->update(appPtr->optionPanel_bin_tcp, kofxGui_Set_Bool, &appPtr->myTUIO.bBinaryMode, sizeof(bool));

			//contourFinder.bTrackFingers = !contourFinder.bTrackFingers;
			//controls->update(appPtr->trackingPanel_trackFingers, kofxGui_Set_Bool, &contourFinder.bTrackFingers, sizeof(bool));
			break;
		case 'd':
			bDetectEdges = !bDetectEdges;
			controls->update(appPtr->logPanel_detectEdges, kofxGui_Set_Bool, &bDetectEdges, sizeof(bool));
			// d used to be fiducial tracking.  Screw that, it's trail detection now. 
			//contourFinder.bTrackFiducials = !contourFinder.bTrackFiducials;
			//bFidMode = contourFinder.bTrackFiducials;
			//controls->update(appPtr->trackingPanel_trackFiducials, kofxGui_Set_Bool, &contourFinder.bTrackFiducials, sizeof(bool));
			break;
		case 'j':
			if (!contourFinder.bTrackObjects)
			{
				ofxGuiPanel* panel2x = controls->addPanel(appPtr->templatePanel, "Templates", MAIN_PANEL_SECOND_X, 455 - (bcamera ? 0 : 80), OFXGUI_PANEL_BORDER, OFXGUI_PANEL_SPACING);
				panel2x->addButton(appPtr->kParameter_SaveTemplateXml, "Save Template" , OFXGUI_BUTTON_HEIGHT, OFXGUI_BUTTON_HEIGHT, kofxGui_Button_Off, kofxGui_Button_Trigger);
				panel2x->addButton(appPtr->kParameter_LoadTemplateXml, "Load Template", OFXGUI_BUTTON_HEIGHT, OFXGUI_BUTTON_HEIGHT, kofxGui_Button_Off, kofxGui_Button_Trigger);
				panel2x->addSlider(appPtr->TemplatePanel_minArea, "Min Area", MAIN_FILTERS_Y, 13, 0.0f, 2000.0f, minTempArea, kofxGui_Display_Int, 0);
				panel2x->addSlider(appPtr->TemplatePanel_maxArea, "Max Area", MAIN_FILTERS_Y, 13, 0.0f, 2000.0f, maxTempArea, kofxGui_Display_Int, 0);
				panel2x->mObjWidth = 200;
				panel2x->mObjHeight = 140;
				panel2x->mObjects[2]->mObjY = 75;
				panel2x->mObjects[3]->mObjY = 105;
				panel2x->adjustToNewContent(120, 0);
			}
			else
			{
				controls->removePanel( this->templatePanel );
			}
			contourFinder.bTrackObjects = !contourFinder.bTrackObjects;
			controls->update(appPtr->trackingPanel_trackObjects, kofxGui_Set_Bool, &contourFinder.bTrackObjects, sizeof(bool));
			break;

/*		case 'f':
			myTUIO.bOSCMode = false;
			myTUIO.bTCPMode = !myTUIO.bTCPMode;
			myTUIO.bBinaryMode = false;
			bTUIOMode = myTUIO.bTCPMode;
			controls->update(appPtr->optionPanel_tuio_tcp, kofxGui_Set_Bool, &appPtr->myTUIO.bTCPMode, sizeof(bool));
			controls->update(appPtr->optionPanel_tuio_osc, kofxGui_Set_Bool, &appPtr->myTUIO.bOSCMode, sizeof(bool));
			controls->update(appPtr->optionPanel_bin_tcp, kofxGui_Set_Bool, &appPtr->myTUIO.bBinaryMode, sizeof(bool));
			//clear blobs
//			myTUIO.blobs.clear();
			break;*/
		case 'n':
			myTUIO.bOSCMode = false;
			myTUIO.bTCPMode = false;
			myTUIO.bBinaryMode = !myTUIO.bBinaryMode;
			bTUIOMode = myTUIO.bBinaryMode;
			controls->update(appPtr->optionPanel_tuio_tcp, kofxGui_Set_Bool, &appPtr->myTUIO.bTCPMode, sizeof(bool));
			controls->update(appPtr->optionPanel_tuio_osc, kofxGui_Set_Bool, &appPtr->myTUIO.bOSCMode, sizeof(bool));
			controls->update(appPtr->optionPanel_bin_tcp, kofxGui_Set_Bool, &appPtr->myTUIO.bBinaryMode, sizeof(bool));
			//clear blobs
			//myTUIO.blobs.clear();
			break;
		case 'm':
			setFiducialSettings(!bFidtrackInterface);
			break;
		case 'g':
			contourFinder.bTrackFingers=!contourFinder.bTrackFingers;
			controls->update(appPtr->trackingPanel_trackFingers, kofxGui_Set_Bool, &contourFinder.bTrackFingers, sizeof(bool));

			break;
		case 'v':
				//#ifdef TARGET_WIN32

				
                //#endif
			filter->bVerticalMirror ? filter->bVerticalMirror = false : filter->bVerticalMirror = true;
			filter_fiducial->bVerticalMirror ? filter_fiducial->bVerticalMirror = false : filter_fiducial->bVerticalMirror = true;
			controls->update(appPtr->propertiesPanel_flipV, kofxGui_Set_Bool, &appPtr->filter->bVerticalMirror, sizeof(bool));
			break;
			break;
		case 'r':
			if (bMultiCamsInterface && bcamera)
			{
					if ( multiplexerManager != NULL )
						multiplexerManager->removeAllCameraBasesFromMultiplexer();
					if ( camsGrid != NULL ) 
						camsGrid->resetAll();
					if ( devGrid != NULL )
						devGrid->setImages();
					if ( camsGrid != NULL ) 
						camsGrid->setImages();
			}
			break;
		case 's':
			if (bMultiCamsInterface)
			{
				if (bcamera)
					multiplexerManager->applySettingsToMultiplexer();
			}
			else
			{
				saveSettings();
			}
			break;
		case 'l':
			bShowLabels ? bShowLabels = false : bShowLabels = true;
			controls->update(appPtr->trackedPanel_ids, kofxGui_Set_Bool, &appPtr->bShowLabels, sizeof(bool));
			break;
		case 'p':
			bShowPressure ? bShowPressure = false : bShowPressure = true;
			break;
		case ' ':
			if (!bMultiCamsInterface)
			{
				if (bMiniMode && !bCalibration) // NEED TO ADD HERE ONLY GO MINI MODE IF NOT CALIBRATING
				{
					bMiniMode = false;
					bShowInterface = true;
					filter->bMiniMode = bMiniMode;
					filter_fiducial->bMiniMode = bMiniMode;
					ofSetWindowShape(927,600); //default size
				}
				else if(!bCalibration)
				{
					bMiniMode = true;
					bShowInterface = false;
					filter->bMiniMode = bMiniMode;
					filter_fiducial->bMiniMode = bMiniMode;
					ofSetWindowShape(190,200); //minimized size
				}
			}
			break;
		case 'x': //Exit Calibrating
			if (!bcamera)
				break;
			if (bCalibration)
			{	
				if (multiplexerManager!=NULL)
					multiplexerManager->cancelCalibration();
				bShowInterface = true;
				bCalibration = false;
				calib.calibrating = false;
				tracker.isCalibrating = false;
				if (bFullscreen == true) ofToggleFullscreen();
				bFullscreen = false;
				ofSetFrameRate(camRate * 1.3);	
			}
			break;
		case OF_KEY_RETURN: //Close Template Selection and save it
			if( contourFinder.bTrackObjects && isSelecting )
			{
			isSelecting = false;
			templates.addTemplate(rect,minRect,maxRect,camWidth/320,camHeight/240);
			rect = ofRectangle();
			minRect = rect;
			maxRect = rect;
			minTempArea = 0;
			maxTempArea = 0;
			controls->update(appPtr->TemplatePanel_minArea, kofxGui_Set_Float, &appPtr->minTempArea, sizeof(float));
			controls->update(appPtr->TemplatePanel_maxArea, kofxGui_Set_Float, &appPtr->maxTempArea, sizeof(float));
			}
			break;
		case 'i':
			if(!bFidMode)
			{
				filter->bTrackDark = !filter->bTrackDark;
				controls->update(appPtr->trackedPanel_darkblobs, kofxGui_Set_Bool, &filter->bTrackDark, sizeof(bool));

			}
			else
			{
				filter_fiducial->bTrackDark = !filter_fiducial->bTrackDark;
				controls->update(appPtr->trackedPanel_darkblobs, kofxGui_Set_Bool, &filter_fiducial->bTrackDark, sizeof(bool));

			}
 			break;
		default: //Check key character <<<<===== Remove this
			//printf("%c",e.key);
			break;
		}
	} 
}

void ofxNCoreVision::_keyReleased(ofKeyEventArgs &e)
{ 
	if (showConfiguration)
	{
		if ( e.key == 'c' && !bCalibration)
		{
			if (!bcamera)
				return;
			ofSetFrameRate(40);	//BUG : when framerate is high - calibration is not stable		
			bShowInterface = false;
			// Enter/Exit Calibration
			bCalibration = true;
			calib.calibrating = true;
			tracker.isCalibrating = true;
			startCalibrating = true;
			multiplexerManager->applySettingsToMultiplexer();
			multiplexerManager->initializeCalibration();
			//multiplexerManager->startCalibration();
			if (bFullscreen == false) ofToggleFullscreen();
			bFullscreen = true;
		}
	}
	if ( e.key == '~' || e.key == '`' && !bMiniMode && !bCalibration) showConfiguration = !showConfiguration;
}

/*****************************************************************************
*	MOUSE EVENTS
*****************************************************************************/
void ofxNCoreVision::_mouseDragged(ofMouseEventArgs &e)
{
	if (showConfiguration)
		controls->mouseDragged(e.x, e.y, e.button); //guilistenerc
	if(contourFinder.bTrackObjects)
	{
		if( e.x > 612 && e.x < 912 && e.y > 25 && e.y < 272 )
		{
			if( e.x < rect.x || e.y < rect.y )
			{
				rect.width = rect.x - e.x;
				rect.height = rect.y - e.y;

				rect.x = e.x;
				rect.y =  e.y;
			}
			else
			{
				rect.width = e.x - rect.x;
				rect.height = e.y - rect.y;
			}
		}
	}
	if ( bDraggingImage && bMultiCamsInterface ) 
	{
		draggingImage->setXYPos(
			e.x - draggingXOffset < 0 ? 0 : e.x - draggingXOffset,
			e.y - draggingYOffset < 0 ? 0 : e.y - draggingYOffset );
	}
}

void ofxNCoreVision::_mousePressed(ofMouseEventArgs &e)
{
	if (showConfiguration)
	{
		controls->mousePressed( e.x, e.y, e.button ); //guilistener
		if ( contourFinder.bTrackObjects )
		{
			if ( e.x > 385 && e.x < 705 && e.y > 30 && e.y < 270 )
			{
				isSelecting = true;
				rect.x = e.x;
				rect.y = e.y;
				rect.width = 0;
				rect.height = 0;
			}
		}
		//Launch the website in browser
		//if ( e.x > 722 && e.y > 586 )
		//ofLaunchBrowser("http://ccv.nuigroup.com");
	}
}

void ofxNCoreVision::_mouseReleased(ofMouseEventArgs &e)
{
	if (showConfiguration)
		controls->mouseReleased(e.x, e.y, 0); //guilistener
	if( e.x > 385 && e.x < 705 && e.y > 30 && e.y < 270 )
	{
		if	( contourFinder.bTrackObjects && isSelecting )
		{
			minRect = rect;
			maxRect = rect;
		}
	}
	if ( bDraggingImage ) 
		this->bDraggingImage = false;
}

/*****************************************************************************
* Getters
*****************************************************************************/

std::map<int, Blob> ofxNCoreVision::getBlobs()
{
	return tracker.getTrackedBlobs();
}

std::map<int,Blob> ofxNCoreVision::getObjects()
{
	return tracker.getTrackedObjects();
}

/*****************************************************************************
* ON EXIT
*****************************************************************************/
void ofxNCoreVision::_exit(ofEventArgs &e)
{
	saveSettings();

	//Save templates
	if(contourFinder.bTrackObjects)
		templates.saveTemplateXml();
	
	// AlexP
	// C++ guarantees that operator delete checks its argument for null-ness
    #ifdef TARGET_WIN32
	if (multiplexer!=NULL)
	{
		delete multiplexer;	multiplexer = NULL;
	}
	if (multiplexerManager!=NULL)
	{
		delete multiplexerManager; multiplexerManager = NULL;
	}
	#endif
	delete vidPlayer; vidPlayer = NULL;
	delete filter;		filter = NULL;
	delete movieWriter;
	if (logFile.is_open()) {
		logFile.close();
	}
	// -------------------------------- SAVE STATE ON EXIT
	printf("Vision module has exited!\n");
}

void ofxNCoreVision::switchMultiCamsGUI( bool showCams ) 
{
	printf( "\nofxNCoreVision::switchMultiCamsGUI\n" );
	if ( showCams ) 
	{
		bMultiCamsInterface = true;
		removeMainPanels();
		addMulticameraPanels();
		if ( camsGrid != NULL ) 
			camsGrid->setImages();
		if ( devGrid != NULL ) 
			devGrid->setImages();
		multiplexerManager->resumeCaptureFromAllCameras();
	} 
	else 
	{
		bMultiCamsInterface = false;
		removeMulticameraPanels();
		addMainPanels();
		multiplexerManager->pauseAllUnusedCameras();
	}
}

void ofxNCoreVision::updateCameraGridSize( int x, int y ) 
{
	//devGrid->setMode( kofxGui_Grid_Selectable, ( x * y <= multiplexerManager->getCameraBaseCount() )  );
	if ( multiplexerManager != NULL ) 
		multiplexerManager->setCameraGridSize( x, y );
	if ( camsGrid != NULL ) 
		camsGrid->setXY( x, y );
	if ( devGrid != NULL ) 
		devGrid->setImages();
}

void ofxNCoreVision::updateCalibrationGridSize( int x, int y )
{
	if ( multiplexerManager != NULL ) 
		multiplexerManager->setCalibratonGridSize( x, y );
}

char * ofxNCoreVision::makeFormattedTimestamp()
{
time_t				lrawtime;
struct tm *			ltimeinfo;
char *timestr = (char *)malloc(sizeof(char)*80);

lrawtime = time(NULL);
ltimeinfo = localtime(&lrawtime);
strftime(timestr,80,"%Y-%m-%d-%H%M%S",ltimeinfo);
return(timestr);
}

void ofxNCoreVision::DAQmxErrorCheck(int32 error, TaskHandle handle)
{
	if (error) {
		if( DAQmxFailed(error) )
			DAQmxGetExtendedErrorInfo(daqErrorStr,2048);
		if( handle!=0 ) {
			/*********************************************/
			// DAQmx Stop Code
			/*********************************************/
			DAQmxStopTask(handle);
			DAQmxClearTask(handle);
		}
		if( DAQmxFailed(error) )
			printf("DAQmx Error: %s\n",daqErrorStr);
		printf("End of program, press Enter key to quit\n");
		getchar();
		//return 0;
	}
}