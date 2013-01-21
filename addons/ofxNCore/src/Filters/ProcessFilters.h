/*
*  ProcessFilters.h
*  
*
*  Created on 2/2/09.
*  Copyright 2009 NUI Group. All rights reserved.
*
*/

#ifndef PROCESS_FILTERS_H_
#define PROCESS_FILTERS_H_

#include "Filters.h"
//#include "opencv2\gpu\gpu.hpp"
//#include "arrayfire.h"
#include <malloc.h>

//using namespace cv;
//using namespace af;

class ProcessFilters : public Filters {

  public:

    void allocate( int w, int h ) 
	{
        camWidth = w;
        camHeight = h;
		//initialize learnrate
		fLearnRate = 0.0f;
        bMiniMode = false;
		exposureStartTime = ofGetElapsedTimeMillis();
        //CPU Setup
        grayImg.allocate(camWidth, camHeight);		//Gray Image
        grayBg.allocate(camWidth, camHeight);		//Background Image
        subtractBg.allocate(camWidth, camHeight);   //Background After subtraction
        grayDiff.allocate(camWidth, camHeight);		//Difference Image between Background and Source
        highpassImg.allocate(camWidth, camHeight);  //Highpass Image
        amplifyImg.allocate(camWidth, camHeight);		//Amplied Image
        floatBgImg.allocate(camWidth, camHeight);	//ofxShortImage used for simple dynamic background subtraction
        //GPU Setup
		gpuReadBackBuffer = new unsigned char[camWidth*camHeight*3];
        gpuReadBackImageGS.allocate(camWidth, camHeight);
        allocateGPU();
		//^^ Commented out for now, till full GPU implementation
    }

    void allocateGPU(){

        glGenTextures(1, &gpuSourceTex);
        glGenTextures(1, &gpuBGTex);

		delete gpuReadBackBuffer;

        gpuReadBackBuffer = new unsigned char[camWidth*camHeight];
        gpuReadBackImageGS.allocate(camWidth, camHeight);

        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, gpuSourceTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8,  camWidth, camHeight, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, NULL);
        glBindTexture(GL_TEXTURE_2D, gpuBGTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8,  camWidth, camHeight, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, NULL);
        glBindTexture(GL_TEXTURE_2D, 0);
        glDisable(GL_TEXTURE_2D);
        subtractFilter = new GPUImageFilter("xml/filters/absSubtract.xml", camWidth, camHeight);
        subtractFilter2 = new GPUImageFilter("xml/filters/subtract.xml", camWidth, camHeight);
        contrastFilter = new GPUImageFilter("xml/filters/contrast.xml", camWidth, camHeight);
        gaussVFilter = new GPUImageFilter("xml/filters/gaussV.xml", camWidth, camHeight);
        gaussHFilter = new GPUImageFilter("xml/filters/gauss.xml", camWidth, camHeight);
        gaussVFilter2 = new GPUImageFilter("xml/filters/gaussV2.xml", camWidth, camHeight);
        gaussHFilter2 = new GPUImageFilter("xml/filters/gauss2.xml", camWidth, camHeight);
        threshFilter = new GPUImageFilter("xml/filters/threshold.xml", camWidth, camHeight);
        copyFilter = new GPUImageFilter("xml/filters/copy.xml", camWidth, camHeight);
        grayScale = new GPUImageFilter("xml/filters/grayScale.xml", camWidth, camHeight);
    }

/****************************************************************
 *	CPU Filters
 ****************************************************************/
void applyCPUFilters(CPUImageFilter& img){

        //Set Mirroring Horizontal/Vertical
        if(bVerticalMirror || bHorizontalMirror) img.mirror(bVerticalMirror, bHorizontalMirror);

        if(!bMiniMode) grayImg = img; //for drawing
        //Dynamic background with learn rate
        if(bDynamicBG){
            floatBgImg.addWeighted( img, fLearnRate);
			//grayBg = floatBgImg;  // not yet implemented
			 cvConvertScale( floatBgImg.getCvImage(), grayBg.getCvImage(), 255.0f/65535.0f, 0 );       
			 grayBg.flagImageChanged();
        }

        //recapature the background until image/camera is fully exposed
        if((ofGetElapsedTimeMillis() - exposureStartTime) < CAMERA_EXPOSURE_TIME) bLearnBakground = true;

        //Capture full background
        if (bLearnBakground == true){
            floatBgImg = img;
			//grayBg = floatBgImg;  // not yet implemented
			cvConvertScale( floatBgImg.getCvImage(), grayBg.getCvImage(), 255.0f/65535.0f, 0 );       
			grayBg.flagImageChanged();
            bLearnBakground = false;
        }

		//Background Subtraction
        //img.absDiff(grayBg, img); 		
		if(bTrackDark)
			cvSub(grayBg.getCvImage(), img.getCvImage(), img.getCvImage());
		else
			cvSub(img.getCvImage(), grayBg.getCvImage(), img.getCvImage());

		img.flagImageChanged();
    
		
		if(bSmooth){//Smooth
            img.blur((smooth * 2) + 1); //needs to be an odd number
            if(!bMiniMode)
            subtractBg = img; //for drawing
        }

        if(bHighpass){//HighPass
            img.highpass(highpassBlur, highpassNoise);
            if(!bMiniMode)
            highpassImg = img; //for drawing
        }

        if(bAmplify){//Amplify
            img.amplify(img, highpassAmp);
            if(!bMiniMode)
            amplifyImg = img; //for drawing
        }

		if (bDynamicTH)
			img.adaptiveThreshold(threshold, -threshSize);
		else
			img.threshold(threshold); //Threshold

        if(!bMiniMode)
        grayDiff = img; //for drawing
	
}

// mem layout for gpu
//void mat_to_array(cv::Mat& input, array& output) 
//{
 //   input.convertTo(input, CV_16U); // floating point
	//
	//const int asize = input.rows * input.cols;
	//const int w = input.cols;
	//const int h = input.rows;
	//
	//float *r; float *g; float *b;
	//size_t bsize = sizeof(float)*asize;
	//if (bsize > 0 && bsize < 1024) {
	//	r = (float *)alloca(bsize);
	//	b = (float *)alloca(bsize);
	//	g = (float *)alloca(bsize);
	//} else {
	//	printf("Tried to allocate too much space on the stack");
	//	_exit(1);
	//}

	////Vector<float> r(asize);
 //   //Vector<float> g(asize);
 //   //Vector<float> b(asize);
 //   int tmp = 0;
 //   for (unsigned i = 0; i < h; i++) {
 //       for (unsigned j = 0; j < w; j++) {
 //           Vec3f ip = input.at<Vec3f>(i, j);
 //           tmp = j * h + i; // convert to column major
 //           r[tmp] = ip[2];
 //           g[tmp] = ip[1];
 //           b[tmp] = ip[0];
 //       }
 //   }
 //   output = join(2,
 //                 array(h, w, r),
 //                 array(h, w, g),
 //                 array(h, w, b))/255.f; // merge, set range [0-1]
//}

// mem layout for gpu - the input is unsigned char, the ouput is floats
//void greymat_to_array(cv::Mat& input, array& output) 
//{
    //input.convertTo(input, CV_16U); // floating point
	
//	const int asize = input.rows * input.cols;
//	const int w = input.cols;
//	const int h = input.rows;
//	
//	float *g;
//	size_t bsize = sizeof(unsigned int)*asize;
//	g = (float *)malloc(bsize);
//	
//    int tmp = 0;
//    for (unsigned i = 0; i < h; i++) {
//        for (unsigned j = 0; j < w; j++) {
//            unsigned char ip = input.at<unsigned char>(i, j);
//            tmp = j * h + i; // convert to column major
//            g[tmp] = (float) ip;
//        }
//    }
//   array af_temp(h,w,g);
//   output = af_temp;
//   free(g);
//}



//void applyCUDAFilters(gpu_context_t *ctx, CPUImageFilter& img){
//	
//	//Set Mirroring Horizontal/Vertical
//    //if(bVerticalMirror || bHorizontalMirror) img.mirror(bVerticalMirror, bHorizontalMirror);
//
//    //if(!bMiniMode) grayImg = img; //for drawing
//    //Dynamic background with learn rate
//  //  if(bDynamicBG){
//  //      floatBgImg.addWeighted( img, fLearnRate);
//		////grayBg = floatBgImg;  // not yet implemented
//		// cvConvertScale( floatBgImg.getCvImage(), grayBg.getCvImage(), 255.0f/65535.0f, 0 );       
//		// grayBg.flagImageChanged();
//  //  }
//
//    //recapature the background until image/camera is fully exposed
//    if((ofGetElapsedTimeMillis() - exposureStartTime) < CAMERA_EXPOSURE_TIME) bLearnBakground = true;
//
//    //Capture full background
//	if (bLearnBakground == true){
//        floatBgImg = img;
//		////grayBg = floatBgImg;  // not yet implemented
//		cvConvertScale( floatBgImg.getCvImage(), grayBg.getCvImage(), 255.0f/65535.0f, 0 );       
//		grayBg.flagImageChanged();
//        bLearnBakground = false;
//    }
//    
//	
//    //gpu::getCudaEnabledDeviceCount();
//	//gpu::GpuMat *gpu_im = new gpu::GpuMat(img.width, img.height, sizeof(unsigned char));
//	//gpu_im->upload(temp_im);
//	
//	IplImage *imdata = img.getCvImage();
//	Mat temp_im(imdata);
//	array af_fg = array(temp_im.cols, temp_im.rows, f32);
//	greymat_to_array(temp_im, af_fg);
//	IplImage *bgdata = grayBg.getCvImage();
//	Mat temp_bg(bgdata);
//	array af_bg = array(temp_im.cols, temp_im.rows, f32);
//	greymat_to_array(temp_bg, af_bg);
//	array af_sub = af_bg;
//	if(bTrackDark)
//		array af_sub = af_bg - af_fg;
//	else
//		array af_sub = af_fg - af_bg;
//	// graphics window
//    figure(0, 0, 1366, 768);
//    palette("gray");
//	// image operations
//	subfigure(1, 2, 1);  af::imgplot(af_fg);	title("source image");
//	subfigure(1, 2, 2);  af::imgplot(af_sub);   title("Background sub");
//
//	//Background Subtraction
//    //img.absDiff(grayBg, img); 		
//	//if(bTrackDark)
//	//	cvSub(grayBg.getCvImage(), img.getCvImage(), img.getCvImage());
//	//else
//	//	cvSub(img.getCvImage(), grayBg.getCvImage(), img.getCvImage());
//	//// Need to copy the results to the GPU
//	//IplImage *tmp = img.getCvImage(); 
//	//if(gpu_set_input( ctx, (unsigned char *)tmp->imageData) != GPU_OK) {
//	//	GPU_ERROR("Unable to set context buffer");
//	//	return;
//	//}
//	img.flagImageChanged();
//
//
//	
//	if(bSmooth){//Smooth
//        img.blur((smooth * 2) + 1); //needs to be an odd number
//        if(!bMiniMode)
//        subtractBg = img; //for drawing
//    }
//
//    if(bHighpass){//HighPass
//        img.cuda_highpass(ctx, highpassBlur, highpassNoise);
//        if(!bMiniMode)
//        highpassImg = img; //for drawing
//    }
//
//    if(bAmplify){//Amplify
//        img.cuda_amplify(ctx, img, highpassAmp);
//        if(!bMiniMode)
//        amplifyImg = img; //for drawing
//    }
//
//	if (bDynamicTH)
//		img.adaptiveThreshold(threshold, -threshSize);
//	else
//		img.threshold(threshold); //Threshold
//
//    if(!bMiniMode)
//    grayDiff = img; //for drawing
//	
//	//delete gpu_im;
//}



/****************************************************************
 *	GPU Filters
 ****************************************************************/
    void applyGPUFilters(){

        //recapature the background until image/camera is fully exposed
        if((ofGetElapsedTimeMillis() - exposureStartTime) < CAMERA_EXPOSURE_TIME) bLearnBakground = true;

        if (bLearnBakground == true){

            gpuBGTex = copyFilter->apply(gpuSourceTex, gpuBGTex);
            bLearnBakground = false;
        }

        GLuint processedTex;

        processedTex = subtractFilter->apply(gpuSourceTex, gpuBGTex);

        if(bSmooth){//Smooth
            gaussHFilter->parameters["kernel_size"]->value = (float)smooth;
            gaussVFilter->parameters["kernel_size"]->value = (float)smooth;
            processedTex = gaussHFilter->apply(processedTex);
            processedTex = gaussVFilter->apply(processedTex);
        }

        if(bHighpass){//Highpass
            gaussHFilter2->parameters["kernel_size"]->value = (float)highpassBlur;
            gaussVFilter2->parameters["kernel_size"]->value = (float)highpassBlur;
            processedTex = gaussHFilter2->apply(processedTex);
            processedTex = gaussVFilter2->apply(processedTex);

            if(bSmooth)
                processedTex = subtractFilter2->apply(gaussVFilter->output_texture, processedTex);
            else
                processedTex = subtractFilter2->apply(subtractFilter->output_texture, processedTex);
        }

        if(bAmplify)
		{}//amplify

        threshFilter->parameters["Threshold"]->value = (float)threshold / 255.0; //threshold
        processedTex = threshFilter->apply(processedTex);

        //convert to grayscale so readback is faster. maybe do this from the start?
        grayScale->apply(processedTex);

        //until the rest of the pipeline is fixed well just download the preprocessing result from the gpu and use that for the blob detection
        //TODO: make this part not super slow ;)
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, threshFilter->output_buffer);
        glReadPixels(0,0,camWidth, camHeight, GL_LUMINANCE, GL_UNSIGNED_BYTE, gpuReadBackBuffer);
        gpuReadBackImageGS.setFromPixels(gpuReadBackBuffer, camWidth, camHeight);
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    }

/****************************************************************
 *	Draw Filter Images
 ****************************************************************/
    void draw()
    {
		// SEE ofxNCoreVision: MAIN_TOP_OFFSET
		if (drawAllData)
		{
			grayImg.draw(250, 25, 326, 246);
			grayDiff.draw(250+335, 25, 326, 246);
		}
		//
        floatBgImg.draw(250+137*0, 365, 129, 96);
        subtractBg.draw(250+137*1, 365, 129, 96);
        highpassImg.draw(250+137*2, 365, 129, 96);
		amplifyImg.draw(250+137*3, 365, 129, 96);
    }

    void drawGPU()
    {
	
        drawGLTexture(500, 700, 320, 240, gpuSourceTex);
        drawGLTexture(500, 800, 128, 96, gpuBGTex);
        gaussVFilter->drawOutputTexture(500, 900, 128, 96);
        subtractFilter2->drawOutputTexture(500, 1000, 128, 96);
        threshFilter->drawOutputTexture(500, 1100, 128, 96); //this should be amplify filter but we don't have one yet
        gpuReadBackImageGS.draw(500, 1200, 320, 240);
			/**/
    }
};
#endif
