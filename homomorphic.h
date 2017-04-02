#ifndef h_h
#define h_h

#include <cxcore.h> 
#include <cv.h> 
#include <highgui.h> 



IplImage* homomorphic_filter(IplImage*, IplImage*,bool );
IplImage* createFilter(int);
IplImage* createButterwarthFilter(int,float,float,float,float);
void ShowArr(IplImage* , char* );
void NormalizeDraw64Image(IplImage*, char*);


#endif