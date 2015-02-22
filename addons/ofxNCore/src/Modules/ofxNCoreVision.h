
/*
 *  ofxNCoreVision.h
 *  NUI Group Community Core Vision
 *
 *  Created by NUI Group Dev Team A on 2/1/09.
 *  Updated by Anatoly Churikov on 12/03/11
 *  Copyright 2011 NUI Group. All rights reserved.
 *
 */
#ifndef _ofxNCoreVision_H
#define _ofxNCoreVision_H

//Main
#include "ofMain.h"
#ifdef TARGET_WIN32
	#include "ofxMultiplexerManager.h"
	#include "ofxMultiplexer.h"
	#include <time.h>
	#include <stdio.h>
	#include <Windows.h>
#endif
#include "ofxOpenCv.h"
#include "ofxDirList.h"
#include "ofxVectorMath.h"
#include "ofxNetwork.h"
#include "ofxOsc.h"
#include "ofxThread.h"
#include "ofxXmlSettings.h"
#include "ofxFiducialTracker.h"
#include "ofxFC2MovieWriter.h"
#include "ofxEdgeDetector.h"
#include "ofxTrackingHistory.h"
#include "CSerial.h"
#include "parallelPort.h"
extern "C" {
#include <NIDAQmx.h>
#pragma comment(linker, "/DEFAULTLIB:NIDAQmx.lib")
}

// Our Addon
#include "ofxNCore.h"

// height and width of the source/tracked draw window
#define MAIN_WINDOW_WIDTH  326.0f
#define MAIN_WINDOW_HEIGHT 246.0f

// MAIN AREA POSITIONS
#define MAIN_AREA_X 760
#define MAIN_AREA_Y 30
#define MAIN_AREA_WIDTH 160
#define MAIN_AREA_HEIGHT 160
#define MAIN_AREA_SLIDER_WIDTH 140
#define MAIN_AREA_SLIDER_HEIGHT 10
#define MAIN_AREA_LABEL_WIDTH 140
#define MAIN_AREA_LABEL_HEIGHT 5
#define MAIN_AREA_MATRIX_WIDTH 140
#define MAIN_AREA_MATRIX_HEIGHT 60
#define MAIN_AREA_PREV_X 10
#define MAIN_AREA_PREV_Y 140
#define MAIN_AREA_NEXT_X 110
#define MAIN_AREA_NEXT_Y 140

// MAIN PANEL POSITIONS
#define MAIN_PANEL_WIDTH 175
#define MAIN_PANEL_X 575
#define MAIN_PANEL_SLIDER_WIDTH 155
#define MAIN_PANEL_SLIDER_HEIGHT 10
#define MAIN_PANEL_Y_OFFSET 15
#define MAIN_PANEL_SECOND_WIDTH 170
#define MAIN_PANEL_SECOND_X 15
#define MAIN_PANEL_PADDING 10

// MAIN FILTER POSITIONS
#define MAIN_TOP_OFFSET 25
#define MAIN_FILTERS_W 137
#define MAIN_FILTERS_X 250
#define MAIN_FILTERS_Z 110
#define MAIN_FILTERS_Y 140
#define MAIN_FILTERS_T 360

// DEBUG DISPLAY POSITIONS
#define DEBUG_TEXT_OFFSET_X1 815
#define DEBUG_TEXT_OFFSET_Y1 385
#define DEBUG_TEXT_OFFSET_X2 815
#define DEBUG_TEXT_OFFSET_Y2 442

// OVERLAYS DISPLAY POSITIONS
#define OVERLAYS_OFFSET_X1 799
#define OVERLAYS_OFFSET_Y1 50
#define OVERLAYS_OFFSET_X2 799
#define OVERLAYS_OFFSET_Y2 50

// TODO CLEAN UP ABOVE VARS

class ofxNCoreVision : public ofxGuiListener
{
	// ofxGUI setup stuff
	enum
	{
		propertiesPanel,
		propertiesPanel_flipV,
		propertiesPanel_flipH,
		propertiesPanel_settings,
		propertiesPanel_pressure,

		optionPanel,
		optionPanel_tuio_osc,
		optionPanel_tuio_tcp,
		optionPanel_bin_tcp,
		optionPanel_win_hid,
		optionPanel_serial,
		optionPanel_daq,

		calibrationPanel,
		calibrationPanel_calibrate,
		calibrationPanel_warp,
		calibrationPanel_multicam,
		calibrationPanel_tracker,

		sourcePanel,
		
		TemplatePanel,
		TemplatePanel_minArea,
		TemplatePanel_maxArea,

		backgroundPanel,
		backgroundPanel_remove,
		backgroundPanel_dynamic,
		backgroundPanel_learn_rate,

		smoothPanel,
		smoothPanel_use,
		smoothPanel_smooth,

		amplifyPanel,
		amplifyPanel_use,
		amplifyPanel_amp,

		highpassPanel,
		highpassPanel_use,
		highpassPanel_blur,
		highpassPanel_noise,

		experimentPanel,
		experimentPanel_run,
		experimentPanel_reward,
		experimentPanel_distThresh,
		experimentPanel_followingPropThresh,
		experimentPanel_continuousFollowingThresh,
		experimentPanel_maxVelThresh,

		trackedPanel,
		trackedPanel_darkblobs,
		trackedPanel_use,
		trackedPanel_threshold,
		trackedPanel_thresholdDynamic,
		trackedPanel_min_movement,
		trackedPanel_min_blob_size,
		trackedPanel_max_blob_size,
		trackedPanel_normalizing_on,
		trackedPanel_low_normalizing,
		trackedPanel_high_normalizing,
		trackedPanel_outlines,
		trackedPanel_ids,
		trackedPanel_fid_tile_size,
		trackedPanel_fid_threshold,

		trackingPanel, //Panel for selecting what to track-Fingers, Objects or Fiducials
		trackingPanel_trackFingers,
		trackingPanel_trackObjects,
		trackingPanel_trackFiducials,

		logPanel,   //Panel added for logging and saving tracking data
		logPanel_saveBgImage,
		logPanel_detectEdges,
		logPanel_saveMovie,
		logPanel_logFile,
		logPanel_saveBothMovieLog,

		templatePanel,
		savePanel,
		kParameter_SaveXml,
		kParameter_File,
		kParameter_LoadTemplateXml,
		kParameter_SaveTemplateXml,
		
		cameraGridSettingsPanel,
		cameraGridSettingsPanel_x,
		cameraGridSettingsPanel_y,
		cameraGridSettingsPanel_interleave,

		calibrationGridSettingsPanel,
		calibrationGridSettingsPanel_x,
		calibrationGridSettingsPanel_y,

		devicesListPanel,
		devicesListPanel_grid,
		devicesListPanel_arrow_right,
		devicesListPanel_arrow_left,

		generalSettingsPanel,
		generalSettingsPanel_reset_all,
		generalSettingsPanel_apply_settings,
		generalSettingsPanel_exit,
		generalSettingsPanel_fiducial_mode,

		camerasDisplayPanel,
		camerasDisplayPanel_grid,
		camerasDisplayPanel_grid_reset,
		camerasDisplayPanel_grid_setting,

		dragging_image,

		triggerPanel,
		triggerPanel_use,
	
	};

public:
	ofxNCoreVision(bool debug)
	{
		ofAddListener(ofEvents.mousePressed, this, &ofxNCoreVision::_mousePressed);
		ofAddListener(ofEvents.mouseDragged, this, &ofxNCoreVision::_mouseDragged);
		ofAddListener(ofEvents.mouseReleased, this, &ofxNCoreVision::_mouseReleased);
		ofAddListener(ofEvents.keyPressed, this, &ofxNCoreVision::_keyPressed);
		ofAddListener(ofEvents.keyReleased, this, &ofxNCoreVision::_keyReleased);
		ofAddListener(ofEvents.setup, this, &ofxNCoreVision::_setup);
		ofAddListener(ofEvents.update, this, &ofxNCoreVision::_update);
		ofAddListener(ofEvents.draw, this, &ofxNCoreVision::_draw);
		ofAddListener(ofEvents.exit, this, &ofxNCoreVision::_exit);

		vidPlayer = NULL;
        multiplexer  = NULL;
		debugMode = debug;		
		//initialize filter
		filter = NULL;
		filter_fiducial = NULL;
		//fps and dsp calculation
		frames		= 0;
		fps			= 0;
		lastFPSlog	= 0;
		differenceTime = 0;
		saveMovieSubsample = 3;
		//bools
		bCalibration= 0;
		bFullscreen = 0;
		bShowLabels = 1;
		bMiniMode = 0;
		bDrawOutlines = 1;
		bGPUMode = 0;
		bTUIOMode = 0;
		bSerial = 0;
		bSerialOpen = 0;
		bSaveMovie = 0;
		bSavingMovie = 0;
		bSaveBgImage = 0;
		bFidMode = 0;
		bMulticamDialog = false;
		showConfiguration = 0;
		bSavingLog = 0;
		bSaveMovieLog = 0;
		bDetectEdges = 0;
		bUseCameraTrig = 0;
		//camera
		camRate = 100;
		camWidth = 320;
		camHeight = 240;
		//ints/floats
		backgroundLearnRate = .01;
		MIN_BLOB_SIZE = 2;
		MAX_BLOB_SIZE = 100;
		startCalibrating = false;
		contourFinder.bTrackFingers=false;
		contourFinder.bTrackObjects=false;
		contourFinder.bTrackFiducials=false;
		bcamera = false;
		bWinTouch = 0;
        //if auto tracker is defined then the tracker automagically comes up
        //on startup..
        #ifdef STANDALONE
            bStandaloneMode = true;
        #else
            bStandaloneMode = false;
        #endif
			capturedData = NULL;
		bMultiCamsInterface = false;
		camsGrid = NULL;
		devGrid = NULL;
		draggingImage = NULL;
		bDraggingImage = false;
		draggingXOffset = 0.0f;
		draggingYOffset = 0.0f;
		draggingImage = new ofxGuiImage();
		interleaveMode = false;
		bFidtrackInterface = false;
		// NI DAQ
		nidaqInHandle = 0;
		nidaqOutHandle = 0;
		outputLine = "Dev1/port0/line7";
		inputLine = "Dev1/port1/line0";
		bDaqOpen = 0;
		bUseDaq = 1;
		daqOut = 0;
		daqErrorStr[0] = '\0';
		trackingHist = NULL;
		distThresh = 20;
		followingPropThresh = .80;
		continuousFollowingThresh = .20; 
		maxVelThresh = .8; // this is in px/msec, so it will be low
		bRewardEarned = false;
		bRunExperiment = false;
	}

	~ofxNCoreVision()
	{
		// AlexP
		// C++ guarantees that operator delete checks its argument for null-ness
		delete filter;		filter = NULL;
		delete filter_fiducial;		filter_fiducial = NULL;
		delete multiplexer;		multiplexer = NULL;
		delete trackingHist;	trackingHist = NULL;

		if (bSerialOpen) { //closing the serial port
			serialOut.Close();
		}
		if (bDaqOpen) {
			DAQmxStopTask(&nidaqInHandle);
			DAQmxStopTask(&nidaqOutHandle);
		}
	}

	/****************************************************************
	 *						Public functions
	 ****************************************************************/
	ofxNCoreVision* Instance();
	//Basic Events called every frame
    void _setup(ofEventArgs &e);
    void _update(ofEventArgs &e);
	void _draw(ofEventArgs &e);
    void _exit(ofEventArgs &e);
    //Mouse Events
    void _mousePressed(ofMouseEventArgs &e);
    void _mouseDragged(ofMouseEventArgs &e);
    void _mouseReleased(ofMouseEventArgs &e);
    //Key Events
    void _keyPressed(ofKeyEventArgs &e);
    void _keyReleased(ofKeyEventArgs &e);

	//GUI
	void setupControls();
	void		handleGui(int parameterId, int task, void* data, int length);
	ofxGui*		controls;
	void switchMultiCamsGUI( bool showCams );


	//image processing stuff
	void initDevice();
	void getPixels();
	void grabFrameToCPU();
	void grabFrameToGPU(GLuint target);

	//drawing
	void drawFingerOutlines();
	void drawMiniMode();
	void drawFullMode();

	void drawFiducials();

	//Load/save settings
	void loadXMLSettings();
	void saveSettings();

	//Getters
	std::map<int, Blob> getBlobs();
	std::map<int, Blob> getObjects();
	unsigned char* capturedData;
	// Misc
	void updateMainPanels();
	static char *makeFormattedTimestamp();
	/***************************************************************
	 *					Video Capture Devices
	 ***************************************************************/
	#ifdef TARGET_WIN32
		ofxMultiplexer* multiplexer; 
	#endif
    ofVideoPlayer*	vidPlayer;
	vector<CAMERATYPE> supportedCameraTypes;
	ofxMultiplexerManager* multiplexerManager;
	/****************************************************************
	 *            Variables in xml/app_settings.xml Settings file
	 *****************************************************************/
	bool				bMultiCamsInterface;
	bool				bFidtrackInterface;
	bool				bcamera;
    int					deviceID;
	int 				frameseq;
	int 				threshold;
	int					wobbleThreshold;
	int 				camRate;
	unsigned int 		camWidth;
	unsigned int 		camHeight;
	int					winWidth;
	int					winHeight;
	int					saveMovieSubsample;
	int					MIN_BLOB_SIZE;
	int					MAX_BLOB_SIZE;
	float				backgroundLearnRate;
	int					bWinTouch;
	bool				showConfiguration;
	bool  				bMiniMode;
	bool				startCalibrating;
	bool				bShowInterface;
	bool				bShowPressure;
	bool				bDrawOutlines;
	bool				bTUIOMode;
	bool				bSerial, bSerialOpen;
	bool				bParallel, bParallelOpen;
	bool				bUseDaq, bDaqOpen;
	bool				bGiveReward;
	bool				bSaveBgImage;
	bool				bSaveMovie;
	bool				bSavingMovie;
	bool				bSavingLog;
	bool				bSaveMovieLog;
	bool  				bFullscreen;
	bool 				bCalibration;
	bool				bShowLabels;
	bool				bNewFrame;
	bool				bMulticamDialog;
	bool				bUseCameraTrig;
	bool				interleaveMode;
	//filters
	bool				bAutoBackground;
	//modes
	bool				bGPUMode;
	// Saved data
	string				logFileName;
	string				savedMovieFileName;

	//Area slider variables
	int					minTempArea;
	int					maxTempArea;

	//For the fiducial mode drawing
	bool				bFidMode;
	//auto ~ standalone/non-addon
	bool                bStandaloneMode;
	/*****************************************************
	*		Fiducial Finder
	*******************************************************/
	ofxFiducialTracker	fidfinder;

	float				fiducialDrawFactor_Width; //To draw the Fiducials in the right place we have to scale from camWidth to filter->camWidth
    float				fiducialDrawFactor_Height;

	Filters*			filter_fiducial;
	CPUImageFilter		processedImg_fiducial;



	/****************************************************
	 *End xml/app_config.xml variables
	 *****************************************************/
	
	//Debug mode variables
	bool				debugMode;

	//FPS variables
	int 				frames;
	int  				fps;
	float				lastFPSlog;
	int					differenceTime;
	int					framesSinceSave;

	//Fonts
	ofTrueTypeFont		verdana;
	ofTrueTypeFont      sidebarTXT;
	ofTrueTypeFont		bigvideo;

	//Images
	ofImage				background;

	//Blob Tracker
	BlobTracker			tracker;

	//Template Utilities
	TemplateUtils		templates;

	//Template Registration
	ofRectangle			rect;
	ofRectangle			minRect;
	ofRectangle			maxRect;

	//Object Selection bools
	bool				isSelecting;

	//Area sliders


    /****************************************************************
	 *						Private Stuff
	 ****************************************************************/
	string				videoFileName;

	int					maxBlobs;

	// The variable which will check the initilisation of camera
	//to avoid multiple initialisation
	bool				cameraInited; 

	//Calibration
    Calibration			calib;

	//Blob Finder
	ContourFinder		contourFinder;

	//Image filters
	Filters*			filter;
	CPUImageFilter      processedImg;
	ofxCvColorImage		sourceImg;
	CPUImageFilter		sourceGrayImg;
	
	//XML Settings Vars
	ofxXmlSettings		XML;
	string				message;

	//Communication
	TUIO				myTUIO;
	string				tmpLocalHost;
    int					tmpPort;
	int					tmpFlashPort;
	string				serialPortName;
	CSerial				serialOut;
	string				parallelPortName;
	unsigned int		parallelAd;
	// NI DAQ Digital IO
	TaskHandle			nidaqInHandle, nidaqOutHandle;
	string				outputLine;
	string				inputLine;
	char				daqErrorStr[2048]; 
	bool				daqOut;

	//Logging
	char				dateStr [9];
	char				timeStr [9];
	time_t				rawtime;
	struct tm *			timeinfo;
	char				fileName [80];
	FILE *				stream;
	ofxFC2MovieWriter *	movieWriter;
	ofstream			logFile;
	bool				bDetectEdges;
	// Tracking
	//criteria for following
	float				distThresh; // upper limit for distance from trail 
	float				followingPropThresh; // the proportion of trail that must be followed
	float				continuousFollowingThresh; // the proportion that must be followed continuously
	float				maxVelThresh; // the maximum vel for following - to exclude just running over
	bool				bRewardEarned;
	bool				bRunExperiment;
	ofxEdgeDetector		pathDetector; //To detect paths from the background image
	ofxTrackingHistory	*trackingHist; // Keep track of the behavior over time


	void				removeMainPanels();
	void				removeMulticameraPanels();
	void				addMainPanels();
	void				addMulticameraPanels();
	void				updateCameraGridSize( int x, int y );
	void				updateCalibrationGridSize( int x, int y );
	void				setFiducialSettings(bool isFiducialsSettings);
	void				DAQmxErrorCheck(int32 error, TaskHandle handle);
	void				checkSerial();
	void				checkDAQ();
	void				writeLogFile();

	ofxGuiGrid* camsGrid;
	ofxGuiGrid* devGrid;
	ofxGuiImage* draggingImage;
	bool bDraggingImage;
	float draggingXOffset;
	float draggingYOffset;
	int rawCamId;
	
	/*
	// CUDA variables
	gpu_context_t *ctx;
	*/
};
#endif
