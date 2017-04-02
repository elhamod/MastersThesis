
#ifndef B_A
#define B_A

#include <iostream>
#include "cv.h"
#include "cxcore.h"
#include "highgui.h"

#pragma once
#using <mscorlib.dll>
using namespace System;


using namespace std;
//
IplImage* returnedImage =0;
IplImage* tempImage = 0;

void readFrame(char* fileLocationNamePrefix, int frameNumber, int numberOfDigits, char* extension, IplImage*& returnedImage ) //".jpg"
//IplImage* readFrame(char* fileLocationNamePrefix, int frameNumber, int numberOfDigits, char* extension) //".jpg"
{
	char buffer[200];

	if (returnedImage) cvReleaseImage(&returnedImage);
	
	char* oldChar = new char[200];
	char* newChar = new char[200];
	char* prefix = new char[200];
	strcpy(prefix,fileLocationNamePrefix);
	strcpy(oldChar,itoa(frameNumber,buffer,10));
	strcat(oldChar,extension);
	for (int i = numberOfDigits; i> strlen(itoa(frameNumber,buffer,10));i--)
	{
		strcpy(newChar,"0");
		strcat(newChar,oldChar);
		strcpy(oldChar,newChar);
	}
	strcat(prefix,oldChar);
	tempImage = returnedImage;
	returnedImage = cvLoadImage(prefix);
	cvReleaseImage(&tempImage);
	delete oldChar;
	delete newChar;
	delete prefix;

	//return returnedImage;




}

#endif
