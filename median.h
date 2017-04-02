#ifndef B_V
#define B_V

#include <cv.h>
#include <cvaux.h>
#include <highgui.h>
//#include "BGmodel.h"
#pragma once
#using <mscorlib.dll>
using namespace System;




double find_median(int ,double* ); //generic function to calculate the median of an array
double spacial_median(IplImage* ); //calculates the spacial median of all pixels in a frame and (3 medians for 3 colors)
int find_n(int ,double* ,int); //finds the nth element, and returns its position

#endif
