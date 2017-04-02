#ifndef B_N
#define B_N

//#include "stdafx.h"
#include <cv.h>
#include <cxcore.h>
#include <highgui.h>
#include <math.h>
#include "Buttacharyya.h"
#include <iostream>
#pragma once
#using <mscorlib.dll>
using namespace System;

using namespace std;



public class ButtacharyyaTracker
{
private:
	 static const int HistSizeab =16;
	 static const int HistSizeL =16;
	 
	 //static const float maxSpeed =5; //pixels per frame

	 static const int max_number_of_objects = 10;

public: 

	//REMOVE
	static void TEMPmouseHandler(int event, int x, int y, int flags, void* param) 	 /*mouse handler for clicking the video and getting the BG model*/
	{
		if (event == CV_EVENT_LBUTTONDOWN)
		{
			IplImage* tmpImage = (IplImage*) param;
			char bufferTmp1[10];
			int value  = (int) (unsigned char) tmpImage->imageData[x*3+y*tmpImage->widthStep];
			cvPutText(tmpImage,itoa(value,bufferTmp1,10),cvPoint(30,30),&cvFont(5,2),cvScalar(255,255,255));
		}
	}
	
	static CvHistogram* ConstructHist(int min_w_h, CvMat* Channel1, CvMat* Channel2, CvMat* Channel3,CvMat* BGmask, int depth,double KernelDim )
	{

		CvMat* kernel = Buttacharyya::createEpanechnikov(min_w_h,KernelDim);
		//DisplayMat("Kernel",kernel,depth, 1);
		//multiply the kernel with the ROI
		CvMat* QB = cvCreateMat(min_w_h,min_w_h,CV_32F);
		CvMat* QG = cvCreateMat(min_w_h,min_w_h,CV_32F);
		CvMat* QR = cvCreateMat(min_w_h,min_w_h,CV_32F);
		//CvMat* Q = cvCreateMat(min_w_h,min_w_h*img->nChannels,CV_32F);
	/*	cvMul(kernel,Channel1,QB);
		cvMul(kernel,Channel2,QG);
		cvMul(kernel,Channel3,QR);*/
		cvCopy(Channel1,QB);
		cvCopy(Channel2,QG);
		cvCopy(Channel3,QR);
		//cvMerge(Q1,Q2,Q3,0,Q);
		//DisplayMat("Kernel*SubImage",Q,img->depth, img->nChannels);

		//construct the histogram qu
		IplImage* QBImage = cvCreateImage(cvGetSize(QB),depth,1);
		IplImage* QGImage = cvCreateImage(cvGetSize(QG),depth,1);
		IplImage* QRImage = cvCreateImage(cvGetSize(QR),depth,1);
		IplImage* QImage = cvCreateImage(cvGetSize(QR),depth,3);
		IplImage* BGImage;
		if (BGmask) BGImage = cvCreateImage(cvGetSize(BGmask),depth,1);
		Buttacharyya::CopyMatToImage(QB,QBImage);
		Buttacharyya::CopyMatToImage(QG,QGImage);
		Buttacharyya::CopyMatToImage(QR,QRImage);
		if (BGmask) Buttacharyya::CopyMatToImage(BGmask,BGImage);
		/*cvMerge(QBImage,QGImage,QRImage,0,QImage);
		cvNamedWindow("Multiplied",CV_WINDOW_AUTOSIZE);
		cvShowImage("Multiplied",QImage);*/
		IplImage* planes[] = {QBImage, QGImage, QRImage};
		int hist_size[] = {HistSizeL, HistSizeab, HistSizeab};
		float range[] = { 0, 255 };
		float* ranges[] = { range, range, range };
		CvHistogram* hist = cvCreateHist(3,hist_size,CV_HIST_ARRAY,ranges);
		//cvCalcHist( planes, hist,0,BGmask ? BGImage : 0);

		//Calculating Weighted Histogram
		float* histogram_array = new float[HistSizeL*HistSizeab*HistSizeab];
		for (int i = 0; i < HistSizeL*HistSizeab*HistSizeab; i++)
			histogram_array[i] = 0;
		int indx, indx1,indx2,indx3;
		float Bval, Gval, Rval;
		for (int i = 0; i <QBImage->width; i++)
			for (int j = 0; j < QBImage->height; j++)
			{
				if (!BGmask || (int) (unsigned char) BGImage->imageData[j*QBImage->widthStep+i] != 0 )
				{
					Bval = (int) (unsigned char) QBImage->imageData[j*QBImage->widthStep+i];
					Gval = (int) (unsigned char) QGImage->imageData[j*QBImage->widthStep+i];
					Rval = (int) (unsigned char) QRImage->imageData[j*QBImage->widthStep+i];
					indx1 = cvFloor(Bval*((float)HistSizeL/256));
					indx2 = cvFloor(Gval*((float)HistSizeab/256));
					indx3 = cvFloor(Rval*((float)HistSizeab/256));
					indx =  indx1 +HistSizeL*indx2 + HistSizeL*HistSizeab*indx3;
					histogram_array[indx] += cvmGet(kernel,j,i);
				}
			}

		cvMakeHistHeaderForArray(3,hist_size,hist,histogram_array,ranges);
		cvNormalizeHist(hist,1);

		cvReleaseMat(&QB);
		cvReleaseMat(&QG);
		cvReleaseMat(&QR);
		cvReleaseImage(&QBImage);
		cvReleaseImage(&QGImage);
		cvReleaseImage(&QRImage);
		cvReleaseImage(&QImage);
		if (BGmask) cvReleaseImage(&BGImage);
		cvReleaseMat(&kernel);
		

		return hist;
	}
	
	static void Show3DHistProjections(CvHistogram* Hist, int size, char* name)
	{
		IplImage* hist_img[3];
		int* projections[3];
		projections[0] = new int[size];
		projections[1] = new int[size];
		projections[2] = new int[size];
		hist_img[0] = cvCreateImage(cvSize(1024,300),8,3);
		hist_img[1] = cvCreateImage(cvSize(1024,300),8,3);
		hist_img[2] = cvCreateImage(cvSize(1024,300),8,3);
		Buttacharyya::OutputImageHist3DProjection(Hist,hist_img,projections,size);
		char* name1 = new char[100];
		strcpy(name1,"BlueWindow ");
		char* name2 = new char[100];
		strcpy(name2,"GreenWindow ");
		char* name3 = new char[100];
		strcpy(name3,"RedWindow ");
		strcat(name1,name);
		strcat(name2,name);
		strcat(name3,name);
		cvNamedWindow(name1,CV_WINDOW_AUTOSIZE);
		cvShowImage(name1,hist_img[0]);
		cvNamedWindow(name2,CV_WINDOW_AUTOSIZE);
		cvShowImage(name2,hist_img[1]);
		cvNamedWindow(name3,CV_WINDOW_AUTOSIZE);
		cvShowImage(name3,hist_img[2]);

		for (int i=0; i < 3; i++)
			delete [] projections[i];
		delete [] name1;
		delete [] name2;
		delete [] name3;
		cvReleaseImage(&hist_img[0]);
		cvReleaseImage(&hist_img[1]);
		cvReleaseImage(&hist_img[2]);

		
	}




	//ObjectPnt : 0 == upper left, 1 == lower right
	//blobs: tracker blobs.
	//kernel_h: > 1
	static void Track(CvBlobSeq* blobs, int currentNumOfObjects,IplImage* img,IplImage* BGimage, CvHistogram** quHist,int frameNumber, int* time, float kernel_h)
	{
		float* neighborhoodSize;//(n) in section 2.1

		

		int ID ;

		//set neighborhood sizes
		/*if (frameNumber !=0) 
			delete[] neighborhoodSize;*/
		neighborhoodSize = new float[max_number_of_objects];
			
			

		//get subimage and normalize it

		IplImage* imgBChannel = cvCreateImage(cvGetSize(img),img->depth,1);
		IplImage* imgGChannel = cvCreateImage(cvGetSize(img),img->depth,1);
		IplImage* imgRChannel = cvCreateImage(cvGetSize(img),img->depth,1);
		cvSplit(img,imgBChannel,imgGChannel,imgRChannel,0);
		/*CvMat* SubImageBChannel = 0;
		CvMat* SubImageGChannel = 0;
		CvMat* SubImageRChannel = 0;*/
		/*CvMat* SubImage= 0;*/
		/*cvNamedWindow("in");
		cvShowImage("in",img);*/

		int min_w_h,max_w_h,w[max_number_of_objects], h[max_number_of_objects];
		for (int k=0; k <currentNumOfObjects; k++)
		{
			h[k] = blobs->GetBlob(k)->h;
			w[k] = blobs->GetBlob(k)->w;
			if ( h[k] > w[k])
			{
				min_w_h = w[k];
				max_w_h = h[k];
			}
			else
			{
				min_w_h = h[k];
				max_w_h = w[k];
			}

			neighborhoodSize[k] = min_w_h;
			//neighborhoodSize[k] = 100;
			//minDimention(ObjectPnt[0][k],ObjectPnt[1][k],min_w_h,w[k],h[k]);

		}
		


		CvMat* SubImageChannel[3][max_number_of_objects];
		CvMat* BGSubImage[max_number_of_objects];
	

		//SubImage = cvCreateMat(min_w_h,min_w_h*img->nChannels,CV_32F);
		//cvMerge(SubImageBChannel,SubImageGChannel,SubImageRChannel,0,SubImage);
		//DisplayMat("Subimage",SubImage,img->depth, img->nChannels);

		

		for (int k=0; k <currentNumOfObjects; k++)
		{

			ID = blobs->GetBlob(k)->ID;
			//if (time[ID] == glbl_var.numberOfReadFrame) // just construct the histogram
			if (!quHist[ID])
			{
				CvPoint* pnt = &cvPoint(blobs->GetBlob(k)->x,blobs->GetBlob(k)->y);
				/*int w = blobs->GetBlob(i)->w;
				int h = blobs->GetBlob(i)->h;*/
				SubImageChannel[0][k] = Buttacharyya::unitCircleNormalize(imgBChannel,pnt,w[k],h[k],neighborhoodSize[k]);
				SubImageChannel[1][k] = Buttacharyya::unitCircleNormalize(imgGChannel,pnt,w[k],h[k],neighborhoodSize[k]);
				SubImageChannel[2][k] = Buttacharyya::unitCircleNormalize(imgRChannel,pnt,w[k],h[k],neighborhoodSize[k]);
				BGSubImage[k] = Buttacharyya::unitCircleNormalize(BGimage,pnt,w[k],h[k],neighborhoodSize[k]);

				//quHist[ID] = ConstructHist(neighborhoodSize[k], SubImageChannel[0][k], SubImageChannel[1][k], SubImageChannel[2][k],0, img->depth, 1 );
				quHist[ID] = ConstructHist(neighborhoodSize[k], SubImageChannel[0][k], SubImageChannel[1][k], SubImageChannel[2][k],BGSubImage[k], img->depth, 1 );

				////REMOVE a snapshot of the object
				//IplImage* snapshot = cvCreateImage(cvSize(*tmpW,*tmpH),img->depth,img->nChannels);
				//cvSetImageROI(img,cvRect(*tmpX,*tmpY,*tmpW,*tmpH));
				//cvCopyImage(img,snapshot);
				//cvResetImageROI(img);
				//cvNamedWindow("subjectTrained");
				//cvShowImage("subjectTrained",snapshot);
				//cvReleaseImage(&snapshot);


				/*Show3DHistProjections(quHist[0], HistSize, "qu");
				cvWaitKey(10);*/

				//TEST
				/*IplImage* testImg = cvCloneImage(img);
				for (int k=0; k < currentNumOfObjects; k++)
				cvRectangle(testImg,*(ObjectPnt[0][k]),*(ObjectPnt[1][k]),cvScalar(0,0,255));
				cvNamedWindow("TEST");
				cvShowImage("TEST",testImg);
				cvWaitKey();*/

				//calculate pu(y0)
				/*CvHistogram* puy0Hist = ConstructHist(neighborhoodSize, SubImageBChannel, SubImageGChannel, SubImageRChannel, img->depth, kernel_h );
				Show3DHistProjections(puy0Hist, HistSize, "puy0");
				cvWaitKey();*/
				//Show3DHistProjections(quHist[ID], HistSizeab, "qu");
			}
			else //after constructing the histogram
			{
				//CvPoint* pnt = &cvPoint((kernel_h-1)*0.5*blobs->GetBlob(k)->x,(kernel_h-1)*0.5*blobs->GetBlob(k)->y);
				////make sure it's not outside the boundaries of the image
				//if (pnt->x <0 )pnt->x = 0;
				//if (pnt->y <0 ) pnt->y = 0;
				//if (pnt->x + pnt->w > img->width-1 ) pnt->w = img->width-1 - pnt->x;
				//if (pnt->y + pnt->h > img->height-1 ) pnt->y = img->height-1 - pnt->y;

				CvScalar y0 = cvScalar(0,0); //initialvalue of y0
				//CvMat* wi = cvCreateMat(neighborhoodSize[k]*kernel_h,neighborhoodSize[k]*kernel_h,SubImageChannel[0][0]->type);
				double numerator, denominator,tmp;
				//int start = cvFloor(neighborhoodSize[k]*(1-kernel_h)/2);
				//int end = start + cvFloor(neighborhoodSize[k]*kernel_h); //exclusive
				int start = 0;
				int end = cvFloor(neighborhoodSize[k]); //exclusive
				int center = cvFloor((end+start)/2);
				CvScalar bxi;
				CvScalar y1 = y0;
				double distancey0y1sqr;
				int iteration = 0;
				int ww = min_w_h*kernel_h, hh = max_w_h + min_w_h*(kernel_h-1);// because movement is centered at the person's feet
				CvPoint* pnt = &cvPoint(blobs->GetBlob(k)->x -min_w_h ,blobs->GetBlob(k)->y -min_w_h);
				int tmpww = ww, tmphh = hh;
				do {

					pnt = &cvPoint(pnt->x + y1.val[0]*tmpww/neighborhoodSize[k],pnt->y + y1.val[1]*tmphh/neighborhoodSize[k]);
					//make sure it's not outside the boundaries of the image
					if (pnt->x <0 )
					{
						tmpww = img->width-1 - pnt->x;
						pnt->x = 0;
					}
					if (pnt->y <0 ) 
					{
						pnt->y = 0;
						tmphh = img->height-1 - pnt->y;
					}
					if (pnt->x + ww > img->width-1 ) 
						tmpww = img->width-1 - pnt->x;
					else
						tmpww = ww;
					if (pnt->y + hh > img->height-1 ) 
						tmphh = img->height-1 - pnt->y;
					else
						tmphh = hh;
					//CvPoint* pnt = &cvPoint(blobs->GetBlob(k)->x,blobs->GetBlob(k)->y);
			/*		int w = blobs->GetBlob(k)->w;
					int h = blobs->GetBlob(k)->h;*/

					//reinitialize y0 and its neighborhood
					//if (iteration >0)
					//denormalize
					blobs->GetBlob(k)->x = pnt->x +(tmpww - w[k])*0.5;
					blobs->GetBlob(k)->y = pnt->y +(tmphh - h[k])*0.5;
					if (blobs->GetBlob(k)->x <0 ) 
						blobs->GetBlob(k)->x = 0;
					if (blobs->GetBlob(k)->y <0 ) 
						blobs->GetBlob(k)->y = 0;
					if (blobs->GetBlob(k)->x + blobs->GetBlob(k)->w > img->width-1 ) 
						blobs->GetBlob(k)->x = img->width-1 - blobs->GetBlob(k)->w ;
					if (blobs->GetBlob(k)->y + blobs->GetBlob(k)->h > img->height-1 ) 
						blobs->GetBlob(k)->y = img->height-1 - blobs->GetBlob(k)->h;

					//if (ObjectPnt[0][k]->x <0 || ObjectPnt[0][k]->y <0 || ObjectPnt[0][k]->x >(img2->width-1)-(w[k]-1) || ObjectPnt[0][k]->y >(img2->height-1)-(h[k]-1)) //left the scene
					//	goto objectleft;


					SubImageChannel[0][k] = Buttacharyya::unitCircleNormalize(imgBChannel,pnt,tmpww,tmphh,neighborhoodSize[k]);
					SubImageChannel[1][k] = Buttacharyya::unitCircleNormalize(imgGChannel,pnt,tmpww,tmphh,neighborhoodSize[k]);
					SubImageChannel[2][k] = Buttacharyya::unitCircleNormalize(imgRChannel,pnt,tmpww,tmphh,neighborhoodSize[k]);
					BGSubImage[k] = Buttacharyya::unitCircleNormalize(BGimage,pnt,tmpww,tmphh,neighborhoodSize[k]);

					////REMOVE a snapshot of the object
					//IplImage* snapshot = cvCreateImage(cvSize(ww,hh),img->depth,img->nChannels);
					//cvSetImageROI(img,cvRect(pnt->x,pnt->y,ww,hh));
					//cvCopyImage(img,snapshot);
					//char* bufferTmp1 = new char[100];
					//cvPutText(snapshot,itoa(k,bufferTmp1,10),cvPoint(20,20),&cvFont(2,2),cvScalar(1,1,1));
					//delete[] bufferTmp1;
					//cvResetImageROI(img);
					//cvNamedWindow("Neighborhood");
					//cvShowImage("Neighborhood",snapshot);
					//cvReleaseImage(&snapshot);

					////REMOVE a snapshot of the object
					//snapshot = cvCreateImage(cvSize(blobs->GetBlob(k)->w,blobs->GetBlob(k)->h),img->depth,img->nChannels);
					//cvSetImageROI(img,cvRect(blobs->GetBlob(k)->x,blobs->GetBlob(k)->y,blobs->GetBlob(k)->w,blobs->GetBlob(k)->h));
					//cvCopyImage(img,snapshot);
					//char* bufferTmp2 = new char[100];
					//cvPutText(snapshot,itoa(k,bufferTmp2,10),cvPoint(20,20),&cvFont(2,2),cvScalar(1,1,1));
					//delete[] bufferTmp2;
					//cvResetImageROI(img);
					//cvNamedWindow("subjectSearched");
					//cvShowImage("subjectSearched",snapshot);
					//cvReleaseImage(&snapshot);

					
			
					////REMOVE
					//cvNamedWindow("subimage");
					//IplImage* tmpImg1 = cvCreateImage(cvGetSize(SubImageChannel[1][k]),8,1);
					//IplImage* tmpImg2 = cvCreateImage(cvGetSize(SubImageChannel[2][k]),8,1);
					//IplImage* tmpImg0 = cvCreateImage(cvGetSize(SubImageChannel[0][k]),8,1);
					//IplImage* tmpImgTotal = cvCreateImage(cvGetSize(SubImageChannel[0][k]),8,3);
					//Buttacharyya::CopyMatToImage(SubImageChannel[1][k],tmpImg1);
					//Buttacharyya::CopyMatToImage(SubImageChannel[2][k],tmpImg2);
					//Buttacharyya::CopyMatToImage(SubImageChannel[0][k],tmpImg0);
					//cvMerge(tmpImg0,tmpImg1,tmpImg2,0,tmpImgTotal);
					//char* bufferTmp = new char[100];
					//cvPutText(tmpImgTotal,itoa(iteration,bufferTmp,10),cvPoint(20,20),&cvFont(2,2),cvScalar(1,1,1));
					//delete[] bufferTmp;
					//cvShowImage("subimage",tmpImgTotal);
					//cvWaitKey(0);
					//cvReleaseImage(&tmpImg1);
					//cvReleaseImage(&tmpImg2);
					//cvReleaseImage(&tmpImg0);
					//cvReleaseImage(&tmpImgTotal);
					



					//calculate pu(y0)
					CvHistogram* puy0Hist = ConstructHist(neighborhoodSize[k], SubImageChannel[0][k], SubImageChannel[1][k], SubImageChannel[2][k],BGSubImage[k], img->depth, 1 );//BGSubImage[k]
					//Show3DHistProjections(puy0Hist, HistSizeab, "pu");
					//find the wi map

					

					////REMOVE
					CvMat* wi = cvCreateMat(end,end,CV_32F);

					////REMOVE
					//IplImage* clone = cvCloneImage(glbl_var.BGframe);
					//cvRectangle(clone,cvPoint(blobs->GetBlob(k)->x,blobs->GetBlob(k)->y),cvPoint(blobs->GetBlob(k)->x+blobs->GetBlob(k)->w,blobs->GetBlob(k)->y+blobs->GetBlob(k)->h),cvScalar(0,140,255));
					//cvRectangle(clone,cvPoint(pnt->x,pnt->y),cvPoint(pnt->x+tmpww,pnt->y+tmphh),cvScalar(0,0,255));
					//cvShowImage("video_window",clone);
					//cvReleaseImage(&clone);
					////cvWaitKey(0);
					//

					CvScalar y1numerator = cvScalar(0,0), y1denominator=cvScalar(0,0);
					//CvScalar y1numeratorAlternative = cvScalar(0,0); //used in case tmp is always 0
					for (int i= start; i < end; i++)
						for (int j= start ; j < end; j++)
						{

							//REMOVE and change back
							int temp[3];
							temp[0] = cvmGet(SubImageChannel[0][k],i,j);
							temp[1] = cvmGet(SubImageChannel[1][k],i,j);
							temp[2] = cvmGet(SubImageChannel[2][k],i,j);
							
							bxi.val[0] = cvFloor(temp[0]/HistSizeL);
							bxi.val[1] = cvFloor(temp[1]/HistSizeab);
							bxi.val[2] = cvFloor(temp[2]/HistSizeab);
							numerator = cvQueryHistValue_3D(quHist[ID],bxi.val[0],bxi.val[1],bxi.val[2]);
							denominator = cvQueryHistValue_3D(puy0Hist,bxi.val[0],bxi.val[1],bxi.val[2]);

							if (cvmGet(BGSubImage[k],i,j)!=0) //only if it's a FG pixel
							{
								if (denominator >0)
									tmp = cvSqrt(numerator/denominator);
								else
									//tmp = cvSqrt(numerator); //assuming least value for the denominator i.e. 0.0001
									tmp = 0;// meaning "not used" (check the paper section 4.1)
							}
							else
								tmp = 0; //because it's BG
							
							
							//REMOVE
							cvmSet(wi,i,j,tmp); //not necessary

							y1numerator.val[0] += (j-center)*tmp;
							y1denominator.val[0] += tmp;
							y1numerator.val[1] += (i-center)*tmp;
							y1denominator.val[1] += tmp;

							//calculate values  using l'Hopital's rule in case tmp is always zero
							//y1numeratorAlternative.val[0] += (j-center);
							//y1numeratorAlternative.val[1] += (i-center);
							


						}

						//REMOVE
					//	/*if (ID == 9 || ID == 19 || ID ==29 || ID == 14)
					//	{*/
						cvNamedWindow("wi");
						Buttacharyya::DisplayMat("wi",wi,8,1);
						cvWaitKey(0);
						cvReleaseMat(&wi);
					///*	}*/

						cvReleaseHist(&puy0Hist);
						/*Buttacharyya::DisplayMat("wi",wi,img->depth, 1);
						cvWaitKey(10);*/

						//cvWaitKey();
						//calculate distance and determine exit condition
						//if (y1denominator.val[0] != 0)
							y1.val[0] = y1numerator.val[0]/y1denominator.val[0];
						/*else
							y1.val[0] = y1numeratorAlternative.val[0];*/
						//if (y1denominator.val[1] != 0)
							y1.val[1] = y1numerator.val[1]/y1denominator.val[1];
						/*else
							y1.val[1] = y1numeratorAlternative.val[1];*/
						//cout << "iteration " << iteration << " : y1 = (" << y1.val[0] << "," << y1.val[1] << ")" << endl;
 						iteration++;
						//distancey0y1sqr = (y1.val[0] - y0.val[0])*(y1.val[0] - y0.val[0]) + (y1.val[1] - y0.val[1])*(y1.val[1] - y0.val[1]);
						distancey0y1sqr = y1.val[0]*y1.val[0] +y1.val[1]*y1.val[1];

						/*if (distancey0y1sqr >= 1)
						cvWaitKey(0);*/



				} while(distancey0y1sqr >= 1 && iteration < 10); //until error is10% of the minimum dimension

				/*ObjectPnt[1][k]->x =  ObjectPnt[0][k]->x + w[k]-1 ;
				ObjectPnt[1][k]->y = ObjectPnt[0][k]->y + h[k]-1;*/
				//cvReleaseMat(&wi);

				


			}

		}


		cvReleaseImage(&imgBChannel);
		cvReleaseImage(&imgRChannel);
		cvReleaseImage(&imgGChannel);
		for (int i=0; i < 3; i++)
			for (int j = 0 ; j < currentNumOfObjects; j++)
				cvReleaseMat(&SubImageChannel[i][j]);

		delete [] neighborhoodSize;

		for (int i=0; i <currentNumOfObjects; i++)
			{
				cvReleaseMat(&SubImageChannel[0][i]);
				cvReleaseMat(&SubImageChannel[1][i]);
				cvReleaseMat(&SubImageChannel[2][i]);
				cvReleaseMat(&BGSubImage[i]);
			}








	}

};


#endif
