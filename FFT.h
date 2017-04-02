#ifndef FFT_h
#define FFT_h

#include <cxcore.h> 
#include <cv.h> 
#include <highgui.h> 
#include <iostream>

using namespace std;



//http://opencv-users.1802565.n2.nabble.com/faint-image-after-Inverse-DFT-using-cvDFT-td2193072.html

// Rearrange the quadrants of Fourier image so that the origin is at 
// the image center 
// src & dst arrays of equal size & type 
void cvShiftDFT(CvArr * src_arr, CvArr * dst_arr ) 
{ 
    CvMat * tmp; 
    CvMat q1stub, q2stub; 
    CvMat q3stub, q4stub; 
    CvMat d1stub, d2stub; 
    CvMat d3stub, d4stub; 
    CvMat * q1, * q2, * q3, * q4; 
    CvMat * d1, * d2, * d3, * d4; 

    CvSize size = cvGetSize(src_arr); 
    CvSize dst_size = cvGetSize(dst_arr); 
    int cx, cy; 

    if(dst_size.width != size.width || 
       dst_size.height != size.height){ 
        cvError( CV_StsUnmatchedSizes, "cvShiftDFT", "Source and Destination arrays must have equal sizes", __FILE__, __LINE__ );   
    } 

    if(src_arr==dst_arr){ 
        tmp = cvCreateMat(size.height/2, size.width/2, 
cvGetElemType(src_arr)); 
    } 
    
    cx = size.width/2; 
    cy = size.height/2; // image center 

    q1 = cvGetSubRect( src_arr, &q1stub, cvRect(0,0,cx, cy) ); 
    q2 = cvGetSubRect( src_arr, &q2stub, cvRect(cx,0,cx,cy) ); 
    q3 = cvGetSubRect( src_arr, &q3stub, cvRect(cx,cy,cx,cy) ); 
    q4 = cvGetSubRect( src_arr, &q4stub, cvRect(0,cy,cx,cy) ); 
    d1 = cvGetSubRect( src_arr, &d1stub, cvRect(0,0,cx,cy) ); 
    d2 = cvGetSubRect( src_arr, &d2stub, cvRect(cx,0,cx,cy) ); 
    d3 = cvGetSubRect( src_arr, &d3stub, cvRect(cx,cy,cx,cy) ); 
    d4 = cvGetSubRect( src_arr, &d4stub, cvRect(0,cy,cx,cy) ); 

    if(src_arr!=dst_arr){ 
        if( !CV_ARE_TYPES_EQ( q1, d1 )){ 
            cvError( CV_StsUnmatchedFormats, "cvShiftDFT", "Source and Destination arrays must have the same format", __FILE__, __LINE__ ); 
        } 
        cvCopy(q3, d1, 0); 
        cvCopy(q4, d2, 0); 
        cvCopy(q1, d3, 0); 
        cvCopy(q2, d4, 0); 
    } 
    else{ 
        cvCopy(q3, tmp, 0); 
        cvCopy(q1, q3, 0); 
        cvCopy(tmp, q1, 0); 
        cvCopy(q4, tmp, 0); 
        cvCopy(q2, q4, 0); 
        cvCopy(tmp, q2, 0); 
    } 
} 

void NormalizeDraw64Image(IplImage* img, char* name)
{
	double m,M;
	//scale back
	IplImage* img2 = cvCloneImage(img);
    cvMinMaxLoc(img, &m, &M, NULL, NULL, NULL); 
    cvScale(img, img2, 1.0/(M-m), 1.0*(-m)/(M-m)); 
	cvNamedWindow(name);
	cvShowImage(name,img2);
	cvWaitKey();
}




void ShowMagFreq(IplImage* im, char* name)
{
	/*cvMat* hdr;
	CvMat* image_Re = cvGetMat(im1,hdr);
	CvMat* image_Im = cvGetMat(im2,hdr);*/

	double m,M;
	IplImage* image_Im = cvCreateImage(cvGetSize(im), im->depth,1);
	IplImage* image_Re = cvCreateImage(cvGetSize(im), im->depth,1);
	cvSplit(im,image_Re,image_Im,0,0);
	  cvMinMaxLoc(image_Re, &m, &M, NULL, NULL, NULL); 
	    cvMinMaxLoc(image_Im, &m, &M, NULL, NULL, NULL); 

	   // Compute the magnitude of the spectrum Mag = sqrt(Re^2 + Im^2) 
    cvPow( image_Re, image_Re, 2.0);
    cvPow( image_Im, image_Im, 2.0); 
    cvAdd( image_Re, image_Im, image_Re, NULL); 
    cvPow( image_Re, image_Re, 0.5 ); 


    // Compute log(1 + Mag) 
    cvAddS( image_Re, cvScalarAll(1.0), image_Re, NULL ); // 1 + Mag 
    cvLog( image_Re, image_Re ); // log(1 + Mag) 
	cvShiftDFT(image_Re, image_Re );




     cvMinMaxLoc(image_Re, &m, &M, NULL, NULL, NULL); 
    cvScale(image_Re, image_Re, 1.0/(M-m), 1.0*(-m)/(M-m)); 
	cvShowImage(name,image_Re);

	cvReleaseImage(&image_Im);
	cvReleaseImage(&image_Re);

	cvWaitKey(0);


}



void ShowMagSpace(IplImage* im, char* name)
{
		double m,M;


	IplImage* image_Im = cvCreateImage(cvGetSize(im), im->depth,1);
	IplImage* image_Re = cvCreateImage(cvGetSize(im), im->depth,1);
	cvSplit(im,image_Re,image_Im,0,0);

	   // Compute the magnitude of the spectrum Mag = sqrt(Re^2 + Im^2) 
    cvPow( image_Re, image_Re, 2.0); 
    cvPow( image_Im, image_Im, 2.0); 
    cvAdd( image_Re, image_Im, image_Re, NULL);
    cvPow( image_Re, image_Re, 0.5 ); 

	  cvMinMaxLoc(image_Re, &m, &M, NULL, NULL, NULL); 
    cvScale(image_Re, image_Re, 1.0/(M-m), 1.0*(-m)/(M-m)); 
	cvShowImage(name,image_Re);

	cvWaitKey(0);

	cvReleaseImage(&image_Im);
	cvReleaseImage(&image_Re);

}


int FFT(IplImage* A, IplImage* dft_A) 
{ 
	IplImage* src = cvCloneImage(dft_A);
	CvMat tmp,tmp2;
	cvSetImageROI(src,cvRect(0,0,A->width,A->height));
	cvCopy( A, src );
	cvResetImageROI(src);
	if (A->width < src->width)
	{
		cvSetImageROI(src,cvRect( A->width, 0, src->width - A->width, A->height ));
		cvZero( src );
		cvResetImageROI(src);
	}

	cvDFT( src, dft_A, CV_DXT_FORWARD, src->height );

		cvReleaseImage(&src);
   
 	return 0;

} 

int InvFFT(IplImage* dft_A, IplImage* C) 
{ 
	IplImage* Inv_dft_A = cvCloneImage(dft_A);

	CvMat tmp;
	cvDFT( dft_A, Inv_dft_A, CV_DXT_INVERSE_SCALE  , C->height );
	cvSetImageROI(Inv_dft_A,cvRect((C->width - dft_A->width)/2,(C->height - dft_A->height)/2,dft_A->width,dft_A->height) );
	cvCopy( Inv_dft_A, C );
	cvResetImageROI(Inv_dft_A);

	cvReleaseImage(&Inv_dft_A);

	return 0;

} 

#endif