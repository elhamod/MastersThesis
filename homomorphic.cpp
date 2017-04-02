#include "stdafx.h"
#include "homomorphic.h"
#include "NCC.h"
#include "FFT.h"

//normalize and show an image or Matrix
void ShowArr(IplImage* img, char* name)
{
	double m,M;
	cvMinMaxLoc(img, &m, &M, NULL, NULL, NULL); 
	cvScale(img, img, 1.0/(M-m), 1.0*(-m)/(M-m)); 
	cvShowImage(name, img);
}



//filter is 2planes, both dst and src are single plane
IplImage* homomorphic_filter(IplImage* src, IplImage* filter,bool illuminationCh)
{
	double original_m, original_M;
	int dft_M = cvGetOptimalDFTSize( src->height);
	int dft_N = cvGetOptimalDFTSize( src->width );
	IplImage* temp1 = cvCloneImage(src);

	//take the log
	cvMinMaxLoc(temp1, &original_m, &original_M, NULL, NULL, NULL); 
	cvScale(temp1,temp1,255);

	if (illuminationCh)
	{
		//log for BGR
		cvAddS(temp1,cvScalar(1),temp1);
		cvLog(temp1,temp1);
	}
	else
	{

		//log for Lab
		for (int i=0; i < temp1->width; i++)
			for (int j=0; j < temp1->height; j++)
			{
				double element = cvGet2D(temp1,j,i).val[0];

				if (element <0)
				{
					if (1-element)
					{
						int x;
						x=0;
					}
					cvSet2D(temp1,j,i,cvScalar(-log(1-element)));
				}
				else
					cvSet2D(temp1,j,i,cvScalar(log(1+element)));

			}
	}

	//renormalize the log
	double m,M;

	//find the right size and do FFT
	IplImage* FFT_Mat = cvCreateImage(cvSize(dft_N,dft_M),src->depth,2);
	cvZero(FFT_Mat);
	IplImage* temp2 = cvCreateImage(cvGetSize(src),src->depth,2);
	IplImage* srcIM = cvCreateImage( cvSize(src->width,src->height), IPL_DEPTH_64F, 1); 
	cvZero(srcIM);
	cvMerge(temp1,srcIM,0,0,temp2);
	FFT(temp2, FFT_Mat) ;

	//make the filter suitable for the image size
	IplImage* temp8bit = cvCreateImage(cvSize(dft_N,dft_M),8,2);
	for (int i=0; i < temp8bit->width; i++)
		for (int j=0; j < temp8bit->height; j++)
		{
			temp8bit->imageData[2*i + j*temp8bit->widthStep] = 1;
			temp8bit->imageData[2*i+1 + j*temp8bit->widthStep] = 0;
		}

		IplImage* temp = cvCreateImage(cvSize(dft_N,dft_M),src->depth,2);
		cvScale(temp8bit,temp,1.0);
		int h_offset = filter->width/2;
		int v_offset = filter->height/2;
		int w = (temp->width/2 < h_offset) ? temp->width/2 : h_offset;
		int h = (temp->height/2 < v_offset) ? temp->height/2 : v_offset;
		IplImage* filter64 = cvCreateImage(cvGetSize(filter),IPL_DEPTH_64F,filter->nChannels);
		cvScale(filter, filter64,1);


		//first corner
		cvSetImageROI(temp,cvRect(0,0,w, h));
		cvSetImageROI(filter64,cvRect(0,0,w, h));
		cvCopy(filter64, temp);
		cvResetImageROI(temp);
		cvResetImageROI(filter64);
		//second corner
		cvSetImageROI(temp,cvRect(temp->width - w,0,w, h));
		cvSetImageROI(filter64,cvRect(filter64->width - w,0,w, h));
		cvCopy(filter64, temp);
		cvResetImageROI(temp);
		cvResetImageROI(filter64);
		//third corner
		cvSetImageROI(temp,cvRect(temp->width - w,temp->height - h,w, h));
		cvSetImageROI(filter64,cvRect(filter64->width - w,filter64->height - h, w,h));
		cvCopy(filter64, temp);
		cvResetImageROI(temp);
		cvResetImageROI(filter64);
		//first corner
		cvSetImageROI(temp,cvRect(0,temp->height - h,w, h));
		cvSetImageROI(filter64,cvRect(0,filter64->height - h,w, h)); 
		cvCopy(filter64, temp);
		cvResetImageROI(temp);
		cvResetImageROI(filter64);

		//multiply by the filter
		NCC(FFT_Mat,temp,temp);

		//do IFFT
		InvFFT(temp, temp);
		IplImage* dst = cvCreateImage(cvGetSize(temp),src->depth,1); //Size should be changed so that the sides are trimmed
		cvSplit(temp,dst,0,0,0);

		//exponantiate
		if (illuminationCh)
		{
			//exp for BGR
			cvExp(dst,dst);
		}
		else
		{
			//exp for Lab
			for (int i=0; i < dst->width; i++)
				for (int j=0; j < dst->height; j++)
				{
					double element = cvGet2D(dst,j,i).val[0];
					if (element <0)
						cvSet2D(dst,j,i,cvScalar(-exp(-element)));
					else
						cvSet2D(dst,j,i,cvScalar(exp(element)));

				}
		}

		cvAddS(dst,cvScalar(-1),dst);
		cvScale(dst,dst,1.0/255);

		//release
		cvReleaseImage(&filter64);
		cvReleaseImage(&temp);
		cvReleaseImage(&temp8bit);
		cvReleaseImage(&FFT_Mat);
		cvReleaseImage(&temp2);
		cvReleaseImage(&srcIM);
		cvReleaseImage(&temp1);


		return dst;
}

//the filter is 2 planes real and imaginary
IplImage* createFilter(int freq)
{
	IplImage* filter = cvCreateImage(cvSize(freq*4,freq*4),8,2);
	cvZero(filter);
	cvAddS(filter,cvScalar(1,0),filter);


	for(int i=0; i <freq; i++)
		for(int j=0; j <freq; j++)
		{
			filter->imageData[2*i + filter->widthStep*j] = 0; //real part
			filter->imageData[2*i+1 + filter->widthStep*j] = 0; //imaginary part
		}

		for(int i=3*freq; i <freq*4; i++)
			for(int j=3*freq; j <freq*4; j++)
			{
				filter->imageData[2*i + filter->widthStep*j] = 0; //real part
				filter->imageData[2*i+1 + filter->widthStep*j] = 0; //imaginary part
			}

			for(int i=0; i <freq; i++)
				for(int j=3*freq; j <freq*4; j++)
				{
					filter->imageData[2*i + filter->widthStep*j] = 0; //real part
					filter->imageData[2*i+1 + filter->widthStep*j] = 0; //imaginary part
				}

				for(int i=3*freq; i <freq*4; i++)
					for(int j=0; j <freq; j++)
					{
						filter->imageData[2*i + filter->widthStep*j] = 0; //real part
						filter->imageData[2*i+1 + filter->widthStep*j] = 0; //imaginary part
					}

					//scale back
					IplImage* filter_64_2ch = cvCreateImage(cvGetSize(filter),IPL_DEPTH_64F,2 );
					cvScale(filter, filter_64_2ch, 1.0, 0.0); 

					return filter;
}

//the filter is 2 planes real and imaginary
IplImage* createButterwarthFilter(int freq, float a, float n, float d, float e) 
// (1 - (1 + pow((square_d/(double)a),(douibe)n)))) *d + e
// --------------- u2+v2          a and n            offset and scale
{
	IplImage* filter = cvCreateImage(cvSize(freq*2,freq*2),8,2);
	cvZero(filter);
	cvAddS(filter,cvScalar(255,0),filter);


	for(int i=0; i <freq; i++)
		for(int j=0; j <freq; j++)
		{
			float square_d = pow((i*i + j*j)/a,n);
			if (a != 0)
				filter->imageData[2*i + filter->widthStep*j] = 255*((1 - 1/(1 + square_d)) *d + e); //real part
			else
				filter->imageData[2*i + filter->widthStep*j] = 255*d + e;
			filter->imageData[2*i+1 + filter->widthStep*j] = 0; //imaginary part
		}

		for(int i=freq; i <freq*2; i++)
			for(int j=freq; j <freq*2; j++)
			{
				float square_d = pow(((freq*2-1 - i)*(freq*2-1 - i) + (freq*2-1 - j)*(freq*2-1 - j))/a,n);
				if (a != 0)
					filter->imageData[2*i + filter->widthStep*j] = 255*((1 - 1/(1 + square_d)) *d + e); //real part
				else
					filter->imageData[2*i + filter->widthStep*j] = 255*d + e;
				filter->imageData[2*i+1 + filter->widthStep*j] = 0; //imaginary part
			}

			for(int i=0; i <freq; i++)
				for(int j=freq; j <freq*2; j++)
				{
					float square_d = pow((i*i + (freq*2-1 - j)*(freq*2-1 - j))/a,n);
					if (a != 0)
						filter->imageData[2*i + filter->widthStep*j] = 255*((1 - 1/(1 + square_d)) *d + e); //real part
					else
						filter->imageData[2*i + filter->widthStep*j] = 255*d + e;
					filter->imageData[2*i+1 + filter->widthStep*j] = 0; //imaginary part
				}

				for(int i=freq; i <freq*2; i++)
					for(int j=0; j <freq; j++)
					{
						float square_d =pow(((freq*2-1 - i)*(freq*2-1 - i) + j*j)/a,n);
						if (a != 0)
							filter->imageData[2*i + filter->widthStep*j] = 255*((1 - 1/(1 + square_d)) *d + e); //real part
						else
							filter->imageData[2*i + filter->widthStep*j] = 255*d + e;
						filter->imageData[2*i+1 + filter->widthStep*j] = 0; //imaginary part
					}

					//scale back
					filter->imageData[0] = (unsigned char) (int) (255*d+e);
					IplImage* filter_64_2ch = cvCreateImage(cvGetSize(filter),IPL_DEPTH_64F,2 );
					cvScale(filter, filter_64_2ch, 1.0/255, 0.0); 

					cvReleaseImage(&filter);

					return filter_64_2ch;
}