#ifndef B_H
#define B_H

#include <cv.h>
#include <cxcore.h>
#include <highgui.h>
#include <iostream>
#pragma once
#using <mscorlib.dll>
using namespace System;

using namespace std;



public class Buttacharyya
{
public:
	//void mouseHandler(int event, int x, int y, int flags, void* param)
	//  {
	//    switch(event){
	//      case CV_EVENT_LBUTTONDOWN:
	//		IplImage* img = (IplImage*) param;
	//		cout << "x  = ";
	//		cout << x;
	//		cout << " y = ";
	//		cout << y;
	//		//cout << " == ";
	//		//cout << (int) (unsigned char) img->imageData[y*img->widthStep + x*];
	//		cout << "\n";
	//        break;
	//
	//    }
	//  }

	static void normalize(CvMat* imgIn, CvMat* imgOut)
	{
		double minVal, maxVal;
		double value;
		cvMinMaxLoc(imgIn,&minVal,&maxVal);
		/*cout << "max = " << maxVal <<endl;
		cout << "min = " << minVal <<endl;*/
		for (int i=0; i < imgIn->width; i++)
			for (int j=0; j < imgIn->height; j++)
			{
				double v = cvmGet(imgIn,j,i);
				value = (v-minVal)*255/(maxVal- minVal);
				cvmSet(imgOut,j,i,value);
			}
	}

	//returns an image of the desired projection of the histogram
	static void OutputImageHist3DProjection(CvHistogram *hist,IplImage* OutputImage[],int* projection[],int binsize)
	{
		float maxR=-1, maxG=-1, maxB=-1;
		//cvGetMinMaxHistValue(hist,&min,&max);
		cvZero(OutputImage[0]);
		cvZero(OutputImage[1]);
		cvZero(OutputImage[2]);
		//first round to find the max
		//the 0,0,0 is omitted because it's the background of the faces
		for(int i=0; i <binsize; i++)
		{
			float bin_val[3]={0,0,0};
			for (int j=0; j < binsize; j++)
				for (int k=0; k < binsize; k++)
				{
					if (j==0 && k == 0 && i==0) continue; //skip all black
					bin_val[0] += cvQueryHistValue_3D(hist,i,j,k); 
					bin_val[1] += cvQueryHistValue_3D(hist,j,i,k);
					bin_val[2] += cvQueryHistValue_3D(hist,k,j,i);
				}
			if (maxB < bin_val[0]) maxB = bin_val[0];
			if (maxG < bin_val[1]) maxG = bin_val[1];
			if (maxR < bin_val[2]) maxR = bin_val[2];
		}
		/*cout << "Max Red = " << maxR <<endl;
		cout << "Max Green = " << maxG <<endl;
		cout << "Max Blue = " << maxB <<endl;*/
		for (int i=0; i <binsize; i++)
		{
			float bin_val[3]={0,0,0};
			for (int j=0; j < binsize; j++)
				for (int k=0; k < binsize; k++)
				{
					if (j==0 && k == 0 && i==0) continue; //skip all black
					bin_val[0] += cvQueryHistValue_3D(hist,i,j,k); 
					bin_val[1] += cvQueryHistValue_3D(hist,j,i,k);
					bin_val[2] += cvQueryHistValue_3D(hist,k,j,i);
				}
			projection[0][i] = bin_val[0];
			projection[1][i] = bin_val[1];
			projection[2][i] = bin_val[2];
			float bin_height[3];
			bin_height[0] = bin_val[0]*OutputImage[0]->height/maxB;
			bin_height[1] = bin_val[1]*OutputImage[1]->height/maxG;
			bin_height[2] = bin_val[2]*OutputImage[2]->height/maxR;
			CvScalar colour[3];
			colour[0]= cvScalar(0,140,255);
			colour[1]= cvScalar(0,255,0);
			colour[2]= cvScalar(0,0,255);
			cvRectangle(OutputImage[0],cvPoint((OutputImage[0]->width/binsize)*i,OutputImage[0]->height),cvPoint((OutputImage[0]->width/binsize)*(i+1) - 1,OutputImage[0]->height - bin_height[0]),colour[0],1);
			cvRectangle(OutputImage[1],cvPoint((OutputImage[1]->width/binsize)*i,OutputImage[1]->height),cvPoint((OutputImage[1]->width/binsize)*(i+1) - 1,OutputImage[1]->height - bin_height[1]),colour[1],1);
			cvRectangle(OutputImage[2],cvPoint((OutputImage[2]->width/binsize)*i,OutputImage[2]->height),cvPoint((OutputImage[2]->width/binsize)*(i+1) - 1,OutputImage[2]->height - bin_height[2]),colour[2],1);

		}
	}

	static void CopyMatToImage(CvMat* mat, IplImage* img)
	{
		for(int i=0; i < img->height; i++)
			for(int j=0; j < img->width; j++)
			{
				double v = cvmGet(mat,i,j);
				char value = (unsigned char) (int) v;
				img->imageData[i*img->widthStep + j] = (int) (unsigned char) value;
			}


	}

	static void CopyImageToMat(IplImage* img, CvMat* mat)
	{
		for(int i=0; i < img->height; i++)
			for(int j=0; j < img->width; j++)
			{
				double v = (int)(unsigned char) img->imageData[i*img->widthStep+j];
				cvmSet(mat,i,j,v);
			}
	}

	static void DisplayMat(char* windowName, CvMat* mat, int depth, int nCh)
	{
		cvNamedWindow(windowName);
		IplImage* displayImage = cvCreateImage(cvSize(mat->width,mat->height),depth, nCh);
		CvMat* Normalized = cvCreateMat(mat->width,mat->height,mat->type);
		normalize(mat,Normalized);
		CopyMatToImage(Normalized,displayImage);
		IplImage* resizedImage = cvCreateImage(cvSize(displayImage->width*4,displayImage->height*4),depth,nCh);
		cvResize(displayImage,resizedImage);
		cvNamedWindow(windowName);
		cvShowImage(windowName,resizedImage);
		//cvSetMouseCallback(windowName,mouseHandler,displayImage);
		//char* tempChar = new char[100];
		//strcpy(tempChar,windowName);
		//strcat(tempChar,".png");
		//cvSaveImage(tempChar,displayImage);
		cvReleaseImage(&displayImage);
		cvReleaseMat(&Normalized);
		cvReleaseImage(&resizedImage);
	}

	static CvMat* unitCircleNormalize(IplImage* Image,CvPoint* leftup, int w, int h, int min_w_h)
	{
		CvMat* SubImage = cvCreateMat(min_w_h,min_w_h,CV_32F);
		IplImage* temp  = cvCreateImage(cvGetSize(SubImage),Image->depth,Image->nChannels);
		cvSetImageROI( Image, cvRect(leftup->x, leftup->y,w,h) );
		cvResize(Image,temp );//CV_INTER_NN
		CopyImageToMat(temp,SubImage);
		cvResetImageROI( Image );
		cvReleaseImage(&temp);
		return SubImage;
	}




	static CvMat* createEpanechnikov(int d, double h)
	{
		CvMat* kernel = cvCreateMat(d,d,CV_32F);
		double k,distance,radius2;
		int ii,jj;
		int radius = cvFloor(d*h/2);
		int center =  cvFloor(d/2);
		for (int i=0; i<d; i++)
			for (int j=0; j<d; j++)
			{
				/*cvmSet(kernel,i,j,1);*/

				ii = (i-center);
				jj = (j-center);
				radius2 = radius*radius;
				distance = (ii*ii + jj*jj)/radius2;
				if (distance >= 1) //outside the kernel
					cvmSet(kernel,i,j,0);
				else
				{
					k = (1- distance);
					cvmSet(kernel,i,j,k);
				}
					
			}
			return kernel;
	}

	static void minDimention(CvPoint* pnt1, CvPoint* pnt2, int &min_w_h, int &w, int &h)
	{
		w = pnt2->x - pnt1->x + 1;
		h = pnt2->y - pnt1->y + 1;
		if (w <h)
			min_w_h = w;
		else
			min_w_h  =h;
	}

};
#endif
