/*
*  CPUImageFilter.h
*  
*
*  Created on 2/2/09.
*  Copyright 2009 NUI Group. All rights reserved.
*
*/

#ifndef CPUImageFilter_H
#define CPUImageFilter_H

#include "ofxCvGrayscaleImage.h"
//#include "API/gpu_filter_api.h"

class CPUImageFilter : public ofxCvGrayscaleImage {

  public:

    CPUImageFilter(){image_buffer = NULL; uimage_buffer = NULL;};

    void operator = ( unsigned char* _pixels );
    void operator = ( const ofxCvGrayscaleImage& mom );
    void operator = ( const ofxCvColorImage& mom );
    void operator = ( const ofxCvFloatImage& mom );

	//amplifies signal
	void amplify( CPUImageFilter& mom, float level );
	//picks out light spots from image
	void highpass(float blur1, float blur2 );
	/*
	// Versions utilizing CUDA computations
	void cuda_amplify (gpu_context_t *ctx, CPUImageFilter& mom, float level );
	void cuda_highpass(gpu_context_t *ctx, float blur1, float blur2);
	*/
  protected:
	char *image_buffer;
	unsigned char *uimage_buffer;

};

#endif

