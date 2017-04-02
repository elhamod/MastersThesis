#ifndef B_T
#define B_T


#include <cv.h>
#include <cvaux.h>
#include <highgui.h>
#include <list>
#include "Tracker2.h"
#include "Behaviour.h"
//#include "Buttacharyya.h"
#pragma once
#using <mscorlib.dll>
using namespace System;



//#define L_max 20 //L is the maximum number of codebooks in the model (likely to need only 6 at max)




//classes

public class codebook //defines a codeword
{
public:


	codebook(); //constructor for the codebook

	CvScalar codebook::getRGB() //gets the RGB center of the codeword
	{
		return codebook::RGB;
	}
	void codebook::setRGB(CvScalar in_param) //sets the RGB center of the codeword
	{
		codebook::RGB.val[0] = in_param.val[0];
		codebook::RGB.val[1] = in_param.val[1];
		codebook::RGB.val[2] = in_param.val[2];
	}


	CvScalar getLab() //gets the Lab center of the codeword (only one of Lab and RGB is used)
	{
		return Lab;
	}

	void setLab(CvScalar x) //sets the Lab center of the codeword
	{
		Lab = x;
	}

	float getImax()
	{
		return Imax;
	}
	void setImax(float in_param)
	{
		Imax = in_param;
	}

	float getImin()
	{
		return Imin;
	}
	void setImin(float in_param)
	{
		Imin = in_param;
	}

	int getCW_freq()
	{
		return CW_freq;
	}
	void setCW_freq(int in_param)
	{
		CW_freq = in_param;
	}

	int getMNRL()
	{
		return MNRL;
	}
	void setMNRL(int in_param)
	{
		MNRL = in_param;
	}

	int getfirst_access()
	{
		return first_access;
	}
	void setfirst_access(int in_param)
	{
		first_access = in_param;
	}

	int getlast_access()
	{
		return last_access;
	}
	void setlast_access(int in_param)
	{
		last_access = in_param;
	}

	codebook* get_next_CW()
	{
		return next_CW;
	}

	void set_next_CW(codebook* x)
	{

		next_CW = x;
	}

	void codebook::delete_next_CW()
	{
		if (!next_CW)
			return; 

		codebook* temp = next_CW;
		next_CW = temp->next_CW;
		delete temp;
	}


private:
	CvScalar RGB; //only integers 0 - 255
	CvScalar Lab;
	float Imax; //could be floating (because squared root is used)
	float Imin; //could be floating (because squared root is used)
	int CW_freq; 
	int MNRL;
	int first_access;
	int last_access;
	codebook* next_CW; // because the data structure uses CW->Cw->CW->.... -> NULL



};




public class BG_codebook_model
{


public:
	BG_codebook_model(int, int); //int height, int width
	BG_codebook_model(); //default constructor (not usually used)
	~BG_codebook_model(); //destructor
	void update_model(IplImage*); //takes an image (RGB) as an input, and updates the model in training period
	void BG_subtraction(IplImage*); //(currentFrame (RGB), returned bacground (magenta/white)) 1 = foreground, 0 = background
	void update_MNRLs(); //updating the CWs' MNRLs that haven't been accessed lately.



	int* rtrn_ptr_t() //returns a pointer to t for the slide bar use. 
	{
		return &t;
	}

	int get_t() //returns the value of t;
	{
		return t;
	}

	void setSize(int h, int w)				
	{
		BG_codebook_model(h, w);
	}

	void calc_epsilons(); //calculate epsilons based on spacio tempora median
	void update_medians(); //adds the spatial median of the current frame
	codebook* getCodebook(int ,int,int);//returns a specified codebook
	codebook* force_getCodebook(int ,int ,int ); //ONLY USED for CB read: returns a specified codebook
	short* getNum_of_active_models_per_pixel(); //returns a pointer to the num_... array
	float colordist(CvScalar, CvScalar); //(sample, ref)
	bool brightness(float, float, float); //(I, I low, I high)

	void BG_codebook_model::set_spc_medians_size(int sz)//sets the size of the spatial medians dynamically allocated array
	{
		spc_medians = new double[sz];
	} 


	float get_epsilon1() //k*delta
	{
		return epsilon1;
	}
	float get_epsilon2() //k2*delta
	{
		return epsilon2;
	}

	void set_epsilon1(float x) //k*delta
	{
		epsilon1 = x;
	}
	void set_epsilon2(float x) //k2*delta
	{
		epsilon2 = x;
	}


	float get_k() //get k in training stage
	{
		return k;
	}
	void set_k(float k_val) //set k in training stage//set k in training stage
	{
		k = k_val;
	}

	float get_k2()	//get k in testing stage
	{
		return k2;
	}
	void set_k2(float k2_val) 	 //set k in testing stage
	{
		k2 = k2_val;
	}

	float get_alpha()
	{
		return alpha;
	}
	void set_alpha(float alpha_val)
	{
		alpha = alpha_val;
	}

	float get_beta() //used to infer I high
	{
		return beta;
	}
	void set_beta(float beta_val)
	{
		beta = beta_val;
	} 

	int get_N() //determines the period of training stage
	{
		return N;
	}
	void set_N(int param_N)
	{
		N = param_N;
	} 

	int get_TM() //determines the Threshold for MNRL (usually N/2)
	{
		return TM;
	}
	void set_TM(int param_TM)
	{
		TM = param_TM;
	} 


	float get_Ilow(float int_max)
	{
		return alpha*int_max;
	}

	float get_Ihigh(float int_min, float int_max)
	{
		if (int_min/alpha < beta*int_max)
			return int_min/alpha;
		else
			return int_max*beta;
	}

	void set_displayed_dimensions(int x, int y) //(width, height)
	{
		displayed_width = x;
		displayed_height = y;
	}
	int get_displayed_height()
	{
		return displayed_height;
	}
	int get_displayed_weight()
	{
		return displayed_width;
	}

	void reset_model_dimensions(int, int); //(width, height) resets the whole background model, so only used before the modeling starts

	void set_h(int x)
	{
		h = x;
	}

	void set_w(int x)
	{
		w = x;
	}

	int get_h() //used to retrieve processing dimensions
	{
		return h;
	}
	int get_w()
	{
		return w;
	}

	float calc_I_fromSqr( char*);//calculates I = Sqr(R2 + G2 + B2)

	void set_Lmax(int x)
	{
		L_max = x;

		//PUT BACK
		//delete [] CB;
		//CB = new codebook[w*h*L_max];
	}
	int get_Lmax()
	{
		return L_max;
	}

	void set_fmin(int param)
	{
		fmin = param;
	}
	float get_fmin()
	{
		return fmin;
	}

	void set_fminState(bool param)
	{
		fmin_active = param;
	}
	bool get_fminState()
	{
		return fmin_active;
	}

	void set_DeltaS(float param)
	{
		DeltaS = param;
	}
	float get_DeltaS()
	{
		return DeltaS;
	}

	float get_DeltaC()
	{
		return DeltaC;
	}
	void set_DeltaC(float x)
	{
		DeltaC = x;
	}

	float get_DeltaE()
	{
		return DeltaE;
	}
	void set_DeltaE(float x)
	{
		DeltaE = x;
	}

	void set_DeltaL(float x)
	{
		DeltaL = x;
	}
	float get_DeltaL()
	{
		return DeltaL;
	}

	float calculateAverageCWsPerPxl()
	{
		float sum = 0;
		for(int i = 0; i < w; i++)
		{
			for(int j = 0; j < h; j++)
			{
				sum += num_of_active_models_per_pixel[i + j*w];
			}
		}

		sum /= w*h;
		return sum;
	}

	int find_n_MNRL(int,int,int); // (x,y,n) finds the nth best MNRL for a pixel and returns its CW index
	bool Lab_colorDifference_is_within(CvScalar,CvScalar); //takes a pixel in Lab, and returns true if its less than DeltaE (2.3 by default)
	bool Lab_colorChromaticityDifference_is_within(CvScalar, CvScalar);//takes a pixel in Lab, and returns true if chromaticity its less than DeltaC (2.3 by default)
	bool Lab_colorLuminanceDifference_is_within(CvScalar, CvScalar); //takes a pixel in Lab, and returns true if Luminance its less than DeltaC (2.3 by default)
	float Lab_colorDifference(CvScalar,CvScalar); //takes a pixel in Lab, and returns the color difference
	float Lab_colorChromaticityDifference(CvScalar,CvScalar); //takes a pixel in Lab, and returns the color difference
	float Lab_colorLuminanceDifference(CvScalar,CvScalar); //takes a pixel in Lab, and returns the color difference
	void swapCWs(codebook*&, int, int); //swap the 2 CWs of pixel codebook* of orders int and int. 
	void deleteCW(codebook*& , int ); //deletes the CW at an index

	int  get_used_method() //0=Kim's 1=Lab+Kim
	{
		return used_method;
	}
	void  set_used_method(int param)//0=Kim's 1=Lab+Kim
	{
		used_method = param;
	}

	int get_depth() //the depth of the image in the model
	{
		return depth;
	}
	void set_depth(int param)
	{
		depth= param;
	}

	int get_nCh() //3 (RGB) 1 (Gray)
	{
		return nCh;
	}
	void set_nCh(int param)
	{
		nCh = param;
	}

	int get_widthStep()//the width step of teh processed video
	{
		return widthStep;
	}
	void set_widthStep(int param)
	{
		widthStep = param;
	}

	codebook* get_needed_CB(int, int); //(index of CB, required order) returns the pointer to point to the requested codebook of order (int) from best to worse

	bool get_updatedDisabledDuringSubtraction()
	{
		return updatedDisabledDuringSubtraction;
	} 
	void set_updatedDisabledDuringSubtraction(bool param)
	{
		updatedDisabledDuringSubtraction = param;
	}

	void NFPP_update_pass1(); //Corrects BGmask and assigns label numbers to BGlabels
	void NFPP_update_pass2(); //merges the labels

	//connectivity array functions
	void connectivityArray_setArea(int indx,int area) //index, area
	{
		connectivityArray[indx][1] = area;
	}
	int connectivityArray_getArea(int indx) //index
	{
		return connectivityArray[indx][1];
	}

	void connectivityArray_setParent(int indx,int parent) //index, parent
	{
		connectivityArray[indx][0] = parent;
	}
	int connectivityArray_getParent(int indx) //index
	{
		return connectivityArray[indx][0];
	}

	int newIndexObject()
	{	
		++indexOfNextObject;
		return indexOfNextObject; //return then increment
	}

	int findUltimateParent(int); //returns the topmost parent of the index
	int label_union(int,int); //7unifies the aprent of 2 lables and returns the new parent

	void set_minArea(int a)
	{	
		minArea = a;
	}

	void set_maxArea(int a)
	{	
		maxArea = a;
	}

	int get_minArea()
	{
		/*CvBlobSeq* newBlobList;
		CvBlobDetector* blobDetect = cvCreateBlobDetectorCC();
		blobDetect->DetectNewBlob();
		CvBlob* j;
		j->*/
		return minArea;
	}

	int get_maxArea()
	{
		/*CvBlobSeq* newBlobList;
		CvBlobDetector* blobDetect = cvCreateBlobDetectorCC();
		blobDetect->DetectNewBlob();
		CvBlob* j;
		j->*/
		return maxArea;
	}

	void resetConnectivityArray();




	////Blob Detection and tracking
	list<TrackerObject*> trackerBlobs; //maintained by the tracker
	list<ConnectedComponent*> BGSubtractionBlobs; //used per frame for BG subtraction
	double CC_Obj_max_acceptable_distance_intersection, CC_Obj_max_acceptable_distance_onetoone; //if less than that, then the object is deemed invisible.
	//int MAX_NUM_OF_BLOBS; 
	int nextCollisionGroup; //holds the number of occlusion groups (reset every frame)
	int nextID; //to give new objects new IDs
	void findBGSubtractionBlobs(int*);
	float calculatePrecentageOverlappingArea(CvBlob* reference, CvBlob* OtherBlob); 
	void correlateBlobs(); // correlates the tracker's blobs and BGSubtraction Blobs
	int offset_contour; // the offset for outer and inner contours
	int nextPersonGroup; //used for object classification based on the paper by Bird et al. "Real time online detetction.."
	void blobPreprocessing(); //assigns blob properties and does contour contrast
	void deleteBoarderObjects();
	void deleteNonvisibleNonpersistentObjects();
	void deleteBGSubtractionBlobs();
	void resetObjectHistory();
	void ObjectSplitting();
	void ObjectMerging();
	void oneToOne();
	void createNewObjects();
	void deleteAbsentObjects();
	void updateNonVisibleObjects();
	void updateObjectsKalman();
	void updateObjectsLastHistory();
	void assignIDsToHpsAndMps();
	//auxuliary
	void rearrangeParentsAfterSplit();
	void deleteAllsiblingsMatched();
	void ApplyPendingSplits();
	double absenceFrames(TrackerObject* );
	bool pixelIntersect(TrackerObject*,ConnectedComponent*, bool); //returns true when the dominant CC within the object is the give CC
	float pixelIntersectratio(TrackerObject*,ConnectedComponent*, bool );
	bool pixelToPixelIntersect(TrackerObject* ,ConnectedComponent* , bool );
	void fillCandidateOwners(TrackerObject*);

	fstream objectFile; //debugs history construction
	fstream objectHistoryFile; //for each frame gives the full history
	fstream ObjectProcessingFile; //describes merge - split - correlation..etc




	////we need two correlation tables because both entries could have non associated values
	//list<ConnectedComponent*> getCCLsit()
	//{
	//	return BGSubtractionBlobs;
	//}

	//feedback
	void feedback();
	int OBJ_CONFIDENCE_TH; // if an object stays for longer than this time, then it's a stationary foreground object (smaller than bgmodel.N and bgmodel.f) (used in feedback)
	bool* stationary_object; //if true for a pixel, then that pixel appears as forgeround although it has a reliable codeword for it.
	IplImage* original_BG; // if true, then it is an indication that it is not a foreground pixel originally but rather was extended by morphology






	//Thresholds
	float MOTION_TOLERANCE; //used to distinguish moving from nonmoving objects
	float TYPE_THRESHOLD; // a heuristic measure: < 0.25 => bag, else: person
	int MAX_ABSENCE; //maximum object absence allowed 
	int PERSISTENCE_THRESHOLD; // the amount of time required before an object is considered reliable
	float AVERAGE_PERSON_HEIGHT; //used for xTop, yTop calculations
	float FALLING_TOLERANCE; // the max x-y distance acceptable between the head and feet
	
	







private:

	codebook** CB; //points to an array of HxW x L_max
	short int*  num_of_active_models_per_pixel; // a HxW array representing L at each pixel
	int h; //height
	int w; //width
	float epsilon1; //used to compare colordist function 
	float epsilon2; //used for background subtraction
	int t; //time = number of frame
	double* spc_medians; //the N spacial medians (3 colors) stored incrementally as time progresses
	float k; //the coefficient which is multiplied by epsilon1 in Kim's thesis
	float k2; //the coefficient which is multiplied by epsilon2 in Kim's thesis
	float alpha;
	float beta;
	bool fmin_active; //a proposed improvement over the algorithm to incorporate frequency too (not only MNRL)
	int N; //number of frames used for training (def: 400)
	int TM; //number of frames used as a threshold (def: 1/2 N)
	int fmin; //the shortest acceptable access time for a CB
	int displayed_height; //the displayed resolution
	int displayed_width; 
	int L_max; //specifies the maximum number of CW per pixel
	float DeltaE; //used to specify the acceptable color difference in Lab.
	float DeltaC; //used to specify the acceptable color chromaticity difference in Lab.
	float DeltaL; //used to specify the acceptable color Luminance difference in Lab.
	float DeltaS; //used to specify the acceptable color Luminance difference for a shadow in Lab.
	int used_method; //0=Kim's 1=Lab+Kim
	int depth; //same as the depth of the image
	int nCh; //3 (RGB) or 1 (Gray)
	int widthStep; //the width step of teh processed video
	bool updatedDisabledDuringSubtraction; //used when a saved codebook is loaded;
	codebook* var_for_return; //used by functions to return pointers (never used by user)

	//labeling
	int** connectivityArray; // 2 rows (1st index= child, 2nd index: 0=parent, 1= area ) //check An Improved Real-time Blob Detection for Visual Surveillance
	int indexOfNextObject;
	int minArea;
	int maxArea;







};

#endif
