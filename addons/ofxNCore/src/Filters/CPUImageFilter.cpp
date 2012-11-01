/*
*  CPUImageFilter.h
*  
*
*  Created on 2/2/09.
*  Copyright 2009 NUI Group. All rights reserved.
*
*/

#include "CPUImageFilter.h"
#include "ofxCvColorImage.h"
#include "ofxCvGrayscaleImage.h"
#include "ofxCvFloatImage.h"
#include "cuda.h"
#include "../../cudaFilter/Amplify/gpu_amplify.h"
#include "../../cudaFilter/GaussBlurTex/gpu_blur_tex.h"
#include "../../cudaFilter/BgSub/gpu_sub.h"
#include "../../cudaFilter/API/gpu_filter_api.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

//--------------------------------------------------------------------------------
void CPUImageFilter::amplify ( CPUImageFilter& mom, float level ) {

	float scalef = level / 128.0f;
	//gpu_error cudaerr = gpu_amplify( 
	cvMul( mom.getCvImage(), mom.getCvImage(), cvImageTemp, scalef );
	swapTemp();
	flagImageChanged();
}

void CPUImageFilter::cuda_amplify (gpu_context_t *ctx, CPUImageFilter& mom, float level ) {
	gpu_error_t cudaErr;
	float scalef = level / 128.0f;

	cudaErr = gpu_amplify( ctx, scalef);
	if (cudaErr != GPU_OK) {
		GPU_ERROR("Unable to amplify image using CUDA");
		return;
	}
	if (gpu_get_output(ctx, &uimage_buffer) != GPU_OK) {
		GPU_ERROR("Unable to get GPU output buffer after amplify");
		return;
	}
	memcpy((char *)cvImageTemp->imageData, (void *)uimage_buffer, width*height);
	//cvImageTemp->imageData = static_cast<char *>(uimage_buffer);
	swapTemp();
	flagImageChanged();
}
void CPUImageFilter::highpass ( float blur1, float blur2 ) {
	
	//Blur Original Image
	if(blur1 > 0)
	cvSmooth( cvImage, cvImageTemp, CV_BLUR , (blur1 * 2) + 1);

	//Original Image - Blur Image = Highpass Image
	cvSub( cvImage, cvImageTemp, cvImageTemp );

	//Blur Highpass to remove noise
	if(blur2 > 0)
	cvSmooth( cvImageTemp, cvImageTemp, CV_BLUR , (blur2 * 2) + 1);

	swapTemp();
	flagImageChanged();
}

void CPUImageFilter::cuda_highpass (gpu_context_t *ctx, float blur1, float blur2)
{
	gpu_error_t cudaErr;
	
	////////////// calculating kernel //////////////
	//float sum = 0;
	//float sig = blur1;
	//float KERNEL_LENGTH = (2*sig)+1;
	//float *tempKernel = (float *) malloc(sizeof(float)*KERNEL_LENGTH*2);
	//float dist = 0;
 //   for(int i = 0; i <KERNEL_LENGTH*2; i++)
 //   {
 //   	dist = (float)i;
 //   	//tempKernel[i] = expf(- dist * dist / 2); //exponential decay, e^(-dist)
	//	tempKernel[i] = (1/(sig*sqrtf(2*M_PI)))*(expf(-.5*pow(dist/sig, 2))); //guassian kernel
 //   	sum += tempKernel[i];
 //   }
 //   for(int i = 0; i < KERNEL_LENGTH*2; i++)
 //       tempKernel[i] /= tempKernel[0];            


	//Blur Original Image
	if(blur1 >= 1) {
		if(gpu_blur(ctx, (int)blur1) != GPU_OK){
			GPU_ERROR("Unable to blur the image");
		}
	}
	// Subtract the background
	//unsigned char *imgdata_ref = static_cast<unsigned char *>(cvImage->imageData);
	unsigned char *imgdata_ref = (unsigned char *)cvImage->imageData;
	if (gpu_sub(ctx, imgdata_ref) != GPU_OK)
		GPU_ERROR("Unable to subtract background for highpass filtering");
	
	// Blur Highpass to remove noise
	if(blur1 >= 1) {
		if(gpu_blur(ctx, (int)blur2) != GPU_OK){
			GPU_ERROR("Unable to blur the image");
		}
	}
	
	// Now get output from the GPU
	if (gpu_get_output(ctx, &uimage_buffer) != GPU_OK) {
		GPU_ERROR("Unable to get GPU output buffer after filtering");
		return;
	}
	//cvImageTemp->imageData = static_cast<char *>(uimage_buffer);
	//cvImageTemp->imageData = (char *)uimage_buffer;
	
	memcpy((char *)cvImageTemp->imageData, (char *)uimage_buffer, width*height);
	swapTemp();
	flagImageChanged();
}

//--------------------------------------------------------------------------------
void CPUImageFilter::operator =	( unsigned char* _pixels ) {
    setFromPixels( _pixels, width, height );
	image_buffer = (char *) malloc(width*height*sizeof(char));
	uimage_buffer = (unsigned char *) malloc(width*height*sizeof(unsigned char));
}

//--------------------------------------------------------------------------------
void CPUImageFilter::operator = ( const ofxCvGrayscaleImage& _mom ) {
    if(this != &_mom) {  //check for self-assignment
        // cast non-const,  no worries, we will reverse any chages
        ofxCvGrayscaleImage& mom = const_cast<ofxCvGrayscaleImage&>(_mom); 
            
        if( matchingROI(getROI(), mom.getROI()) ) {
            cvCopy( mom.getCvImage(), cvImage, 0 );           
            flagImageChanged();
        } else {
            ofLog(OF_LOG_ERROR, "in =, ROI mismatch");
        }
    } else {
        ofLog(OF_LOG_WARNING, "in =, you are assigning a ofxCvGrayscaleImage to itself");
    }
	image_buffer = (char *) malloc(width*height*sizeof(char));
	uimage_buffer = (unsigned char *) malloc(width*height*sizeof(unsigned char));
}

//--------------------------------------------------------------------------------
void CPUImageFilter::operator = ( const ofxCvColorImage& _mom ) {
    // cast non-const,  no worries, we will reverse any chages
    ofxCvColorImage& mom = const_cast<ofxCvColorImage&>(_mom); 
	if( matchingROI(getROI(), mom.getROI()) ) {
		cvCvtColor( mom.getCvImage(), cvImage, CV_RGB2GRAY );
        flagImageChanged();
	} else {
        ofLog(OF_LOG_ERROR, "in =, ROI mismatch");
	}
	image_buffer = (char *) malloc(width*height*sizeof(char));
	uimage_buffer = (unsigned char *) malloc(width*height*sizeof(unsigned char));
}

//--------------------------------------------------------------------------------
void CPUImageFilter::operator = ( const ofxCvFloatImage& _mom ) {
    // cast non-const,  no worries, we will reverse any chages
    ofxCvFloatImage& mom = const_cast<ofxCvFloatImage&>(_mom); 
	if( matchingROI(getROI(), mom.getROI()) ) {
		//cvConvertScale( mom.getCvImage(), cvImage, 1.0f, 0);
        cvConvert( mom.getCvImage(), cvImage );        
        flagImageChanged();
	} else {
        ofLog(OF_LOG_ERROR, "in =, ROI mismatch");
	}
	image_buffer = (char *) malloc(width*height*sizeof(char));
	uimage_buffer = (unsigned char *) malloc(width*height*sizeof(unsigned char));
}