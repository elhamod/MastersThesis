#ifndef B_D
#define B_D

#include <cv.h>
#include <cvaux.h>
#include <highgui.h>
#include "BGmodel.h"
#include <fstream>
#include "cameraModel.h"
#include "homomorphic.h"
#include "xmlUtil.h"

#pragma once
#using <mscorlib.dll>


// BGsubtraction methods
#define KimOriginal 0
#define LabSpherical 1
#define LabCylindrical 2

//Codeword matching cases
#define AllMethods_BG 0
#define KimOriginal_FG_ColorBrightness_Training 1
#define KimOriginal_FG_Color_Training 2
#define KimOriginal_FG_ColorBrightness_Testing 3
#define KimOriginal_FG_Color_Testing 4
#define KimOriginal_FG_BrightnessLow 5
#define KimOriginal_FG_BrightnessHigh 6
#define AllMethods_FG_MNRLorFreq 7
#define LabSpherical_FG_Color 11
#define LabCylindrical_FG_ChromaLuminance 12
#define LabCylindrical_FG_Shadow 14


//video reading method
#define readFromVideo 0
#define readFromImages 1

//time or frame
#define IN_TERMS_OF_FRAMES 1
#define IN_TERMS_OF_SECS 0

// distance units
#define METER 0
#define CENTIMETER 2
#define MILLIMETER 3
#define PIXELS 4





public struct mousehandler_param
	{
		int pxlNum_x; //pixel coordinates
		int pxlNum_y;

		float	Imax;//Kims CW parameters
		float	Imin;
		int	MNRL;
		int	freq;
		int	lastacc;
		int	firstacc;

		int	codebookNumber; //the "depth" of the CW

		double R; //to show the pixel in the current frame
		double G;
		double B;

		double L; //same as RGB but converted to Lab
		double a;
		double b;

		double R_CB; //to show the CW color (in case the original RGB approach is used)
		double G_CB;
		double B_CB;

		double L_CB; //to show the CW color (in case Lab is used)
		double a_CB;
		double b_CB;

		short maxCB;

		int region_num;
		int region_area;
	};

public class g_variables
{
	
//global variables
public:
	g_variables(); //constructor
	char* filename; //file name
	CvCapture* videoStream; //the video stream
	IplImage* currentFrame; //the frame being displayed
	IplImage* queryFrame; //used to query the frame before resizing it if any
	IplImage* BGframe; //the background (colored to show different reasons for considering a pixel as foreground)
	IplImage* BGmask; //background mask(0 = BG)
	IplImage* previousFrame; //the second last frame (used to find the median to calculate the epsilons)
	IplImage* Map_returned_image; //the image of the saved background of index x
	IplImage* Detailed_BG_returned_image;//contains coloring of the reason for each pixel to be foreground
	IplImage* OriginalCurrentFrameBeforeConvert;//used to preserve a copy of an image before conversion to Lab (because Copy is faster than converting back)
	IplImage* initialMask; //used to remove parts of the frame that we don't want processed
	IplImage* initialMask3Channels; //a copy with 3 channels instead of 1
	IplImage* blackFrame;
	BG_codebook_model* BGmodel; //the BG model
	bool video_paused; //used for checking whether the video is running or not
	int currentFramePos; //the absolute position of the current frame (used for loading images from image database)
	void reset(); //resets the variables
	IplImage* displayedFrame; //the displayed (resized) image
	IplImage* LabFrame;
	IplImage* concatimage;//to display both background and foreground along eachother
	int speedLimit; // if speed is faster than this, it's limited (milli seconds)
	bool video_finished;
	CvScalar convert_pxl_BGR2Lab(CvScalar);
	CvScalar convert_pxl_Lab2BGR(CvScalar);
	void BGMaskToFrame(); //combines BGmask and currentFrame to get the BGframe
	void fetch_mh_params(); //fetches the parameters for the CW to be displayed
	int calc_pxl_status_pointer(float &, int , codebook* , CvScalar, float & );//delta, method, codebook and pixel, d2 used in case of cylinder or just pass a dummy)
	CvScalar real2display_Lab(CvScalar); //OpenCv technicalities
	CvScalar display2real_Lab(CvScalar); //OpenCV technicalities
	void queryframe2currentframe(); // resizes query frame to current frame
	bool videoReadingMethod; //0 = video, 1 = images
	int numberOfReadFrame; //represents the total number of frames read at (ReadProcess) (not the same as t in BGmodel) (both should be equal if frame by frame)
	float OriginalVideoFrameRate; //float to allow division
	bool realtimeProcessing; //0= frame by frame, 1 = real-time
	bool timeOrFrames; //0=time, 1= frames
	float AbsoluteTime; //total amount of time passed since the beginning of the video in seconds (float to allow division)
	int used_tracker; //=0 if using mean shift tracker, 1 if using Tavakkoli tracker.
	bool useHomomorphic;
	bool GaussianSmooth;
	bool useMorphology;
	bool seperateTrainTest;
	ofstream CPUfile;
	bool debugScreen;
	bool debugPrediction;
	bool debugSpeed;
	bool debugDummy;
	bool debugBlob;
	bool debugFaint;
	bool initialMaskFrame;
	bool frameIsProcessed; // this is used for real time processing to synchronize readFrame and processFrame
	
	int skippedFramesCounter; //used to skip frames in a frame by frame processing criterion
	int numberOfFramesToSkip; //used to skip frames in a frame by frame processing criterion

	//troubleshooting parameters
	bool saveFramesToHard; 
	bool saveTimes;
	bool saveBGmask;
	bool saveAnnotatedFrame;
	bool saveBGFrame;
	bool saveOriginalFrame;
	bool saveObjectProcessingFile;
	bool saveObjectFile;
	bool savePostProcessing;
	bool saveContours;
	bool saveObjectHistoryFiles;
	bool saveObjectPredictions;
	bool saveObjectHistograms;
	bool saveObjectImages;
	bool saveBehaviourFile;

	//camera calibration
	bool CameraCalibrated;
	Etiseo::CameraModel *cam ;
	float camCalXRatio, camCalYRatio; //ratios of resized frame to original size

	//CB and BG queries
	mousehandler_param mh_prm; //to pass parameters to mouse handler
	int DisplayedCB; //The index of the displayed information of a CB
	int DisplayedMapCB; //used for CB frame display
	int MaxCB; //the max CB available for the clicked pixel
	bool codebook_lbls_changed; //set to true if the user request new codeword
	int calc_pxl_status_mh(float &, int, float &);//(colorDif"delta" by reference,method used, d2 used in case of cylinder or just pass a dummy)


	//Resizing
	double resize_factor; //frame desplay multiplication factor
	bool resize_processing_factor; //frame processing multiplication factor

	//Postprocessing
	bool NFPP_checked;
	bool NFPP2_checked;
	bool repeatLabeling; //used because sometimes there are min area blobs inside other min area blobs
	int numOfRepeating;
	int* BGlabels; //used by post processing to label connected pixels 


	//Image reading variables
	//char* ImageReadingPath;
	char* ImageReadingExtension;
	int numberOfDigits;
	int startFromFrame;

	//Tracker
	bool trackerEnabled;
	bool trackerEnabledOnlyAfterTraining;
	int DisplayedTrackerBlob;
	void drawTrackerBlobs();
	char foldername[100]; //where the frames are saved
	IplImage* copiedQueryFrame;// used to save blobs of tracker

	//Behaviour	
	bool behaviourEnabled;
	bool ghostEnabled;
	bool useHistDifference;
	BehaviourMatrix BehaviourDescriptor;
	char behaviour_log[10000];



	//Lab and RGB conversions by lookup table
	CvScalar BGR2Lab_table[256*256*256]; //the 2nd dimension is changeable to change granularity (B G R, L a b)
	CvScalar Lab2BGR_table[256*256*256]; //the 2nd dimension is changeable to change granularity (L a b, B G R)
	CvScalar convertBGR2Lab_byTable(CvScalar);
	CvScalar convertLab2BGR_byTable(CvScalar);
	void buildBGR2Lab_table(); 
	void buildLab2BGR_table(); 
	void loadBGR2Lab_table();
	void loadLab2BGR_table();
	void ConvertFrameBGR2Lab(IplImage*);
	void convertCurrentFrameRGB2sRGB();
	void equalizeHistLabFrame();
	void homomorphicFiltering();

				


	//aux 
	int sec_to_frame(float,bool);
	float frame_to_sec(int,bool);
	char saveFramesTo[100];
	bool comparableInSize(CvBlob*, CvBlob*, float&, float&); //compares the sizes of the blobs and returns true if they are comparable wrt a constant inside the function
	bool comparableInSize(float, float, float&); //compares the sizes of the blobs and returns true if they are comparable wrt a constant inside the function


	//boarders
	int boarder_x;
	int boarder_y;

	//predefined RGB colors
	CvScalar predefinedColors[8];


	double sampledAbsoluteTime;

	//lists of IDs of objects participating in behaviours
	list<int> abandonedLuggaeID;
	list<int> theftLuggageID;
	list<int> loiterID;
	list<int> faintID;
	list<int> fightID;
	list<int> walkTogetherID;
	list<int> meetID;
	






};





extern g_variables glbl_var;


#endif;
