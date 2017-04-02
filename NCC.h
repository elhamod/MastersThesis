#ifndef NCC_h
#define NCC_h


#include <cxcore.h> 
#include <cv.h> 
#include <highgui.h> 


void NCC(IplImage* A, IplImage* B, IplImage* output)
{
	cvMulSpectrums( B, A, output, 0  );
}

#endif