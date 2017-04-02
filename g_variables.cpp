#include "stdafx.h"
#include "g_variables.h"
#include <cv.h>
#include <cvaux.h>
#include <highgui.h>
#include <fstream>





g_variables glbl_var; //instantiating the global variables


g_variables::g_variables()
{
	reset();
}

void g_variables::queryframe2currentframe()
{
	cvReleaseImage(&currentFrame);
	currentFrame = cvCreateImage(cvSize(BGmodel->get_w(),BGmodel->get_h()), BGmodel->get_depth(),BGmodel->get_nCh());


	cvResize( queryFrame,currentFrame,CV_INTER_AREA);
}

CvScalar g_variables::convert_pxl_BGR2Lab(CvScalar x)
{

	CvScalar r;
	IplImage* temp_RGB = cvCreateImage(cvSize(1,1), currentFrame->depth, currentFrame->nChannels);
	temp_RGB->imageData[0] = x.val[0];
	temp_RGB->imageData[1] = x.val[1];
	temp_RGB->imageData[2] = x.val[2];
	IplImage* temp_Lab = cvCreateImage(cvSize(1,1), currentFrame->depth, currentFrame->nChannels);

	//Conver sRGB to Linear RGB
	for (int k=0; k<3; k++)
	{
		float floatColor = (((float) (int) (unsigned char)temp_RGB->imageData[k])/255);
		temp_RGB->imageData[k]= (unsigned char) (int) ((floatColor < 0.04045) ? (floatColor/12.92)*255 : pow(((floatColor + 0.055)/1.055),2.4)*255);
	}
	cvCvtColor(temp_RGB,temp_Lab,CV_BGR2Lab);



	r = cvScalar((unsigned char)temp_Lab->imageData[0],(unsigned char) temp_Lab->imageData[1],(unsigned char) temp_Lab->imageData[2]);

	cvReleaseImage(&temp_RGB);
	cvReleaseImage(&temp_Lab);
	return r;
}

CvScalar g_variables::convert_pxl_Lab2BGR(CvScalar x)
{
	CvScalar r;

	IplImage* temp_RGB = cvCreateImage(cvSize(1,1), currentFrame->depth, currentFrame->nChannels);
	temp_RGB->imageData[0] = x.val[0];
	temp_RGB->imageData[1] = x.val[1];
	temp_RGB->imageData[2] = x.val[2];
	IplImage* temp_Lab = cvCreateImage(cvSize(1,1), currentFrame->depth, currentFrame->nChannels);

	cvCvtColor(temp_RGB,temp_Lab,CV_Lab2BGR);

	//Convert Linear RGB to sRGB
	for (int k=0; k<3; k++)
	{
		float floatColor = (((float) (int) (unsigned char)temp_Lab->imageData[k])/255);
		temp_Lab->imageData[k]= (unsigned char) (int)((floatColor < 0.0031) ? (floatColor*12.92)*255 : (1.055*pow(floatColor,(float) 0.416)-0.055)*255);
	}
	r = cvScalar((unsigned char)temp_Lab->imageData[0],(unsigned char) temp_Lab->imageData[1],(unsigned char) temp_Lab->imageData[2]);

	cvReleaseImage(&temp_RGB);
	cvReleaseImage(&temp_Lab);
	return r;
}

void g_variables::reset()
{

	DisplayedCB = 0;
	DisplayedMapCB = 0;
	MaxCB = 0;
	codebook_lbls_changed=0;
	if (BGframe) cvReleaseImage( &BGframe);
	BGframe = 0;
	if (initialMask) cvReleaseImage( &initialMask);
	initialMask = 0;
	if (initialMask3Channels) cvReleaseImage( &initialMask3Channels);
	initialMask3Channels = 0;
	if (blackFrame) cvReleaseImage( &blackFrame);
	blackFrame = 0;;
	//if (queryFrame) cvReleaseImage( &queryFrame);
	queryFrame = 0;
	delete[]  BGlabels;
	BGlabels = 0;
	if (BGmask) cvReleaseImage( &BGmask);
	BGmask = 0;
	if (currentFrame) cvReleaseImage( &currentFrame);
	currentFrame = 0;
	if (previousFrame) cvReleaseImage( &previousFrame);
	previousFrame = 0;
	if (displayedFrame) cvReleaseImage( &displayedFrame);
	displayedFrame = 0;
	if (Detailed_BG_returned_image) cvReleaseImage( &Detailed_BG_returned_image);
	Detailed_BG_returned_image = 0;
	if (OriginalCurrentFrameBeforeConvert) cvReleaseImage( &OriginalCurrentFrameBeforeConvert);
	OriginalCurrentFrameBeforeConvert = 0;
	if (Map_returned_image) cvReleaseImage( &Map_returned_image);
	Map_returned_image = 0;
	resize_factor = 2;
	resize_processing_factor = true; //false
	speedLimit = 33; //default frame time
	video_finished = 0;
	NFPP_checked = 0;
	NFPP2_checked = 0;
	//videoReadingMethod = 0; //video

	DisplayedTrackerBlob = 1;

	numberOfReadFrame = 0;

	realtimeProcessing = 1;
	timeOrFrames = IN_TERMS_OF_SECS;
	AbsoluteTime = 0;
	sampledAbsoluteTime = 0;

	trackerEnabled = false;
	behaviourEnabled = false;
	ghostEnabled = false;
	useHistDifference = false;

	used_tracker = 1;

	saveFramesToHard = false;
	saveTimes = true;
	saveBGmask = false;
	saveAnnotatedFrame = true;
	saveBGFrame = false;
	saveObjectProcessingFile = true;
	saveObjectFile = true;
	savePostProcessing = false;
	saveOriginalFrame = true;
	saveContours = false;
	saveObjectHistoryFiles = true;
	saveObjectPredictions = false;
	saveObjectHistograms = false;
	saveObjectImages = false;
	saveBehaviourFile = true;

	useHomomorphic = false;
	GaussianSmooth = false;
	useMorphology = false;
	seperateTrainTest=false;
	debugScreen = false;
	debugPrediction = false;
	debugSpeed = false;
	CameraCalibrated = false;
	cam = NULL;
	frameIsProcessed = false;

	skippedFramesCounter = 0;
	numberOfFramesToSkip = 0;


	boarder_x = 10; //pixels
	boarder_y  =10;

	predefinedColors[0] = cvScalar(0,0,255); //red
	predefinedColors[1] = cvScalar(0,255,0); //green
	predefinedColors[2] = cvScalar(255,0,0); //blue
	predefinedColors[3] = cvScalar(0,255,255); //yellow
	predefinedColors[4] = cvScalar(0,165,255); //orange
	predefinedColors[5] = cvScalar(255,255,0); //cyan
	predefinedColors[6] = cvScalar(255,0,255); //pink
	predefinedColors[7] = cvScalar(255,255,255); //white




	initialMaskFrame = false;
	


}


bool g_variables::comparableInSize(float area_a, float area_b, float &ratio)
{





	float COMPARABLE_RATIO = 0.2; //heuristic 0.45 (for 2006), 0.2(for station)

	//(ii) by areas
	ratio = ( area_a < area_b ? area_a/area_b : area_b/area_a);
	//ratio = area_a/area_b ;
	if ( ratio < COMPARABLE_RATIO ) return false;



	return true;

}

bool g_variables::comparableInSize(CvBlob* a, CvBlob* b, float &ratio_w, float &ratio_h)
{



	float COMPARABLE_RATIO = 0.2; //heuristic 0.45 for 2006), 0.2(for station)
	bool useArea = false;

	if (useArea )
	{
		//(i) (i)by w*h
		float area_a = a->w*a->h;
		float area_b = b->w*b->h;
		ratio_w = ( area_a < area_b ? area_a/area_b : area_b/area_a);
		ratio_h = -1;
		//ratio = area_a/area_b;
		if ( ratio_w < COMPARABLE_RATIO ) 
				return false;
	}
	else
	{

		//(ii) bu w and h
		ratio_w = ( a->w < b->w? a->w/b->w : b->w/a->w);
		ratio_h = ( a->h < b->h? a->h/b->h : b->h/a->h);
		if(ratio_w < COMPARABLE_RATIO)
		{
			return false;
		}

		else if(ratio_h < COMPARABLE_RATIO) 
		{
			return false;
		}

	}



	return true;

}

void g_variables::BGMaskToFrame() //combines BGmask and currentFrame to get the BGframe
{

	if( glbl_var.debugScreen) //just color with white // 
	{
		//(i)
		IplImage* BGmask3Ch  = cvCreateImage(cvGetSize(BGmask),8,3);
		//IplImage* zeroArr = cvCreateImage(cvGetSize(BGmask),8,1);
		//cvZero(zeroArr);
		//cvMerge(zeroArr,BGmask,zeroArr,NULL,BGmask3Ch);
		//find the desired color related the blob's ID
		cvMerge(BGmask,BGmask,BGmask,NULL,BGmask3Ch);
		cvAddWeighted(OriginalCurrentFrameBeforeConvert, 1, BGmask3Ch, 0.5, 0, BGframe);// (glbl_var.debugBlob ? 0.5 : 0)
		cvReleaseImage(&BGmask3Ch);
		//cvReleaseImage(&zeroArr);
		//return;
	}
	else //give colors to each object
	{

		//(ii)
		//stationary object
		bool* stationaryobject_ptr = glbl_var.BGmodel->stationary_object;

		int* BGlabel_ID = BGlabels;
		char* BGmask_ptr = BGmask->imageData;
		char* BGframe_ptr = BGframe->imageData;
		char* currentFrame_ptr = OriginalCurrentFrameBeforeConvert->imageData;
		int h = BGmodel->get_h();
		int w = BGmodel->get_w();
		int nCh = BGmodel->get_nCh();
		for (int cnt_y=0; cnt_y<h; cnt_y++)
		{
			for (int cnt_x=0; cnt_x<w; cnt_x++)
			{

				//BGmask_ptr = BGmask->imageData + cnt_y*BGmodel->get_w() + cnt_x;
				if(*BGmask_ptr == 0) //BG
				{
					//BGframe_ptr	= BGframe->imageData + cnt_y*BGmodel->get_widthStep() + cnt_x*BGmodel->get_nCh();
					//currentFrame_ptr = currentFrame->imageData + cnt_y*BGmodel->get_widthStep() + cnt_x*BGmodel->get_nCh();
					*(BGframe_ptr) = *(currentFrame_ptr);
					*(BGframe_ptr+1) = *(currentFrame_ptr+1);
					*(BGframe_ptr+2) = *(currentFrame_ptr+2);
				}
				else //FG
				{

					////stationary object
					//if(*stationaryobject_ptr)
					//{
					//	*(BGframe_ptr) = 0;
					//	*(BGframe_ptr+1) = 255;
					//	*(BGframe_ptr+2) = 0;
					//}
					//else
					//{

					//	//BGframe_ptr	= BGframe->imageData + cnt_y*BGmodel->get_widthStep() + cnt_x*BGmodel->get_nCh();
					//	*(BGframe_ptr) = 128;
					//	*(BGframe_ptr+1) = 0;
					//	*(BGframe_ptr+2) = 128;


					//}



					//if related to an object, color it
					list<TrackerObject*>::iterator itr;
					list<TrackerObject*>::iterator itr2;
					bool found = false;
					int ID;
					for( itr = BGmodel->trackerBlobs.begin() ; itr != BGmodel->trackerBlobs.end() ; itr++)
					{
						if((*itr)->CC && ((*BGlabel_ID) == (*itr)->CC->blob->ID) && (*itr)->itOrChildrenPersistent())
						{
							found = true;
							ID = (*itr)->blob->ID;
							break;
						}
					}
					if(found)
					{


						//find the child with biggest area
						int area = 0;
						int minTime = int::MaxValue;
						for(itr2 = (*itr)->occlusion_list.begin() ; itr2 != (*itr)->occlusion_list.end() ; itr2++)
						{
							//if((*itr2)->area > area)
							if((*itr2)->time < minTime)
							{
								ID = (*itr2)->blob->ID;
								//area = (*itr2)->area;
								minTime = (*itr2)->time;
							}
						}
						CvScalar color = predefinedColors[ID%7];

						for(int i =0; i <3; i++)
						{
							int value = (int) (color.val[i] + ((int) (unsigned char) *(currentFrame_ptr+i)) -128);
							*(BGframe_ptr+i) = (unsigned char) ( value > 255 ? 255 : (value < 0 ? 0 : value)) ;
						}
						

						/**(BGframe_ptr) = (char) (int) (0.75*color.val[0] + 0.25*((int)(unsigned)*(currentFrame_ptr+1)));
						*(BGframe_ptr+1) = (char) (int) (0.75*color.val[1] + 0.25*((int)(unsigned)*(currentFrame_ptr+1)));
						*(BGframe_ptr+2) = (char) (int) (0.75*color.val[2] + 0.25*((int)(unsigned)*(currentFrame_ptr+2)));*/


					}
					else
					{
						*(BGframe_ptr) = *(currentFrame_ptr);
						*(BGframe_ptr+1) = *(currentFrame_ptr+1);
						*(BGframe_ptr+2) = *(currentFrame_ptr+2);
					}
				}



				BGmask_ptr++;
				BGlabel_ID++;
				BGframe_ptr += nCh;
				currentFrame_ptr += nCh;

				//stationary object
				stationaryobject_ptr++;

			}
		}
	}
}

void g_variables::fetch_mh_params()
{
	codebook* selectedCB =  BGmodel->getCodebook( mh_prm.pxlNum_x, mh_prm.pxlNum_y, DisplayedCB);
	if (! currentFrame) 
		cvCopy( previousFrame, currentFrame);
	int indx =  mh_prm.pxlNum_y* BGmodel->get_widthStep() +  mh_prm.pxlNum_x* BGmodel->get_nCh();


	if (selectedCB != NULL) //if CW exists
	{
		mh_prm.MNRL = selectedCB->getMNRL();
		mh_prm.freq = selectedCB->getCW_freq();
		mh_prm.lastacc = selectedCB->getlast_access();
		mh_prm.firstacc = selectedCB->getfirst_access();
		mh_prm.codebookNumber =  DisplayedCB;

		mh_prm.maxCB = ( BGmodel->getNum_of_active_models_per_pixel())[( mh_prm.pxlNum_y)* BGmodel->get_w() + ( mh_prm.pxlNum_x)];

		if ( BGmodel->get_used_method() == KimOriginal) //RGB Kim
		{
			mh_prm.R_CB = selectedCB->getRGB().val[0];
			mh_prm.G_CB = selectedCB->getRGB().val[1];
			mh_prm.B_CB = selectedCB->getRGB().val[2];
			mh_prm.Imax = selectedCB->getImax();
			mh_prm.Imin = selectedCB->getImin();
		}
		else if ( BGmodel->get_used_method() == LabSpherical ||  BGmodel->get_used_method() == LabCylindrical) //Lab Kim
		{
			mh_prm.L_CB = selectedCB->getLab().val[0];
			mh_prm.a_CB = selectedCB->getLab().val[1];
			mh_prm.b_CB = selectedCB->getLab().val[2];
			mh_prm.Imax = -1;
			mh_prm.Imin = -1;
		}




	}
	else //if CW doesn't
	{
		//negative values indicate it doesn't exist
		mh_prm.Imax = -1;
		mh_prm.Imin = -1;
		mh_prm.MNRL = -1;
		mh_prm.freq = -1;
		mh_prm.lastacc = -1;
		mh_prm.firstacc = -1;
		mh_prm.codebookNumber = -1;
		mh_prm.maxCB = 0;

	}

	//save color components values
	if ( BGmodel->get_used_method() == KimOriginal)
	{
		mh_prm.R = (unsigned char) currentFrame->imageData[indx];
		mh_prm.G = (unsigned char) currentFrame->imageData[indx+1];
		mh_prm.B = (unsigned char) currentFrame->imageData[indx+2];
	}
	else if ( BGmodel->get_used_method() == LabSpherical ||  BGmodel->get_used_method() == LabCylindrical)
	{
		CvScalar x = cvScalar((unsigned char)  LabFrame->imageData[indx]
		,(unsigned char)  LabFrame->imageData[indx+1]
		,(unsigned char)  LabFrame->imageData[indx+2]);
		x =  display2real_Lab(x);
		mh_prm.L = x.val[0];
		mh_prm.a = x.val[1];
		mh_prm.b = x.val[2];
	}
}

int g_variables::calc_pxl_status_mh(float &d, int m, float &d2)//delta, method, and delta2 (in case not used, use a dummy)
{
	if ( BGmodel->get_used_method() == KimOriginal)//Kim RGB
	{
		d =  BGmodel->colordist( cvScalar(  mh_prm.R,  mh_prm.G,  mh_prm.B), cvScalar(  mh_prm.R_CB,  mh_prm.G_CB,  mh_prm.B_CB));
		bool colorCond = (d >  BGmodel->get_epsilon2());
		bool colorCond2 = (d >  BGmodel->get_epsilon1());
		bool timeCond = ( BGmodel->get_t() >  BGmodel->get_N());

		char* temp = new char[3];
		temp[0]=  mh_prm.R;
		temp[1]=  mh_prm.G;
		temp[2]=  mh_prm.B;
		float I_temp =  BGmodel->calc_I_fromSqr(temp);
		delete [] temp;
		bool brightnessCond = ! BGmodel->brightness(I_temp, mh_prm.Imin,  mh_prm.Imax);
		bool I_less_Imin = (I_temp <  BGmodel->get_Ilow( mh_prm.Imin));
		bool fminCond = (( mh_prm.freq <  BGmodel->get_fmin()) && ( BGmodel->get_fminState()));
		bool MNRLCond = ( mh_prm.MNRL >  BGmodel->get_TM());
		bool AgeCond = (BGmodel->get_t() - mh_prm.firstacc < BGmodel->get_N());

		if ( MNRLCond || fminCond || (AgeCond && !glbl_var.seperateTrainTest)) //MNRL or fmin
			return 7;
		else if (colorCond && timeCond && brightnessCond) //Color & brightness in training stage
			return 1;
		else if (colorCond && timeCond) //color in training
			return 2;
		else if ( colorCond2 && !timeCond && brightnessCond) //Color & brightness in test stage
			return 3;
		else if ( colorCond2 && !timeCond) //Color in test stage
			return 4;
		else if (brightnessCond && I_less_Imin) //Brightness low
			return 5;
		else if (brightnessCond && !I_less_Imin) //Brightness high
			return 6;

		else //BG
			return 0;
	}
	else if ( BGmodel->get_used_method() == LabSpherical) //Lab sphere Kim
	{
		d =  BGmodel->Lab_colorDifference( cvScalar(  mh_prm.L,  mh_prm.a,  mh_prm.b), 
			cvScalar(  mh_prm.L_CB,  mh_prm.a_CB,  mh_prm.b_CB));
		bool colorCond = (d >  BGmodel->get_DeltaE());
		bool MNRLCond = ( mh_prm.MNRL >  BGmodel->get_TM());
		bool fminCond = (( mh_prm.freq <  BGmodel->get_fmin()) && ( BGmodel->get_fminState()));
		bool timeCond = ( BGmodel->get_t() >  BGmodel->get_N());

		bool AgeCond = (BGmodel->get_t() - mh_prm.firstacc < BGmodel->get_N());

		if ( MNRLCond || fminCond || (AgeCond && !glbl_var.seperateTrainTest)) //MNRL or fmin
			return 7;
		else if ( colorCond ) //color difference
			return 11;
		else //BG
			return 0;
	}
	else if ( BGmodel->get_used_method() == LabCylindrical) //Lab Cylinder Kim
	{

		d =  BGmodel->Lab_colorChromaticityDifference( cvScalar(  mh_prm.L,  mh_prm.a,  mh_prm.b), 
			cvScalar(  mh_prm.L_CB,  mh_prm.a_CB,  mh_prm.b_CB));
		d2 =  BGmodel->Lab_colorLuminanceDifference( cvScalar(  mh_prm.L,  mh_prm.a,  mh_prm.b), 
			cvScalar(  mh_prm.L_CB,  mh_prm.a_CB,  mh_prm.b_CB));
		bool chromaCond = (d >  BGmodel->get_DeltaC());
		bool luminanceCond = (d2 >  BGmodel->get_DeltaL());
		bool ShadowCond;
		if (luminanceCond == false) //either shadow or BG
			ShadowCond = (d2 >  BGmodel->get_DeltaS());
		bool MNRLCond = ( mh_prm.MNRL >  BGmodel->get_TM());
		bool fminCond = (( mh_prm.freq <  BGmodel->get_fmin()) && ( BGmodel->get_fminState()));
		bool timeCond = ( BGmodel->get_t() >  BGmodel->get_N());

		bool AgeCond = (BGmodel->get_t() - mh_prm.firstacc < BGmodel->get_N());

		if ( MNRLCond || fminCond || (AgeCond && !glbl_var.seperateTrainTest)) //MNRL or fmin
			return 7;
		else if (chromaCond || luminanceCond)
			return 12;
		else if (ShadowCond)
			return 14;
		else //BG
			return 0;

		//if (chromaCond && luminanceCond)
		//	return 12;
		//else if ( chromaCond ) //color difference
		//	return 13;
		//else if ( luminanceCond )
		//	return 14;
		//else if ( MNRLCond) //MNRL time
		//	return 7;
		//else //BG
		//	return 0;

	}
}

int g_variables::calc_pxl_status_pointer(float &d, int m, codebook* ptr, CvScalar x, float &d2)//delta and method
{
	bool timeCond = ( BGmodel->get_t() >  BGmodel->get_N());
	bool fminCond = ((ptr->getCW_freq() <  BGmodel->get_fmin()) && ( BGmodel->get_fminState()));
	bool MNRLCond = (ptr->getMNRL() >  BGmodel->get_TM());
	bool AgeCond = (BGmodel->get_t() - ptr->getfirst_access() < BGmodel->get_N());
	bool colorCond;
	bool colorCond2;
	bool brightnessCond;
	bool chromaCond;
	bool I_less_Imin;
	bool luminanceCond;
	bool ShadowCond;
	float I_temp;
	char* temp;
	switch(BGmodel->get_used_method())
	{
	case KimOriginal://Kim RGB
		d =  BGmodel->colordist( x, ptr->getRGB());
		colorCond = (d >  BGmodel->get_epsilon2());
		colorCond2 = (d >  BGmodel->get_epsilon1());


		temp = new char[3];
		temp[0]= ptr->getRGB().val[0];
		temp[1]= ptr->getRGB().val[1];
		temp[2]= ptr->getRGB().val[2];
		I_temp =  BGmodel->calc_I_fromSqr(temp);
		delete [] temp;
		brightnessCond = ! BGmodel->brightness(I_temp,ptr->getImin(), ptr->getImax());
		I_less_Imin = (I_temp <  BGmodel->get_Ilow(ptr->getImin()));


		if (colorCond && timeCond && brightnessCond) //Color & brightness in training stage
			return KimOriginal_FG_ColorBrightness_Training;
		else if (colorCond && timeCond) //color in training
			return KimOriginal_FG_Color_Training;
		else if ( colorCond2 && !timeCond && brightnessCond) //Color & brightness in test stage
			return KimOriginal_FG_ColorBrightness_Testing;
		else if ( colorCond2 && !timeCond) //Color in test stage
			return KimOriginal_FG_Color_Testing;
		else if (brightnessCond && I_less_Imin) //Brightness low
			return KimOriginal_FG_BrightnessLow;
		else if (brightnessCond && !I_less_Imin) //Brightness high
			return KimOriginal_FG_BrightnessHigh;
		else if ( MNRLCond || fminCond || (AgeCond && !glbl_var.seperateTrainTest)) //MNRL or fmin
			return AllMethods_FG_MNRLorFreq;
		else //BG
			return AllMethods_BG;

		break;
	case LabSpherical: //Lab sphere Kim
		d =  BGmodel->Lab_colorDifference( x, ptr->getLab() );
		colorCond = (d >  BGmodel->get_DeltaE());

		if ( MNRLCond || fminCond || (AgeCond && !glbl_var.seperateTrainTest)) //MNRL time or fmin
			return AllMethods_FG_MNRLorFreq;
		else if ( colorCond ) //color difference
			return LabSpherical_FG_Color;
		else //BG
			return AllMethods_BG;
		break;
	case LabCylindrical:
		d =  BGmodel->Lab_colorChromaticityDifference( x, ptr->getLab() );
		d2 =  BGmodel->Lab_colorLuminanceDifference( x, ptr->getLab() );
		chromaCond = (d >  BGmodel->get_DeltaC());
		luminanceCond = (d2 >  BGmodel->get_DeltaL());
		if (luminanceCond == false) //either shadow or BG
			ShadowCond = (d2 >  BGmodel->get_DeltaS());


		if (chromaCond || luminanceCond)
			return LabCylindrical_FG_ChromaLuminance;
		else if (ShadowCond)
			return LabCylindrical_FG_Shadow;
		else if ( MNRLCond || fminCond || (AgeCond && !glbl_var.seperateTrainTest)) //MNRL time or fmin
			return AllMethods_FG_MNRLorFreq;
		else //BG
			return AllMethods_BG;
		break;
	}
}

CvScalar g_variables::real2display_Lab(CvScalar x)
{
	CvScalar returned;
	returned.val[0] = x.val[0]*255/100;
	returned.val[1] = x.val[1] + 128 ;
	returned.val[2] = x.val[2] + 128 ;
	return returned;
}

CvScalar g_variables::display2real_Lab(CvScalar x)
{
	CvScalar returned;
	returned.val[0] = x.val[0]*100/255;
	returned.val[1] = x.val[1] - 128 ;
	returned.val[2] = x.val[2] - 128 ;
	return returned;
}


// red = detected, blue = BG blobs, green= tracker blobs
void g_variables::drawTrackerBlobs()
{
	int x1, x2, y1, y2,ID;
	int xp1, xp2, yp1, yp2;
	char buffer[100];
	char buffer2[100];
	char buffer3[100];
	char buffer4[100];
	char buffer5[100];
	char type[100];
	char occlusion[100];
	CvScalar blobColor;
	list<TrackerObject*>::iterator ptrObj;
	list<TrackerObject*>::iterator ptrObj2;

	if (!glbl_var.debugScreen || glbl_var.debugBlob)
		cvResize(glbl_var.BGframe,glbl_var.displayedFrame,CV_INTER_AREA );
	else
		cvResize(glbl_var.copiedQueryFrame,glbl_var.displayedFrame,CV_INTER_AREA );

	//write text on both BG frame and displayed frame
	CvFont font;
	cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.6, 0.6, 0, 2, CV_AA);

	


	for (ptrObj= BGmodel->trackerBlobs.begin(); ptrObj != BGmodel->trackerBlobs.end(); ptrObj++)
	{



		if (glbl_var.debugScreen)
		{
			xp1 = (*ptrObj)->predictedPosition().x *glbl_var.resize_factor- (*ptrObj)->predictedPosition().w/2*glbl_var.resize_factor;
			yp1 = (*ptrObj)->predictedPosition().y*glbl_var.resize_factor - (*ptrObj)->predictedPosition().h/2*glbl_var.resize_factor;
			xp2 = (*ptrObj)->predictedPosition().x*glbl_var.resize_factor + (*ptrObj)->predictedPosition().w/2*glbl_var.resize_factor;
			yp2 = (*ptrObj)->predictedPosition().y*glbl_var.resize_factor + (*ptrObj)->predictedPosition().h/2*glbl_var.resize_factor;
		}


		if (glbl_var.debugPrediction)
		{
			cvRectangle(glbl_var.displayedFrame,cvPoint(xp1,yp1), cvPoint(xp2,yp2),cvScalar(0,255,255),2);
			//cvRectangle(glbl_var.copiedQueryFrame,cvPoint(xp1,yp1), cvPoint(xp2,yp2),cvScalar(0,255,255),1);
			//draw centroids as circles
			cvCircle(glbl_var.displayedFrame, cvPoint((*ptrObj)->predictedPosition().x *glbl_var.resize_factor,(*ptrObj)->predictedPosition().y*glbl_var.resize_factor), 2, cvScalar(0,255,255));

		}

		//ID of prediction
		if (glbl_var.debugPrediction)
		{
			ID = (*ptrObj)->blob->ID;
			//cvRectangle(BGframe,cvPoint((xp2 + 2*xp1)/3-1, yp2 - 3), cvPoint((2*xp2 + xp1)/3+1,yp2 + 3),cvScalar(255,255,255),CV_FILLED);
			//cvRectangle(glbl_var.copiedQueryFrame,cvPoint((xp2 + 2*xp1)/3-1, yp2 - 3), cvPoint((2*xp2 + xp1)/3+1,yp2 + 3),cvScalar(255,255,255),CV_FILLED);
			cvPutText(glbl_var.displayedFrame,itoa(ID,buffer,10), cvPoint((xp2 + 2*xp1)/3, yp2 - 4), &font,cvScalar(0,255,255));
			//cvPutText(glbl_var.copiedQueryFrame,itoa(ID,buffer,10), cvPoint((xp2 + 2*xp1)/3, yp2 - 2), &font,cvScalar(0,255,255));
		}

		//if ((*ptrObj)->absence != -1) continue; //skip absent frames
		if (!(*ptrObj)->matched ) continue;// if not visible also don't show 
		//THIS SHOULD BE CORRECTED TO BE ONLY DEPENDENT ON ISVISIBLE !


		//don't show object if they and their children aren't persistent (unless debugging mode)
		//(i)
		//if (glbl_var.debugScreen || (*ptrObj)->persistent ) //(glbl_var.BGmodel->get_t()-1) - (*ptrObj)->persistence >= glbl_var.BGmodel->PERSISTENCE_THRESHOLD
		//	goto persists;
		//for (ptrObj2= (*ptrObj)->occlusion_list.begin(); ptrObj2 != (*ptrObj)->occlusion_list.end(); ptrObj2++)
		//{
		//	if ( (*ptrObj)->persistent)  //(glbl_var.BGmodel->get_t()-1) - (*ptrObj2)->persistence >= glbl_var.BGmodel->PERSISTENCE_THRESHOLD
		//	{
		//		goto persists;
		//	}
		//}
		//(ii)
		if (glbl_var.debugScreen || (*ptrObj)->itOrChildrenPersistent()) goto persists;


		continue; //because the object is not persistent if it excution reaches here

persists:
		//x1 = (*ptrObj)->blob->x;
		//y1 = (*ptrObj)->blob->y;
		//x2 = x1 + (*ptrObj)->blob->w;
		//y2 = y1 + (*ptrObj)->blob->h;
		x1 = (*ptrObj)->CC->blob->x*glbl_var.resize_factor;
		y1 = (*ptrObj)->CC->blob->y*glbl_var.resize_factor;
		x2 = x1 + (*ptrObj)->CC->blob->w*glbl_var.resize_factor;
		y2 = y1 + (*ptrObj)->CC->blob->h*glbl_var.resize_factor;

		objClassification obj_class = (*ptrObj)->object_classification;

		//dont show CCs at the boarders
		//if (x1 <= 0|| y1 <=0 || x2 >= glbl_var.BGmodel->get_w()*glbl_var.resize_factor || y2 >= glbl_var.BGmodel->get_h()*glbl_var.resize_factor)
		//	continue;


		/*if (((*ptrObj)->time != -1) && (glbl_var.BGmodel->get_t()-1 - (*ptrObj)->time > BGmodel->LOITERING_THRESHOLD))
			blobColor = cvScalar(0,0,0);
		else
		{*/
			//color code per occlusion group
			//if (!(*ptrObj)->occlusion)
			blobColor = cvScalar(0,0,255);
			//else
			//	blobColor = cvScalar(((*ptrObj)->occlusion%4)*64,0,0);
		//}

		//draw the rectangleon both BG frame and displayed frame
		/*if (glbl_var.debugScreen)
		cvRectangle(BGframe,cvPoint(x1,y1), cvPoint(x2,y2),blobColor,1);
		else
		cvRectangle(glbl_var.copiedQueryFrame,cvPoint(x1,y1), cvPoint(x2,y2),blobColor,1);*/
		//only show persistent objects
		if (glbl_var.debugScreen)
			cvRectangle(glbl_var.displayedFrame,cvPoint(x1,y1), cvPoint(x2,y2),blobColor,2);
		//draw centroids as circles
		cvCircle(glbl_var.displayedFrame, cvPoint((*ptrObj)->blob->x*glbl_var.resize_factor,(*ptrObj)->blob->y*glbl_var.resize_factor), 2, cvScalar(0,255,0));

		//draw predictions
		//predictions



		//ID and object classification
		//cvRectangle(BGframe,cvPoint((x2 + 2*x1)/3-1, y2 - 3), cvPoint((2*x2 + x1)/3+1,y2 + 3),cvScalar(255,255,255),CV_FILLED);
		//cvRectangle(glbl_var.copiedQueryFrame,cvPoint((x2 + 2*x1)/3-1, y2 - 3), cvPoint((2*x2 + x1)/3+1,y2 + 3),cvScalar(255,255,255),CV_FILLED);
		/*if (glbl_var.debugScreen)
		cvPutText(BGframe,itoa(ID,buffer,10), cvPoint((x2 + 2*x1)/3, y2 - 2), &font,blobColor);
		else
		cvPutText(glbl_var.copiedQueryFrame,itoa(ID,buffer,10), cvPoint((x2 + 2*x1)/3, y2 - 2), &font,blobColor);*/
		char lbl[100];
		strcpy(lbl,( obj_class == UNKNOWN ? "U" :(obj_class == PERSON ? "P" : (obj_class == STILL_PERSON ? "SP" : "O" )) ));
		//if debug, add parent ID
		if(glbl_var.debugDummy || (*ptrObj)->occlusion_list.empty())
		{
				ID = (*ptrObj)->blob->ID;
				strcat(lbl,itoa(ID,buffer,10));
		}
		for (ptrObj2= (*ptrObj)->occlusion_list.begin(); ptrObj2 != (*ptrObj)->occlusion_list.end(); ptrObj2++)
		{
			strcat(lbl,",");
			int ID = (*ptrObj2)->blob->ID;
			strcat(lbl,itoa(ID,buffer,10));
		}
		//cvPutText(glbl_var.displayedFrame,itoa(ID,buffer,10), cvPoint((x2 + 2*x1)/3, y2 - 4), &font,blobColor);
		if(glbl_var.debugScreen)
			cvPutText(glbl_var.displayedFrame,lbl, cvPoint((x2 + 2*x1)/3, y2 - 4), &font,blobColor);




		//time
		//cvRectangle(BGframe,cvPoint((x2 + 2*x1)/3-1, y1 - 3), cvPoint((2*x2 + x1)/3+1,y1 + 3),cvScalar(255,255,255),CV_FILLED);
		//cvRectangle(glbl_var.copiedQueryFrame,cvPoint((x2 + 2*x1)/3-1, y1 - 3), cvPoint((2*x2 + x1)/3+1,y1 + 3),cvScalar(255,255,255),CV_FILLED);
		//if (glbl_var.debugScreen)
		//	cvPutText(BGframe,strcat( itoa((glbl_var.numberOfReadFrame - (*ptrObj)->time)/glbl_var.OriginalVideoFrameRate,buffer2,10),"sec"), cvPoint((x2 + 2*x1)/3, y1 - 2), &font,blobColor);
		//else
		//	cvPutText(glbl_var.copiedQueryFrame,strcat( itoa((glbl_var.numberOfReadFrame - (*ptrObj)->time)/glbl_var.OriginalVideoFrameRate,buffer2,10),"sec"), cvPoint((x2 + 2*x1)/3, y1 - 2), &font,blobColor);
		if(glbl_var.debugScreen)
			cvPutText(glbl_var.displayedFrame,strcat( itoa(frame_to_sec(glbl_var.BGmodel->get_t()-1- (*ptrObj)->time,true),buffer2,10),"s"), cvPoint((x2 + 2*x1)/3, y1 - 4), &font,blobColor);


		////type
		//if ((*ptrObj)->type == 0)
		//	strcpy(type,"bag");
		//else
		//	strcpy(type,"person");
		//cvPutText(BGframe, type, cvPoint(x1 - 10, y2 + 10), &font,blobColor);
		//cvPutText(glbl_var.copiedQueryFrame,type, cvPoint(x1 - 10, y2 + 10), &font,blobColor);

		////occluser/occluded/Group
		//if ((*ptrObj)->occlusion)
		//{
		//	strcat(occlusion,itoa((*ptrObj)->occlusion,buffer3,10));
		//	cvPutText(BGframe, occlusion, cvPoint( (x2 + x1)/2 , y1 - 10), &font,blobColor);
		//	cvPutText(glbl_var.copiedQueryFrame,occlusion, cvPoint((x2 + x1)/2 , y1 - 10), &font,blobColor);
		//}

		////associated CC
		//cvPutText(BGframe, itoa((*ptrObj)->CC->blob->ID,buffer4,10), cvPoint((x2 + x1)/2, y2 + 10), &font,blobColor);
		//cvPutText(glbl_var.copiedQueryFrame, itoa((*ptrObj)->CC->blob->ID,buffer4,10), cvPoint((x2 + x1)/2, y2 + 10), &font,blobColor);


		////CC-Obj color distance
		//sprintf(buffer5,"%.2f",(*ptrObj)->distanceFromCC);
		//cvPutText(BGframe, buffer5, cvPoint(x2 + 10, y1 - 10), &font,blobColor);
		//cvPutText(glbl_var.copiedQueryFrame, buffer5, cvPoint(x2 + 10, y1 - 10), &font,blobColor);


		//draw circles of faint
		if (glbl_var.debugFaint)
		{
			//prepare dummy image
			IplImage* dummyImage = cvCreateImage(cvGetSize(glbl_var.displayedFrame), glbl_var.displayedFrame->depth,glbl_var.displayedFrame->nChannels);;
			for(int i=0; i < dummyImage->width; i++)
			{
				for(int j=0; j < dummyImage->height; j++)
				{
					dummyImage->imageData[i*dummyImage->nChannels + j*dummyImage->widthStep + 1] = (unsigned char) 128;
					dummyImage->imageData[i*dummyImage->nChannels + j*dummyImage->widthStep + 2] = (unsigned char) 128;
					dummyImage->imageData[i*dummyImage->nChannels + j*dummyImage->widthStep] = (unsigned char) 128;
				}
			}

			//circle
			double originx,originy;
			double projx,projy;
			double ux, uy, x2,y2, ux2, uy2, wx, wy;
			double originXcamera= (*(*ptrObj)->currentHistoryMeasurement->begin())->xBase, originYcamera = (*(*ptrObj)->currentHistoryMeasurement->begin())->yBase;
			//double originXcamera= 11160, originYcamera = 1900;			
			cam->worldToImage(originXcamera, originYcamera, 0, ux2, uy2);
			ux2*=glbl_var.resize_factor/glbl_var.camCalXRatio;
			uy2*=glbl_var.resize_factor/glbl_var.camCalYRatio;
			for(double phi = 0; phi  < 360 ; phi += 1)
			{
				wx = originXcamera + glbl_var.BGmodel->FALLING_TOLERANCE*cos(phi*3.14/180);
				wy = originYcamera + glbl_var.BGmodel->FALLING_TOLERANCE*sin(phi*3.14/180);
				cam->worldToImage(wx, wy, 0, ux, uy);
				ux*=glbl_var.resize_factor/glbl_var.camCalXRatio;
				uy*=glbl_var.resize_factor/glbl_var.camCalYRatio;
				
			/*	cam->undistortedToDistortedImageCoord(ux,uy,x2,y2);
				cam->undistortedToDistortedImageCoord(ux2,uy2,originx,originy);
				cvLine(dummyImage,cvPoint(originx,originy),cvPoint(x2,y2),cvScalar(0, 0,0),2);*/
				cvLine(dummyImage,cvPoint(ux2,uy2),cvPoint(ux,uy),cvScalar(0, 0,0),2);
			}


			double headX= (*(*ptrObj)->currentHistoryMeasurement->begin())->xTop, headY = (*(*ptrObj)->currentHistoryMeasurement->begin())->yTop;
			cam->worldToImage(headX,headY,BGmodel->AVERAGE_PERSON_HEIGHT,wx,wy);
			wx*=glbl_var.resize_factor/glbl_var.camCalXRatio;
			wy*=glbl_var.resize_factor/glbl_var.camCalYRatio;
			//cvLine(dummyImage,cvPoint(originx,originy),cvPoint(wx,wy),cvScalar(0, 255,0),2,CV_AA );
			cvLine(dummyImage,cvPoint(ux2,uy2),cvPoint(wx,wy),cvScalar(0, 255,0),2,CV_AA );


			//approximate projections
			cam->worldToImage(headX, headY, 0, ux2, uy2);
			ux2*=glbl_var.resize_factor/glbl_var.camCalXRatio;
			uy2*=glbl_var.resize_factor/glbl_var.camCalYRatio;
			//cam->undistortedToDistortedImageCoord(ux2,uy2,projx,projy);
			//cvLine(dummyImage,cvPoint(projx,projy),cvPoint(x2,y2),cvScalar(0, 0,255),1,CV_AA );
			cvLine(dummyImage,cvPoint(ux2,uy2),cvPoint(wx,wy),cvScalar(0, 0,255),1,CV_AA );

			cvAddWeighted(glbl_var.displayedFrame, 1, dummyImage, 1, -128, glbl_var.displayedFrame);


			cvReleaseImage(&dummyImage);


		}

		//lines of velocity and normals
		if(glbl_var.debugSpeed && glbl_var.CameraCalibrated)
		{
			double xBase,yBase, xTop, yTop, xAltitude, yAltitude;

			
			list<historyPoint*>::iterator itr_hp = (*ptrObj)->currentHistoryMeasurement->begin(); //assuming the object is visible and so only one hp exists
			glbl_var.cam->worldToImage((*itr_hp)->xBase,(*itr_hp)->yBase,0,xBase,yBase);
			//glbl_var.cam->worldToImage((*itr_hp)->xTop,(*itr_hp)->yTop,glbl_var.BGmodel->AVERAGE_PERSON_HEIGHT,xTop,yTop);
			//glbl_var.cam->worldToImage((*itr_hp)->xBase,(*itr_hp)->yBase,glbl_var.BGmodel->AVERAGE_PERSON_HEIGHT,xAltitude,yAltitude);
			xBase*=glbl_var.resize_factor/glbl_var.camCalXRatio;
			yBase*=glbl_var.resize_factor/glbl_var.camCalYRatio;
			//xTop*=glbl_var.resize_factor/glbl_var.camCalXRatio;
			//yTop*=glbl_var.resize_factor/glbl_var.camCalYRatio;
			//xAltitude*=glbl_var.resize_factor/glbl_var.camCalXRatio;
			//yAltitude*=glbl_var.resize_factor/glbl_var.camCalYRatio;



			//altitude and top-base line in green
			//cvLine(glbl_var.displayedFrame,cvPoint(xBase,yBase),cvPoint(xTop,yTop),cvScalar(0,255,0)); //top-base
			//cvLine(glbl_var.displayedFrame,cvPoint(xBase,yBase),cvPoint(xAltitude,yAltitude),cvScalar(0,125,0)); //top-base

			//velocities in yellow
			list<motionPossibility*>::iterator itr_mp;
			list<TrackerObject*> temp_lst = (*ptrObj)->occlusion_list;
			temp_lst.push_front(*ptrObj);
			for (ptrObj2= temp_lst.begin(); ptrObj2 != temp_lst.end(); ptrObj2++)
			{
				for(itr_hp = (*ptrObj2)->lastHistoryMeasurement->begin() ; itr_hp != (*ptrObj2)->lastHistoryMeasurement->end()  ; itr_hp++)
				{

					glbl_var.cam->worldToImage((*itr_hp)->xBase,(*itr_hp)->yBase,0,xBase,yBase);
					xBase*=glbl_var.resize_factor/glbl_var.camCalXRatio;
					yBase*=glbl_var.resize_factor/glbl_var.camCalYRatio;

					for(itr_mp = (*itr_hp)->motionPossibilities.begin() ; itr_mp != (*itr_hp)->motionPossibilities.end()  ; itr_mp++)
					{
						double x_vel, y_vel;
						glbl_var.cam->worldToImage((*itr_mp)->vxBase + (*itr_hp)->xBase, (*itr_mp)->vyBase + (*itr_hp)->yBase,0,x_vel,y_vel);

						x_vel*=glbl_var.resize_factor/glbl_var.camCalXRatio;
						y_vel*=glbl_var.resize_factor/glbl_var.camCalYRatio;


						cvLine(glbl_var.displayedFrame,cvPoint(xBase,yBase),cvPoint(x_vel,y_vel),cvScalar(0,255,255)); 

					}

				}
			}

		}






	}


	//behaviour log

	//char behaviour_log[10000];
	if(glbl_var.behaviourEnabled)
	{
		strcpy(behaviour_log,"");
		glbl_var.BehaviourDescriptor.detectBehaviours();
	}





	//for (int i=0; i < BGmodel->BGSubtractionBlobs->GetBlobNum(); i++)
	//{
	//	x1 = BGmodel->BGSubtractionBlobs->GetBlob(i)->x;
	//	y1 = BGmodel->BGSubtractionBlobs->GetBlob(i)->y;
	//	x2 = x1 + BGmodel->BGSubtractionBlobs->GetBlob(i)->w;
	//	y2 = y1 + BGmodel->BGSubtractionBlobs->GetBlob(i)->h;
	//	ID = BGmodel->BGSubtractionBlobs->GetBlob(i)->ID;
	//	cvRectangle(BGframe,cvPoint(x1,y1), cvPoint(x2,y2),cvScalar(0,0,0),2);
	//	CvFont font;
	//	cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.4, 0.4, 0, 2, CV_AA);
	//	cvPutText(BGframe,itoa(ID,buffer,10), cvPoint(x1 + 10, y1 - 10), &font,cvScalar(0,0,0)); 

	//}
}

CvScalar g_variables::convertBGR2Lab_byTable(CvScalar x)
{
	CvScalar r;
	r = BGR2Lab_table[int(x.val[0]+ x.val[1]*256 + 256*256*x.val[2])];
	return r;
}
CvScalar g_variables::convertLab2BGR_byTable(CvScalar x)
{
	CvScalar r;
	r = Lab2BGR_table[int(x.val[0]+ x.val[1]*256 + 256*256*x.val[2])];
	return r;
}
void g_variables::buildBGR2Lab_table()
{
	ofstream BGR2LabModel("BGR2LabModel.clr",ios::out);

	int B = 0;
	int G = 0;
	int R = 0;
	CvScalar temp;
	do
	{
		temp =  convert_pxl_BGR2Lab(cvScalar(B,G,R));
		BGR2LabModel << (int) temp.val[0]  << ' ' << (int) temp.val[1] << ' '  << (int) temp.val[2] << ' ';
		//BGR2LabModel << temp.val[0] << "," << temp.val[1] << "," << temp.val[2] <<"\n";

		//BGR2Lab_table[B + G*256+ R*256*256] = temp;
		B++;
		if (B >= 256)
		{
			B=0;
			G++;
		}
		if (G >= 256)
		{
			G=0;
			R++;
		}
	} while( R <256);

	BGR2LabModel.close();
}
void g_variables::buildLab2BGR_table()
{
	ofstream Lab2BGRModel("Lab2BGRModel.clr",ios::out);

	int L = 0;
	int a = 0;
	int b = 0;
	CvScalar temp;
	do
	{
		temp = convert_pxl_Lab2BGR(cvScalar(L,a,b));
		Lab2BGRModel << (int) temp.val[0]  << ' '<< (int) temp.val[1] << ' ' << (int) temp.val[2] <<' ' ;
		//Lab2BGRModel << temp.val[0] << "," << temp.val[1] << "," << temp.val[2] <<"\n";
		//Lab2BGR_table[L + a*256+ b*256*256] = temp;
		L++;
		if (L >= 256)
		{
			L=0;
			a++;
		}
		if (a >= 256)
		{
			a=0;
			b++;
		}
	} while( b <256);

	Lab2BGRModel.close();
}

void g_variables::loadBGR2Lab_table()
{
	ifstream BGR2LabModel("BGR2LabModel.clr",ios::in);

	int B = 0;
	int G = 0;
	int R = 0;
	CvScalar temp;	
	double tmp[3];
	do
	{
		//temp =  convert_pxl_BGR2Lab(cvScalar(B,G,R));
		BGR2LabModel >> tmp[0] >> tmp[1] >> tmp[2]; 
		temp.val[0] =  tmp[0];
		temp.val[1] =  tmp[1];
		temp.val[2] =  tmp[2];
		BGR2Lab_table[B + G*256+ R*256*256] = temp;
		B++;
		if (B >= 256)
		{
			B=0;
			G++;
		}
		if (G >= 256)
		{
			G=0;
			R++;
		}
	} while( R <256);

	BGR2LabModel.close();
}

void g_variables::loadLab2BGR_table()
{
	ifstream Lab2BGRModel("Lab2BGRModel.clr",ios::in);

	int L = 0;
	int a = 0;
	int b = 0;
	CvScalar temp;
	double tmp[3];
	do
	{
		//temp = convert_pxl_Lab2BGR(cvScalar(L,a,b));
		Lab2BGRModel >> tmp[0] >> tmp[1] >> tmp[2]; 
		temp.val[0] =  tmp[0];
		temp.val[1] =  tmp[1];
		temp.val[2] =  tmp[2];
		Lab2BGR_table[L + a*256+ b*256*256] = temp;
		L++;
		if (L >= 256)
		{
			L=0;
			a++;
		}
		if (a >= 256)
		{
			a=0;
			b++;
		}
	} while( b <256);

	Lab2BGRModel.close();

}

void g_variables::homomorphicFiltering()
{

	int size = 100;
	float a=2;
	float n =2;
	float d =1 ;
	float e  =0;
	IplImage* image64_1 = cvCreateImage(cvGetSize(currentFrame),IPL_DEPTH_64F,1 );
	IplImage* image64_2_original = cvCreateImage(cvGetSize(currentFrame),IPL_DEPTH_64F,1 );
	IplImage* image64_3_original = cvCreateImage(cvGetSize(currentFrame),IPL_DEPTH_64F,1 );
	IplImage* Channel1 = cvCreateImage(cvGetSize(currentFrame), currentFrame->depth,1);
	IplImage* Channel2 = cvCreateImage(cvGetSize(currentFrame), currentFrame->depth,1);
	IplImage* Channel3 = cvCreateImage(cvGetSize(currentFrame), currentFrame->depth,1);
	cvSplit(currentFrame,Channel1,Channel2,Channel3,0);
	cvScale(Channel1, image64_1, 1.0/255);  
	cvScale(Channel2, image64_2_original, 1);  
	cvScale(Channel3, image64_3_original, 1); 
	IplImage* dst1 = cvCreateImage(cvSize(currentFrame->width,currentFrame->height),IPL_DEPTH_64F ,1);
	IplImage* filter1 = createButterwarthFilter(size,a,n,d,e);
	dst1 = homomorphic_filter(image64_1,filter1,1);
	IplImage* final = cvCreateImage(cvGetSize(dst1),dst1->depth,3);
	cvScale(dst1, dst1, 255.0); 
	cvSetImageROI(final,cvRect(0,0,image64_2_original->width,image64_2_original->height));
	cvSetImageROI(dst1,cvRect(0,0,image64_2_original->width,image64_2_original->height));
	cvMerge(dst1,image64_2_original,image64_3_original,0,final);
	cvScale(final,currentFrame,1.0);
	cvReleaseImage(&image64_1);
	cvReleaseImage(&image64_2_original);
	cvReleaseImage(&image64_3_original);
	cvReleaseImage(&Channel1);
	cvReleaseImage(&Channel2);
	cvReleaseImage(&Channel3);
	cvReleaseImage(&dst1);
	cvReleaseImage(&filter1);
	cvReleaseImage(&final);
}

void g_variables::convertCurrentFrameRGB2sRGB()
{
	for (int i=0; i < currentFrame->width; i++)
		for (int j=0; j < currentFrame->height; j++)
			for (int k=0; k<3; k++)
			{
				int indx = i*currentFrame->nChannels + j*currentFrame->widthStep+k;
				float floatColor = (((float) (int) (unsigned char)currentFrame->imageData[indx])/255);
				currentFrame->imageData[indx] = (unsigned char) (floatColor < 0.04045) ? (floatColor/12.92)*255 : pow(((floatColor + 0.055)/1.055),2.4)*255;
			}
}

void g_variables::ConvertFrameBGR2Lab(IplImage* img)
{
	int size=  img->height*img->widthStep;
	CvScalar tmp;
	for(int i=0; i < size; i+=3)
	{
		tmp = cvScalar((double) (unsigned char) img->imageData[i], (double) (unsigned char) img->imageData[i+1], (double) (unsigned char) img->imageData[i+2]);
		tmp = convertBGR2Lab_byTable(tmp);
		img->imageData[i] = (unsigned char)(double) tmp.val[0];
		img->imageData[i+1] = (unsigned char)(double) tmp.val[1];
		img->imageData[i+2] = (unsigned char)(double) tmp.val[2];
	}


}
void g_variables::equalizeHistLabFrame()
{
	//histogram equalization of the Lightness component
	IplImage* Ch[3];
	for(int i=0; i<3; i++)
		Ch[i] = cvCreateImage(cvGetSize(currentFrame),8,1);
	cvSplit(currentFrame,Ch[0],Ch[1],Ch[2],NULL);
	cvEqualizeHist(Ch[0],Ch[0]);
	cvMerge(Ch[0],Ch[1],Ch[2],NULL,currentFrame);
}

int g_variables::sec_to_frame(float sec,bool duringProcessing) //duringProcessing = false means using original frame rate, otherwise using 1 every n frame
{

	if (realtimeProcessing && BGmodel->get_t() > 0)
	{
		float temp = sec *  BGmodel->get_t();
		temp /=  AbsoluteTime;
		return floor(temp);
	}
	else 
	{
		float temp = OriginalVideoFrameRate;
		if (duringProcessing) temp /= (numberOfFramesToSkip+1);
		temp *= sec;
		return floor(temp);
	}
}

float g_variables::frame_to_sec(int frame,bool duringProcessing) //duringProcessing = false means using original frame rate, otherwise using 1 every n frame
{

	if (realtimeProcessing && BGmodel->get_t() > 0)
	{
		float temp = frame * AbsoluteTime;
		temp /= BGmodel->get_t();
		return temp;
	}
	else
	{
		float temp = frame;
		temp /= OriginalVideoFrameRate;
		if(duringProcessing) temp *= (numberOfFramesToSkip+1);
		return temp;
	}
}
