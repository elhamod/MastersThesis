#include "stdafx.h"
#include "BGmodel.h"
#include "median.h"
#include "g_variables.h"
#include "Tracker.h"
#include <cv.h>
#include <cvaux.h>
#include <highgui.h>
#include <iterator>
#include "cameraModel.h"
#include "xmlUtil.h"

//blobs
#define MAX_NUM_OF_BLOBS 10
#define	BLOBDETECTIONARRAYSIZES 200
#define	OVERLAP_THRESHOLD 33 // percent

codebook::codebook()
{
	next_CW = NULL;
}

BG_codebook_model::BG_codebook_model() //initializes the size of the model //not gonna be used usually
{

}
BG_codebook_model::~BG_codebook_model()
{
	//delete all dynamically allocated arrays
	delete [] num_of_active_models_per_pixel;
	delete [] CB;
	delete [] spc_medians;

	list<TrackerObject*>::iterator ptrObj;
	list<ConnectedComponent*>::iterator ptrCC;

	//delete [] trackerBlobCorrelationTable; 
	//delete [] BGBlobCorrelationTable; 
	for(ptrCC= BGSubtractionBlobs.begin(); ptrCC != BGSubtractionBlobs.end(); ptrCC++)
		delete (*ptrCC);
	BGSubtractionBlobs.clear();
	for(ptrObj= trackerBlobs.begin(); ptrObj != trackerBlobs.end(); ptrObj++)
		delete (*ptrObj);
	trackerBlobs.clear();


	//stationary object
	delete[] stationary_object;
	//delete[] original_BG;





}

BG_codebook_model::BG_codebook_model(int height, int width) //initializes the size of the model
{
	h = height;
	w = width;
	nCh = glbl_var.currentFrame->nChannels;
	depth = glbl_var.currentFrame->depth;
	widthStep = glbl_var.currentFrame->widthStep;
	L_max = 30;
	t=1; //first frame
	int frame_size = height*width;
	num_of_active_models_per_pixel = new short[frame_size]; //references row by row

	//initialize the model with zero codebooks
	for (int i=0; i<frame_size; i++)
		num_of_active_models_per_pixel[i] = 0;

	CB = new codebook*[frame_size];
	for (int i=0; i <frame_size; i++)
		CB[i] = new codebook();

	//default values for parameters
	k= 4.5;
	k2 = 4.5;
	alpha = 0.5;
	beta = 1.2;
	N = 5;
	TM = 250;
	epsilon1 = 1;
	epsilon2 = 1;
	DeltaE = 2.3;
	DeltaL = 2;
	//DeltaL = 12;
	DeltaS = 1.1;
	//DeltaS = 3;
	DeltaC = 12;
	fmin = 2;
	fmin_active = true;
	used_method = 0; //Kim's
	t=0;

	TYPE_THRESHOLD = 0.22;
	nextCollisionGroup = 1;
	nextID = 0;
	offset_contour = 3;


	nextPersonGroup = 1;

	//post processing parameters
	int tempsize = w*h/4;
	connectivityArray = new int*[tempsize];
	for (int i=0; i <w*h; i++)
		connectivityArray[i] = new int[2];
	minArea = 100; //0
	maxArea = 30000; //w*h
	for (int i=0; i <tempsize; i++)
	{
		connectivityArray[i][0] = -1; //no parent
		connectivityArray[i][1] = 0; //no area
	}
	indexOfNextObject = 0;

	set_spc_medians_size(3000);

	//stationary object
	stationary_object = new bool[w*h];
	//original_BG = new bool[w*h];
	original_BG = NULL;
	for (int i=0; i < w*h; i++)
	{
		stationary_object[i] = false;
		//original_BG[i] = false;
	}



}

void BG_codebook_model::update_medians()
{
	//arrays containing the channels seperately
	double* spacial_mdn = new double[h*w];
	int tmp_cntr = h*w*glbl_var.currentFrame->nChannels;
	int spacial_mdn_cnt = 0;
	CvScalar tmp_param1;
	CvScalar tmp_param2;


	int ptr_indx = 0;
	char* ptr = glbl_var.currentFrame->imageData;
	char* ptr2 = glbl_var.previousFrame->imageData;

	//take the difference between current and previous frame, and store it in the spatial median
	for (int cnt_y=0; cnt_y<h; cnt_y++)
		for (int cnt_x=0; cnt_x<w; cnt_x++)
		{
			tmp_param1 = cvScalar(ptr[ptr_indx],ptr[ptr_indx+1],ptr[ptr_indx+2]);
			tmp_param2 = cvScalar(ptr2[ptr_indx],ptr2[ptr_indx+1],ptr2[ptr_indx+2]);
			spacial_mdn[spacial_mdn_cnt] = colordist(tmp_param1,tmp_param2);
			spacial_mdn_cnt++;
			ptr_indx+=3;
		}



		//assign the spatial median 
		if (t >= 2)
		{
			spc_medians[t-2] = find_median(h*w, spacial_mdn);//the medians should be found starting from t=2 because t=0 doesn't exist and t=1 has only a current frame
		}
		delete [] spacial_mdn;

}

void BG_codebook_model::calc_epsilons()
{
	//epsilon1 = 4.5(median) Kim's Dissertation
	//epsilon2 = epsilon1.

	epsilon1 = k*find_median(t-1, spc_medians);//the medians should be found starting from t=2 because t=0 doesn't exist and t=1 has only a current frame
	epsilon2 = k2*epsilon1/k;
}

void BG_codebook_model::BG_subtraction(IplImage* img)
{
	//Kim et al paper



	//Kim et al paper 





	//only after training period
	if ((t > N) && glbl_var.seperateTrainTest ) 
	{
		int frame_size = h*w;

		char* pixel_ptr = img->imageData;
		float I; //I is used to construct the codebook
		CvScalar x; //x is used to construct the codebook
		codebook* CB_ptr = CB[0];

		int i=0;
		for (int cnt_y=0; cnt_y<h; cnt_y++)
			for (int cnt_x=0; cnt_x<w; cnt_x++)
			{
				//(i)






				if (used_method == KimOriginal)
					x = cvScalar((double)(unsigned char)pixel_ptr[0],(double)(unsigned char)pixel_ptr[1],(double)(unsigned char)pixel_ptr[2],0); //assigning the RGB values from the frame
				if (used_method == LabSpherical || used_method == LabCylindrical)
				{
					x = cvScalar((double) (unsigned char)pixel_ptr[0],(double) (unsigned char)pixel_ptr[1],(double) (unsigned char)pixel_ptr[2],0); //assigning the RGB values from the frame
					x = glbl_var.display2real_Lab(x);
				}


				if (used_method == KimOriginal) I = calc_I_fromSqr(pixel_ptr);

				//(ii)
				codebook* Matching_CB_ptr = CB_ptr; //used to traverse the CB in the L_max direction
				bool Matching_CB_ptr_found = 0;

				for (int j=0; j < num_of_active_models_per_pixel[i]; j++)
					//while (Matching_CB_ptr)
				{

					if (used_method == KimOriginal) //Kim RGB
					{
						if (colordist(x, Matching_CB_ptr->getRGB()) <= epsilon1) //1st condition
							if(brightness(I,Matching_CB_ptr->getImin(), Matching_CB_ptr->getImax()))//2nd condition
							{



								if ( Matching_CB_ptr->getMNRL() <= TM && (Matching_CB_ptr->getCW_freq() >= fmin || !fmin_active)) //the condition corresponding to M in the paper
								{

									Matching_CB_ptr_found = 1; //Matching_CB_ptr will be pointing to teh matched CW

									//put the one with highest freq first
									if (Matching_CB_ptr->getCW_freq() > CB_ptr->getCW_freq())
									{
										glbl_var.BGmodel->swapCWs(CB[i],0,j);
										Matching_CB_ptr = CB[i];
									}

									break; //the code word is found if both conditions are satisfied.
								}
							}
							//Matching_CB_ptr++; //to jump from one pixel to another




					}
					else if (used_method == LabSpherical) //Kim Lab
					{

						if(Lab_colorDifference_is_within(x, Matching_CB_ptr->getLab()))//
						{
							if (Matching_CB_ptr->getMNRL() <= TM && (Matching_CB_ptr->getCW_freq() >= fmin || !fmin_active)) //the condition corresponding to M in the paper
							{

								if ( Matching_CB_ptr->getMNRL() <= TM && (Matching_CB_ptr->getCW_freq() >= fmin || !fmin_active)) //the condition corresponding to M in the paper
								{
									Matching_CB_ptr_found = 1; //Matching_CB_ptr will be pointing to teh matched CW

									//put the one wirth highest freq first
									if (Matching_CB_ptr->getCW_freq() > CB_ptr->getCW_freq())
									{
										glbl_var.BGmodel->swapCWs(CB[i],0,j);
										Matching_CB_ptr = CB[i];
									}

									break; //the code word is found if both conditions are satisfied.
								}
							}
						}
						//Matching_CB_ptr++; //to jump from one pixel to another

					}
					else if (used_method == LabCylindrical)
					{

						if(Lab_colorChromaticityDifference_is_within(x, Matching_CB_ptr->getLab()))//
							if(Lab_colorLuminanceDifference_is_within(x, Matching_CB_ptr->getLab()))//
							{

								if ( Matching_CB_ptr->getMNRL() <= TM && (Matching_CB_ptr->getCW_freq() >= fmin || !fmin_active)) //the condition corresponding to M in the paper
								{
									Matching_CB_ptr_found = 1; //Matching_CB_ptr will be pointing to teh matched CW


									//put the one wirth highest freq first
									if (Matching_CB_ptr->getCW_freq() > CB_ptr->getCW_freq())
									{
										glbl_var.BGmodel->swapCWs(CB[i],0,j);
										Matching_CB_ptr = CB[i];
									}

									break; //the code word is found if both conditions are satisfied.
								}
							}
							//Matching_CB_ptr++; //to jump from one pixel to another

					}

					//added to update MNRL for all codewords (not just the updated one)
					/*			int tempLastaccess = Matching_CB_ptr->getlast_access();
					if ((t - tempLastaccess) > Matching_CB_ptr->getMNRL()) Matching_CB_ptr->setMNRL(t - tempLastaccess);*/



					//tried to test (TM*(num_of_active_models_per_pixel[i]/3+1)


					if (Matching_CB_ptr->get_next_CW() == NULL)
						Matching_CB_ptr->set_next_CW(new codebook());
					Matching_CB_ptr = Matching_CB_ptr->get_next_CW();

				}



				//add a new CB if no match is found



				//(iii)


				if (Matching_CB_ptr_found)//a match is found 
				{
					if (!updatedDisabledDuringSubtraction)
					{


						//fetch the needed in_parameters beforehand
						if (used_method == 0) float Matching_Imax = Matching_CB_ptr->getImax();
						if (used_method == 0) float Matching_Imin = Matching_CB_ptr->getImin();
						int Matching_CW_freq = Matching_CB_ptr->getCW_freq();
						//int Matching_MNRL = Matching_CB_ptr->getMNRL();
						//int Matching_first_access = Matching_CB_ptr->getfirst_access();
						//int Matching_last_access = Matching_CB_ptr->getlast_access();

						if (used_method == 0) 
						{
							CvScalar Matching_RGB = Matching_CB_ptr->getRGB();
							CvScalar temp_RGB = {(Matching_CW_freq * Matching_RGB.val[0] + x.val[0])/(Matching_CW_freq + 1),
								(Matching_CW_freq * Matching_RGB.val[1] + x.val[1])/(Matching_CW_freq + 1),
								(Matching_CW_freq * Matching_RGB.val[2] + x.val[2])/(Matching_CW_freq + 1)};
							Matching_CB_ptr->setRGB(temp_RGB);
							if ( I < Matching_CB_ptr->getImin()) Matching_CB_ptr->setImin(I);
							if ( I > Matching_CB_ptr->getImax()) Matching_CB_ptr->setImax(I);
						}
						else if (used_method == 1 || used_method == 2)
						{
							CvScalar Matching_Lab = Matching_CB_ptr->getLab();
							CvScalar temp_Lab = {(Matching_CW_freq * Matching_Lab.val[0] + x.val[0])/(Matching_CW_freq + 1),
								(Matching_CW_freq * Matching_Lab.val[1] + x.val[1])/(Matching_CW_freq + 1),
								(Matching_CW_freq * Matching_Lab.val[2] + x.val[2])/(Matching_CW_freq + 1)};
							Matching_CB_ptr->setLab(temp_Lab);
						}
						Matching_CB_ptr->setCW_freq(Matching_CW_freq + 1);
						//if ((t - Matching_last_access) > Matching_MNRL) Matching_CB_ptr->setMNRL(t - Matching_last_access);
						Matching_CB_ptr->setlast_access(t);
					}

					glbl_var.BGmask->imageData[i] = 0;



				}
				else //no more CW can be added, so it's a foreground
				{
					glbl_var.BGmask->imageData[i] = 255;
				}

				////added to update MNRL for all codewords (not just the updated one)
				//CB_ptr = CB[i];
				//int tempCBLength = num_of_active_models_per_pixel[i];
				//for (int k=0; k < tempCBLength; k++)
				//{
				//	int tempLastaccess = CB_ptr->getlast_access();
				//	if ((t - tempLastaccess) > CB_ptr->getMNRL()) CB_ptr->setMNRL(t - tempLastaccess);
				//	CB_ptr++;
				//}

				//pixel_ptr += 3; //becaure R G B R G B ...etc
				i++;
				//CB_ptr += L_max; //move to next pixel
				CB_ptr = CB[i];
				pixel_ptr+=  img->nChannels;

			}
	}






	//
	t++;//increment time



}

float BG_codebook_model::colordist(CvScalar x, CvScalar v)
{
	float x_mag_squared = x.val[0]*x.val[0] + x.val[1]*x.val[1] + x.val[2]*x.val[2];//using multiplication instead math pow for speed optimization
	float v_mag_squared = v.val[0]*v.val[0] + v.val[1]*v.val[1] + v.val[2]*v.val[2];//using multiplication instead math pow for speed optimization
	double temp = (x.val[0]*v.val[0] + x.val[1]*v.val[1] + x.val[2]*v.val[2]);
	float x_v_dot_product_squared = temp*temp;//using multiplication instead math pow for speed optimization
	float p_squared = x_v_dot_product_squared/v_mag_squared;
	return cvSqrt(abs(x_mag_squared - p_squared));

}

codebook* BG_codebook_model::getCodebook(int x,int y,int d) //given the x and y and the index of the codeword
{
	//int indx = w*L_max*y + x*L_max + d; 
	//if (num_of_active_models_per_pixel[w*y + x] < (d+1)) //checking whether the CB actually exists
	//	return NULL;
	//else
	//	return (CB + indx);
	var_for_return = CB[(x+ y*w)] ;
	for (int i=0; i<d; i++)
	{
		codebook* nxt = var_for_return->get_next_CW();
		if (nxt == NULL)
			return NULL;
		else
			var_for_return = var_for_return->get_next_CW();
	}
	return var_for_return;
}

codebook* BG_codebook_model::force_getCodebook(int x,int y,int d) //used in CB read ONLY: given the x and y and the index of the codeword
{
	/*int indx = w*L_max*y + x*L_max + d; 
	return (CB + indx);*/
	var_for_return = CB[(x+ y*w)] ;
	if (!var_for_return)
		var_for_return = new codebook();
	else
	{
		for (int i=0; i<d; i++)
		{
			codebook* nxt = var_for_return->get_next_CW();
			if (nxt == NULL)
			{
				codebook* new_CW = new codebook();
				var_for_return->set_next_CW(new_CW);
			}
			//else
			var_for_return = var_for_return->get_next_CW();
		}
	}
	return var_for_return;
}

bool BG_codebook_model::brightness(float intensity, float intensity_min , float intensity_max)
{
	float Ilow = get_Ilow(intensity_max);
	float Ihigh= get_Ihigh(intensity_min,intensity_max);

	if (intensity <= Ihigh && intensity >= Ilow)
		return 1;
	else
		return 0;
}

short* BG_codebook_model::getNum_of_active_models_per_pixel()
{
	return num_of_active_models_per_pixel;
}

void BG_codebook_model::update_model(IplImage* img)
{
	//Kim et al paper 





	//
	//(part II)
	//first N frames for training if seperate train/test stages or indefinitely if not
	if (t <= N || !glbl_var.seperateTrainTest) 
	{
		int frame_size = h*w;

		char* pixel_ptr = img->imageData;
		float I; //I is used to construct the codebook
		CvScalar x; //x is used to construct the codebook
		codebook* CB_ptr = CB[0];


		////ALERT ADDING FEEDBACK FROM TRACKING SO A PERSON WOULD NOT UPDATE THE BACKGROUND
		//list<TrackerObject*>::iterator ptrObj;
		//IplImage* PersonMask = cvCreateImage(cvSize(w,h),8,1);
		//cvZero(PersonMask);
		//for (ptrObj= trackerBlobs.begin(); ptrObj != trackerBlobs.end(); ptrObj++)
		//{
		//	if(!(*ptrObj)->isVisible) continue;//skip nonvisible
		//	for (int cnt_y=(*ptrObj)->blob->y; cnt_y<(*ptrObj)->blob->y + (*ptrObj)->blob->h; cnt_y++)
		//		for (int cnt_x=(*ptrObj)->blob->x; cnt_x<(*ptrObj)->blob->x + (*ptrObj)->blob->w; cnt_x++)
		//		{
		//			bool NotBG = (glbl_var.BGmask->imageData[cnt_x + w*cnt_y] != 0);
		//			bool isPerson = ((*ptrObj)->type == 1) ;
		//			//if it's within the bounding box, a person, and not part of the background
		//			if ( NotBG && isPerson)
		//				PersonMask->imageData[cnt_x + w*cnt_y] = 1;
		//		}
		//}


		int i=0;
		for (int cnt_y=0; cnt_y<h; cnt_y++)
			for (int cnt_x=0; cnt_x<w; cnt_x++)
			{



				//(i)


				if (used_method == KimOriginal)
					x = cvScalar((double)(unsigned char)pixel_ptr[0],(double)(unsigned char)pixel_ptr[1],(double)(unsigned char)pixel_ptr[2],0); //assigning the RGB values from the frame
				if (used_method == LabSpherical || used_method == LabCylindrical)
				{
					x = cvScalar((double) (unsigned char)pixel_ptr[0],(double) (unsigned char)pixel_ptr[1],(double) (unsigned char)pixel_ptr[2],0); //assigning the RGB values from the frame
					x = glbl_var.display2real_Lab(x);
				}


				if (used_method == KimOriginal) I = calc_I_fromSqr(pixel_ptr);

				//(ii)
				codebook* Matching_CB_ptr = CB_ptr; //used to traverse the CB in the L_max direction
				bool Matching_CB_ptr_found = 0;
				bool CW_reliable = 0; //used because during update period, we just need to add CW, and yet show it's still FG


				for (int j=0; j < num_of_active_models_per_pixel[i]; j++)
					//while (Matching_CB_ptr)
				{

					if (used_method == KimOriginal) //Kim RGB
					{
						if (colordist(x, Matching_CB_ptr->getRGB()) <= epsilon1) //1st condition
							if(brightness(I,Matching_CB_ptr->getImin(), Matching_CB_ptr->getImax()))//2nd condition
							{

								//added to update MNRL for all codewords (not just the updated one)
								int tempLastaccess = Matching_CB_ptr->getlast_access();
								if ((t - tempLastaccess) > Matching_CB_ptr->getMNRL()) 
									Matching_CB_ptr->setMNRL(t - tempLastaccess);



								if (t - Matching_CB_ptr->getfirst_access() >= N && Matching_CB_ptr->getMNRL() <= TM && (Matching_CB_ptr->getCW_freq() >= fmin || !fmin_active)) //the condition corresponding to M in the paper
									CW_reliable = 1;
								Matching_CB_ptr_found = 1; //Matching_CB_ptr will be pointing to teh matched CW

								//put the one with highest freq first
								if (Matching_CB_ptr->getCW_freq() > CB_ptr->getCW_freq())
								{
									glbl_var.BGmodel->swapCWs(CB[i],0,j);
									Matching_CB_ptr = CB[i];

									j =0;
								}

								//break; //the code word is found if both conditions are satisfied.
							}
							//Matching_CB_ptr++; //to jump from one pixel to another




					}
					else if (used_method == LabSpherical) //Kim Lab
					{
						//added to update MNRL for all codewords (not just the updated one)
						int tempLastaccess = Matching_CB_ptr->getlast_access();
						if ((t - tempLastaccess) > Matching_CB_ptr->getMNRL()) 
							Matching_CB_ptr->setMNRL(t - tempLastaccess);


						if(Lab_colorDifference_is_within(x, Matching_CB_ptr->getLab()))//
						{
							if (t - Matching_CB_ptr->getfirst_access() >= N && Matching_CB_ptr->getMNRL() <= TM  && (Matching_CB_ptr->getCW_freq() >= fmin || !fmin_active)) //the condition corresponding to M in the paper
								CW_reliable = 1;
							Matching_CB_ptr_found = 1; //Matching_CB_ptr will be pointing to teh matched CW

							//put the one wirth highest freq first
							if (Matching_CB_ptr->getCW_freq() > CB_ptr->getCW_freq())
							{
								glbl_var.BGmodel->swapCWs(CB[i],0,j);
								Matching_CB_ptr = CB[i];

								j =0;
							}

							//break; //the code word is found if both conditions are satisfied.
						}
						//Matching_CB_ptr++; //to jump from one pixel to another

					}
					else if (used_method == LabCylindrical)
					{

						if(Lab_colorChromaticityDifference_is_within(x, Matching_CB_ptr->getLab()))//
							if(Lab_colorLuminanceDifference_is_within(x, Matching_CB_ptr->getLab()))//
							{

								//added to update MNRL for all codewords (not just the updated one)
								int tempLastaccess = Matching_CB_ptr->getlast_access();
								if ((t - tempLastaccess) > Matching_CB_ptr->getMNRL()) 
									Matching_CB_ptr->setMNRL(t - tempLastaccess);




								if (t - Matching_CB_ptr->getfirst_access() >= N && Matching_CB_ptr->getMNRL() <= TM  && (Matching_CB_ptr->getCW_freq() >= fmin || !fmin_active)) //the condition corresponding to M in the paper
									CW_reliable = 1;
								Matching_CB_ptr_found = 1; //Matching_CB_ptr will be pointing to teh matched CW

								//put the one with highest freq first
								if (Matching_CB_ptr->getCW_freq() > CB_ptr->getCW_freq())
								{
									glbl_var.BGmodel->swapCWs(CB[i],0,j);
									Matching_CB_ptr = CB[i];

									j =0;
								}

								//break; //the code word is found if both conditions are satisfied.
							}
							//Matching_CB_ptr++; //to jump from one pixel to another

					}

					////added to update MNRL for all codewords (not just the updated one)
					//int tempLastaccess = Matching_CB_ptr->getlast_access();
					//if ((t - tempLastaccess) > Matching_CB_ptr->getMNRL()) 
					//	Matching_CB_ptr->setMNRL(t - tempLastaccess);





					//tried to test (TM*(num_of_active_models_per_pixel[i]/3+1)

					if (!glbl_var.seperateTrainTest)
					{
						//delete codeword that has an MNRL higher than TM if the aged-enough period FOR THIS CODEWORD IS OVER
						if (Matching_CB_ptr->getMNRL() > TM && t - Matching_CB_ptr->getfirst_access() >= N ) //age condition, periodicity condition
						{


							if (Matching_CB_ptr->get_next_CW() == NULL) //if last one don't delete it but rather overwrite it
							{
								num_of_active_models_per_pixel[i]--;
								Matching_CB_ptr_found = false;
								continue;
							}
							else
							{

								Matching_CB_ptr = Matching_CB_ptr->get_next_CW();
								deleteCW(CB[i],j);
								num_of_active_models_per_pixel[i]--;
								j--;
								Matching_CB_ptr_found = false;


							}




						}
						else
						{
							if (Matching_CB_ptr_found)
								break;

							// create new if at the tail and move to the next one anyway
							//prepare for adding a new codeword at the end.
							if (Matching_CB_ptr->get_next_CW() == NULL )
								Matching_CB_ptr->set_next_CW(new codebook());
							Matching_CB_ptr = Matching_CB_ptr->get_next_CW();
						}

					}
					else
					{

						if (Matching_CB_ptr_found)
							break;

						// create new if at the tail and move to the next one anyway
						//prepare for adding a new codeword at the end.
						if (Matching_CB_ptr->get_next_CW() == NULL )
							Matching_CB_ptr->set_next_CW(new codebook());
						Matching_CB_ptr = Matching_CB_ptr->get_next_CW();
					}




				}


				////ALERT : ADDED FOR PERSON CASE
				//if (PersonMask->imageData[cnt_x + w*cnt_y] == 1)
				//{
				//	glbl_var.BGmask->imageData[i] = 255;
				//	continue;
				//}


				//add a new CB if no match is found



				//(iii)

				if (!Matching_CB_ptr_found && num_of_active_models_per_pixel[i] < L_max) //if no match is found
				{
					num_of_active_models_per_pixel[i]++;//L = L+1
					if (used_method == KimOriginal) 
						Matching_CB_ptr->setRGB(x);
					else if (used_method == LabSpherical || used_method == LabCylindrical) 
						Matching_CB_ptr->setLab(x);
					if (used_method == KimOriginal) Matching_CB_ptr->setImax(I);
					if (used_method == KimOriginal) Matching_CB_ptr->setImin(I);
					Matching_CB_ptr->setCW_freq(1);

					if (glbl_var.seperateTrainTest)
						Matching_CB_ptr->setMNRL(t-1);
					else
						Matching_CB_ptr->setMNRL(0);

					Matching_CB_ptr->setfirst_access(t);
					Matching_CB_ptr->setlast_access(t);

					glbl_var.BGmask->imageData[i] = 255;

				}
				else if (Matching_CB_ptr_found)//a match is found 
				{



					//fetch the needed in_parameters beforehand
					if (used_method == 0) float Matching_Imax = Matching_CB_ptr->getImax();
					if (used_method == 0) float Matching_Imin = Matching_CB_ptr->getImin();
					int Matching_CW_freq = Matching_CB_ptr->getCW_freq();
					//int Matching_MNRL = Matching_CB_ptr->getMNRL();
					int Matching_first_access = Matching_CB_ptr->getfirst_access();
					int Matching_last_access = Matching_CB_ptr->getlast_access();
					if (Matching_first_access > t) Matching_CB_ptr->setfirst_access(t);
					if (Matching_last_access > t) Matching_CB_ptr->setlast_access(t);

					if (used_method == 0) 
					{
						CvScalar Matching_RGB = Matching_CB_ptr->getRGB();
						CvScalar temp_RGB = {(Matching_CW_freq * Matching_RGB.val[0] + x.val[0])/(Matching_CW_freq + 1),
							(Matching_CW_freq * Matching_RGB.val[1] + x.val[1])/(Matching_CW_freq + 1),
							(Matching_CW_freq * Matching_RGB.val[2] + x.val[2])/(Matching_CW_freq + 1)};
						Matching_CB_ptr->setRGB(temp_RGB);
						if ( I < Matching_CB_ptr->getImin()) Matching_CB_ptr->setImin(I);
						if ( I > Matching_CB_ptr->getImax()) Matching_CB_ptr->setImax(I);
					}
					else if (used_method == 1 || used_method == 2)
					{
						CvScalar Matching_Lab = Matching_CB_ptr->getLab();
						CvScalar temp_Lab = {(Matching_CW_freq * Matching_Lab.val[0] + x.val[0])/(Matching_CW_freq + 1),
							(Matching_CW_freq * Matching_Lab.val[1] + x.val[1])/(Matching_CW_freq + 1),
							(Matching_CW_freq * Matching_Lab.val[2] + x.val[2])/(Matching_CW_freq + 1)};
						Matching_CB_ptr->setLab(temp_Lab);
					}
					Matching_CB_ptr->setCW_freq(Matching_CW_freq + 1);
					//if ((t - Matching_last_access) > Matching_MNRL) Matching_CB_ptr->setMNRL(t - Matching_last_access);
					Matching_CB_ptr->setlast_access(t);

					if (CW_reliable) //if the found CW reliable (MNRL and/or fmin) then show as BG, otherwise show as FG
						glbl_var.BGmask->imageData[i] = 0;
					else
						glbl_var.BGmask->imageData[i] = 255;



				}
				else //no more CW can be added, so it's a foreground
				{
					glbl_var.BGmask->imageData[i] = 255;
				}


				//incrementing pointers

				//pixel_ptr += 3; //becaure R G B R G B ...etc
				i++;
				//CB_ptr += L_max; //move to next pixel
				CB_ptr = CB[i];
				pixel_ptr+=  img->nChannels;
			}



			//t++;//increment time
	}





	////TEST
	//int iii=0;
	//for (int cnt_y=0; cnt_y<h; cnt_y++)
	//	for (int cnt_x=0; cnt_x<w; cnt_x++)
	//	{
	//		int DummyCount =0;
	//		codebook* ptrDummy = CB[cnt_x+w*cnt_y];
	//		while (ptrDummy)
	//		{
	//			ptrDummy = ptrDummy->get_next_CW();
	//			DummyCount++;
	//		}
	//		if (num_of_active_models_per_pixel[iii] != DummyCount)
	//		{
	//			if (DummyCount != L_max)
	//			{
	//				int x;
	//				x=0;
	//			}
	//		}
	//		iii++;
	//	}

	//}

	if ((t == N) &&  glbl_var.seperateTrainTest) // wrapping in case we have seperate training and testing periods
	{
		//Part III (wrapping MNRL around)
		//int CB_size= h*w*L_max;
		int CB_size = h*w;

		codebook* CB_ptr = CB[0];
		for (int i=0; i<CB_size; i++)
		{
			CB_ptr = CB[i];
			codebook* temp = CB[i];
			while (temp)
				//for (int j=0; j < num_of_active_models_per_pixel[i]; j++)
			{
				if ( (N- 1 + CB_ptr->getfirst_access() - CB_ptr->getlast_access()) > CB_ptr->getMNRL()) CB_ptr->setMNRL(N- 1 + CB_ptr->getfirst_access() - CB_ptr->getlast_access());
				temp = temp->get_next_CW();
			}

		}
	}
	//else (t > N) the background is updated by means of background subtraction



}

void BG_codebook_model::reset_model_dimensions(int x, int y)
{
	BG_codebook_model(x,y);
}

float BG_codebook_model::calc_I_fromSqr( char* p)
{
	double temp1 = (double) (unsigned char) p[0];
	double temp2 = (double) (unsigned char) p[1];
	double temp3 = (double) (unsigned char) p[2];
	float result = cvSqrt(temp1*temp1 + temp2*temp2 + temp3*temp3);//using multiplication instead math pow for speed optimization
	return result;
}

bool BG_codebook_model::Lab_colorDifference_is_within(CvScalar pxl, CvScalar ref) //takes a pixel in Lab, and returns true if its less than DeltaE (2.3 by default)
{
	double temp1 = pxl.val[0] - ref.val[0];
	double temp2 = pxl.val[1] - ref.val[1];
	double temp3 = pxl.val[2] - ref.val[2];
	if (cvSqrt(temp1*temp1 + temp2*temp2 + temp3*temp3) <= DeltaE)
		return 1;
	else
		return 0;
}

bool BG_codebook_model::Lab_colorChromaticityDifference_is_within(CvScalar pxl, CvScalar ref) //takes a pixel in Lab, and returns true if its less than DeltaE (2.3 by default)
{
	double temp1 = pxl.val[1] - ref.val[1];
	double temp2 = pxl.val[2] - ref.val[2];
	//double temp3 = pxl.val[0] - ref.val[0];
	if (cvSqrt(temp1*temp1 + temp2*temp2) <= DeltaC )//tried *(3-temp3/50) //tried || pxl.val[0] < 15
		return 1;
	else
		return 0;
}

bool BG_codebook_model::Lab_colorLuminanceDifference_is_within(CvScalar pxl, CvScalar ref) //takes a pixel in Lab, and returns true if its less than DeltaE (2.3 by default)
{
	//if ( abs(pxl.val[0] - ref.val[0]) <= DeltaL)


	//float reciprocal = (pxl.val[0] > ref.val[0]) ? pxl.val[0]/(ref.val[0]+1) : ref.val[0]/(pxl.val[0]+1);
	float reciprocal = (pxl.val[0] > ref.val[0]) ? (pxl.val[0]+DeltaL)/(ref.val[0]+DeltaL) : (ref.val[0]+DeltaL)/(pxl.val[0]+DeltaL);
	//resolve cases of division by 0)
	/*if ( float::IsNaN(reciprocal)) 
	reciprocal = 0;
	if (float::IsInfinity(reciprocal) ) 
	reciprocal = 100;*/

	if ( reciprocal < DeltaL ) //tried || (pxl.val[0] < 15 && ref.val[0] < 15)
		return 1;
	else
		return 0;
}



float BG_codebook_model::Lab_colorDifference(CvScalar pxl,CvScalar ref)
{
	double temp1 = pxl.val[0] - ref.val[0];
	double temp2 = pxl.val[1] - ref.val[1];
	double temp3 = pxl.val[2] - ref.val[2];
	return cvSqrt(temp1*temp1 + temp2*temp2 + temp3*temp3);
}

float BG_codebook_model::Lab_colorChromaticityDifference(CvScalar pxl,CvScalar ref)
{
	double temp1 = pxl.val[1] - ref.val[1];
	double temp2 = pxl.val[2] - ref.val[2];
	return cvSqrt(temp1*temp1 + temp2*temp2);
}

float BG_codebook_model::Lab_colorLuminanceDifference(CvScalar pxl,CvScalar ref)
{
	//return abs(pxl.val[0] - ref.val[0]) ;
	//float reciprocal = (pxl.val[0] > ref.val[0]) ? (pxl.val[0]/(ref.val[0]+1)) : (ref.val[0]/(pxl.val[0]+1));
	float reciprocal = (pxl.val[0] > ref.val[0]) ? ((pxl.val[0]+DeltaL)/(ref.val[0]+DeltaL)) : ((ref.val[0]+DeltaL)/(pxl.val[0]+DeltaL));


	//if ( float::IsNaN(reciprocal)) 
	//	reciprocal = 0;
	//if (float::IsInfinity(reciprocal) ) 
	//	reciprocal = 100;

	return reciprocal;
}

codebook* BG_codebook_model::get_needed_CB(int indx,int order) //adjusts the pointer to point to the requested codebook of order from best to worse
{

	int pos;

	if (order+1 > num_of_active_models_per_pixel[indx])
		return NULL;
	double* MNRL_array = new double[L_max];
	//codebook* temp_ptr = CB + indx*L_max;
	var_for_return = CB[indx];

	for (int i=0; i < num_of_active_models_per_pixel[indx]; i++)
	{
		MNRL_array[i] = var_for_return->getCW_freq();
		//temp_ptr++;
		var_for_return = var_for_return->get_next_CW();
	}

	pos = find_n(num_of_active_models_per_pixel[indx] ,MNRL_array ,num_of_active_models_per_pixel[indx] - order);

	//retreive the desired CW
	var_for_return = CB[indx];
	for (int i=0; i<pos; i++)
		var_for_return = var_for_return->get_next_CW();

	delete [] MNRL_array;
	return  var_for_return;

}

//Corrects BGmask and assigns label numbers to BGlabels
void BG_codebook_model::NFPP_update_pass1()
{
	int neighborhood_size; //3, 5 or 8
	int num_FG_pxls;
	int smallest_neighbor_FG_pxl; //check the paper
	int highest_neighbor_FG_pxl; //check the paper
	int smallest_neighbor_BG_pxl; //improvement over the paper to fill in the small BG areas because they are likely to be noise
	int highest_neighbor_BG_pxl; //improvement over the paper to fill in the small BG areas because they are likely to be noise

	//first do a closing morphology
	if (glbl_var.numOfRepeating == 0  )
	{
		if (glbl_var.useMorphology)
		{
			int numberOfIterations = 2;
			IplImage* extended = cvCreateImage(cvSize(glbl_var.BGmask->width + numberOfIterations*2,glbl_var.BGmask->height+numberOfIterations*2),8,1);
			cvZero(extended);
			cvSetImageROI(extended,cvRect(numberOfIterations,numberOfIterations,glbl_var.BGmask->width,glbl_var.BGmask->height));
			cvCopy(glbl_var.BGmask,extended);
			cvResetImageROI(extended);
			//IplConvKernel* element = cvCreateStructuringElementEx(3,3,1,1,CV_SHAPE_RECT );
			cvDilate(extended,extended,0,numberOfIterations);
			cvErode(extended,extended,0,numberOfIterations);
			cvSetImageROI(extended,cvRect(numberOfIterations,numberOfIterations,glbl_var.BGmask->width,glbl_var.BGmask->height));
			cvCopy(extended,glbl_var.BGmask);
			cvReleaseImage(&extended);
		}
	}
	//cvReleaseStructuringElement(&element);

	int indx = 0;
	for (int cnt_y=0; cnt_y<h; cnt_y++)
		for (int cnt_x=0; cnt_x<w; cnt_x++)
		{


			if (glbl_var.numOfRepeating == 0  )
			{
				//check the size of the neighborhood for the pixels (sides or corners or inside)
				bool left = ( cnt_x == 0); bool right = (cnt_x == w-1); bool top = (cnt_y == 0); bool bottom = (cnt_y == h-1);
				if ( (left && top) || (left && bottom) || (right && top) || (right && bottom) ) // a corner
					neighborhood_size = 3; 
				else if (left || bottom || top || right) //on the sides
					neighborhood_size = 5; 
				else //inside
					neighborhood_size = 8;
				//calculate the number of foreground pixels in the neighborhood
				num_FG_pxls = 0;
			}


			int left_top_counter = 0; //used to check only the first 4 top left labels
			smallest_neighbor_FG_pxl = 0; //-1 means the highest and smallest are unknown yet
			highest_neighbor_FG_pxl = 0;
			smallest_neighbor_BG_pxl = 0; //+1 means the highest and smallest are unknown yet
			highest_neighbor_BG_pxl = 0;
			for (int k=-1; k <2; k++)
				for (int l=-1; l<2; l++)
				{

					left_top_counter++;
					int indx2 = (cnt_y+k)*w + (cnt_x+l);

					//skip the pixel itself
					if ( k==0 && l == 0)
						continue;

					//skip nonexisting pixels
					if ((cnt_x+l  <0) || (cnt_x+l >w-1) || (cnt_y+k <0) || (cnt_y+k >h-1))
						continue;

					if (glbl_var.numOfRepeating == 0  )
					{
						//calculate the number of FG pxls in the neighborhood
						if ( glbl_var.BGmask->imageData[indx2] != 0)
							num_FG_pxls++;
					}


					//check label
					//if (left_top_counter <=4) //just check labels at top left
					if (left_top_counter ==2 || left_top_counter ==4) //just check labels at top left
					{
						if (glbl_var.BGlabels[indx2] >0)
						{
							if (glbl_var.BGlabels[indx2] < smallest_neighbor_FG_pxl || smallest_neighbor_FG_pxl == 0) smallest_neighbor_FG_pxl =  glbl_var.BGlabels[indx2];
							if (glbl_var.BGlabels[indx2] > highest_neighbor_FG_pxl) highest_neighbor_FG_pxl =  glbl_var.BGlabels[indx2];
						}
						else
						{
							if (glbl_var.BGlabels[indx2] > smallest_neighbor_BG_pxl || smallest_neighbor_BG_pxl == 0) smallest_neighbor_BG_pxl =  glbl_var.BGlabels[indx2];
							if (glbl_var.BGlabels[indx2] < highest_neighbor_BG_pxl) highest_neighbor_BG_pxl =  glbl_var.BGlabels[indx2];
						}
					}




				}


				//calculate the probability
				if (glbl_var.numOfRepeating == 0 )
				{
					double prob = ((double)num_FG_pxls)/((double)neighborhood_size);

					//adjust the mask
					//|| 
					//|| ((unsigned char) glbl_var.BGmask->imageData[cnt_x + cnt_y*w] == 255)
					if (prob < 0.5 ) //if too little connectivity
						glbl_var.BGmask->imageData[indx] = 0;
					else if (prob > 0.5) //if high connectivity or already marked foreground
						glbl_var.BGmask->imageData[indx] = 255;
				}

				//labeling
				if (((unsigned char) glbl_var.BGmask->imageData[indx] == 0))
				{
					if (smallest_neighbor_BG_pxl == 0) // i.e. no labels around
					{
						int tmp_indx = -newIndexObject();
						glbl_var.BGlabels[indx] = tmp_indx;
						connectivityArray_setArea(-tmp_indx,1); //new label with area = 1
					}
					else //assign the smallest label and preserve connectivity relations
					{
						glbl_var.BGlabels[indx] = smallest_neighbor_BG_pxl;
						connectivityArray_setArea(-smallest_neighbor_BG_pxl,connectivityArray_getArea(-smallest_neighbor_BG_pxl) + 1); //increment teh area of the label
						//check if there are two differents neighbor labels
						if (smallest_neighbor_BG_pxl != highest_neighbor_BG_pxl )
						{

							/*int child, to_be_added_parent, original_parent,previous_child;

							child = -highest_neighbor_BG_pxl;
							to_be_added_parent = -smallest_neighbor_BG_pxl;
							original_parent = connectivityArray_getParent(child);
							while(original_parent > to_be_added_parent )
							{

							if(original_parent == -1) break;
							child = original_parent;
							original_parent = connectivityArray_getParent(child);;
							}

							connectivityArray_setParent(child,to_be_added_parent);
							if( to_be_added_parent !=  original_parent)
							{	
							connectivityArray_setParent(to_be_added_parent,original_parent);
							}*/

							label_union(-smallest_neighbor_BG_pxl,-highest_neighbor_BG_pxl);

							//connectivityArray_setParent(-highest_neighbor_BG_pxl,-smallest_neighbor_BG_pxl);
						}
					}
				} 
				else
				{
					//assign label
					if (smallest_neighbor_FG_pxl == 0) // i.e. no labels around
					{
						int tmp_indx = newIndexObject();
						glbl_var.BGlabels[indx] = tmp_indx;
						connectivityArray_setArea(tmp_indx,1); //new label with area = 1
					}
					else //assign the smallest label and preserve connectivity relations
					{
						glbl_var.BGlabels[indx] = smallest_neighbor_FG_pxl;
						connectivityArray_setArea(smallest_neighbor_FG_pxl,connectivityArray_getArea(smallest_neighbor_FG_pxl) + 1); //increment teh area of the label
						//check if there are two differents neighbor labels
						if (smallest_neighbor_FG_pxl != highest_neighbor_FG_pxl)
						{
							/*int child, to_be_added_parent, original_parent;

							child = highest_neighbor_FG_pxl;
							to_be_added_parent = smallest_neighbor_FG_pxl;
							original_parent = connectivityArray_getParent(child);
							while(original_parent > to_be_added_parent )
							{

							if(original_parent == -1) break;
							child = original_parent;
							original_parent = connectivityArray_getParent(child);
							}

							connectivityArray_setParent(child,to_be_added_parent);
							if( to_be_added_parent !=  original_parent)
							{	
							connectivityArray_setParent(to_be_added_parent,original_parent);
							}*/
							label_union(smallest_neighbor_FG_pxl,highest_neighbor_FG_pxl);

							//connectivityArray_setParent(highest_neighbor_FG_pxl,smallest_neighbor_FG_pxl);
						}
					}

				}
				//else if prob = 0.5 exactly, change nothing


				indx++;
		}
}

void BG_codebook_model::NFPP_update_pass2()
{
	//int tempsize = w*h/4;
	int tempsize = newIndexObject();
	for (int i=tempsize -1; i >= 0; i--) //reassign to ultimate parents and recalculate the area
	{
		int tmp =findUltimateParent(i);
		connectivityArray_setParent(i,tmp);
		if ( tmp != i)
		{
			connectivityArray_setArea(tmp,connectivityArray_getArea(tmp) + connectivityArray_getArea(i));
			//connectivityArray_setArea(i,0);

		}
	}

	//if there are any closed background blobs, turn them into FG
	int indx=-1;
	for (int cnt_y=0; cnt_y<h; cnt_y++)
		for (int cnt_x=0; cnt_x<w; cnt_x++)
		{
			indx++;

			//skip BG pixels
			//if (glbl_var.BGmask->imageData[indx] == 0)
			//continue;



			//if the area is too small or too big, replace with background 
			//most likely,  a single background blob will be kept which is the one larger than w*h/4
			int tempArea = connectivityArray_getArea(connectivityArray_getParent(abs(glbl_var.BGlabels[indx])));
			bool tempItsForeground = glbl_var.BGlabels[indx] >0 ;
			int min_BG_area= minArea; //w*h/4
			if ( !tempItsForeground && tempArea < min_BG_area )
			{
				glbl_var.repeatLabeling =1;

				//if (glbl_var.BGlabels[indx] >0)
				//	glbl_var.BGmask->imageData[indx] = 0; //it's background because it's less than min area
				//	if (glbl_var.BGlabels[indx] <0)
				glbl_var.BGmask->imageData[indx] = 255; //it's foreground because it's less than min area



				/*if (glbl_var.BGlabels[indx] >0)
				glbl_var.BGlabels[indx] = -(connectivityArray_getParent(abs(glbl_var.BGlabels[indx])));
				else
				glbl_var.BGlabels[indx] = (connectivityArray_getParent(abs(glbl_var.BGlabels[indx])));

				*/


			}
			/*else
			{
			if (glbl_var.BGlabels[indx] >0)
			glbl_var.BGlabels[indx] = (connectivityArray_getParent(abs(glbl_var.BGlabels[indx])));
			else
			glbl_var.BGlabels[indx] = -(connectivityArray_getParent(abs(glbl_var.BGlabels[indx])));


			}*/








		}
		if (glbl_var.repeatLabeling) return;


		//replace labels by their parents
		indx=-1;
		for (int cnt_y=0; cnt_y<h; cnt_y++)
			for (int cnt_x=0; cnt_x<w; cnt_x++)
			{
				indx++;

				//skip BG pixels
				//if (glbl_var.BGmask->imageData[indx] == 0)
				//continue;




				//if the area is too small or too big, replace with background 
				int tempArea = connectivityArray_getArea(connectivityArray_getParent(abs(glbl_var.BGlabels[indx])));
				bool tempItsForeground = glbl_var.BGlabels[indx] >0 ;
				if ( tempItsForeground && ((tempArea < minArea)  || (tempArea > maxArea )))
				{
					glbl_var.repeatLabeling =1;

					//if (glbl_var.BGlabels[indx] >0)
					glbl_var.BGmask->imageData[indx] = 0; //it's background because it's less than min area
					//	if (glbl_var.BGlabels[indx] <0)
					//glbl_var.BGmask->imageData[indx] = 255; //it's foreground because it's less than min area

					//	if (glbl_var.BGlabels[indx] >0)
					glbl_var.BGlabels[indx] = -(connectivityArray_getParent(abs(glbl_var.BGlabels[indx])));
					//	else
					//	glbl_var.BGlabels[indx] = (connectivityArray_getParent(abs(glbl_var.BGlabels[indx])));




				}
				else
				{
					if (glbl_var.BGlabels[indx] >0)
						glbl_var.BGlabels[indx] = (connectivityArray_getParent(abs(glbl_var.BGlabels[indx])));
					else
						glbl_var.BGlabels[indx] = -(connectivityArray_getParent(abs(glbl_var.BGlabels[indx])));


				}








			}

}

int BG_codebook_model::findUltimateParent(int indx)
{
	int prnt = connectivityArray_getParent(indx);
	if( prnt == -1 )
		return indx;
	else
		return findUltimateParent(prnt);

}

int BG_codebook_model::label_union(int label1,int label2)
{
	int parent1 = findUltimateParent(label1);
	int parent2 = findUltimateParent(label2);

	if(parent2 < parent1)
	{
		connectivityArray_setParent(parent1,parent2);
		return parent2;
	}
	else if(parent2 > parent1)
	{
		connectivityArray_setParent(parent2,parent1);
		return parent1;
	}
	else
		return parent1;

}

void BG_codebook_model::resetConnectivityArray(){
	int tmp =  w*h/4;
	for (int i=0; i <tmp; i++)
	{
		connectivityArray[i][0] = -1;
		connectivityArray[i][1] = 0;
	}
	indexOfNextObject = 0;
}

void BG_codebook_model::update_MNRLs()
{


	codebook* ptr = CB[0];

	int i=0;
	for (int cnt_y=0; cnt_y<h; cnt_y++)
		for (int cnt_x=0; cnt_x<w; cnt_x++)
		{
			codebook* ptr2 = ptr;
			for (int j=0; j < num_of_active_models_per_pixel[cnt_x + cnt_y*w]; j++)
				//while(ptr2)
			{	
				int LA = ptr2->getlast_access();
				if ((t - LA) > ptr2->getMNRL()) ptr2->setMNRL(t - LA);
				ptr2 = ptr2->get_next_CW();
			}
			//ptr += L_max;
			//ptr++;
			i++;
			ptr = CB[i];
		}
}

void BG_codebook_model::deleteCW(codebook*& head, int a)
{
	codebook* before = head;
	codebook* after = head->get_next_CW();

	if (a == 0)
	{
		head = after;
		delete before;
	}
	else
	{
		for (int i=1; i < a; i++)
		{
			before = before->get_next_CW();
			after = after->get_next_CW();
		}
		before->set_next_CW(after->get_next_CW());
		delete after;

	}
}

void BG_codebook_model::swapCWs(codebook*& head, int a, int b)
{


	if (a == b)
		return;

	int larger = ( a>b ? a : b);
	codebook* temp = head, *priortemp = head;
	codebook* ptrb = head, *ptra = head, *priorptra = head, *priorptrb = head;
	for (int i=0; i <= larger; i++)
	{
		if (i == b) 
		{
			priorptrb = priortemp;
			ptrb = temp;
		}
		if (i == a) 
		{
			priorptra = priortemp;
			ptra = temp;
		}

		priortemp = temp;
		temp = temp->get_next_CW();

	}

	if (a == 0)
		head = ptrb;
	else
		priorptra->set_next_CW(ptrb);

	if (b == 0)
		head = ptra;
	else
		priorptrb->set_next_CW(ptra);

	temp = ptrb->get_next_CW();
	ptrb->set_next_CW(ptra->get_next_CW());
	ptra->set_next_CW(temp);



}



//find the blobs in the new frame
void BG_codebook_model::findBGSubtractionBlobs(int* BGlabels)
{

	list<ConnectedComponent*>::iterator ptrCC;
	//int n = BGSubtractionBlobs->GetBlobNum(); //number before detecting new blobs
	for(ptrCC= BGSubtractionBlobs.begin(); ptrCC != BGSubtractionBlobs.end(); ptrCC++)
		delete (*ptrCC);
	BGSubtractionBlobs.clear();
	ConnectedComponent* newCC = new ConnectedComponent();;

	int indx = -1;
	for (int cnt_y=0; cnt_y<h; cnt_y++)
		for (int cnt_x=0; cnt_x<w; cnt_x++)
		{
			indx++;
			int lbl = BGlabels[indx];
			if (lbl < 0) continue; //BG
			bool found= false;
			for(ptrCC= BGSubtractionBlobs.begin(); ptrCC != BGSubtractionBlobs.end(); ptrCC++)
			{
				if (lbl == (*ptrCC)->blob->ID) //it exists already, check x1,y1,x2,y2 and update if the new coordinates are further from what is already stored (getting the largest covering rectangle)
				{
					found = true;

					//coordinates and dimensions
					if ((*ptrCC)->blob->x > cnt_x) 
					{
						(*ptrCC)->blob->w += (*ptrCC)->blob->x  - cnt_x ;
						(*ptrCC)->blob->x = cnt_x ;
					}
					else if((*ptrCC)->blob->w < cnt_x - (*ptrCC)->blob->x + 1)
					{
						(*ptrCC)->blob->w =   cnt_x - (*ptrCC)->blob->x+1  ;
					}

					if ((*ptrCC)->blob->y > cnt_y) 
					{
						(*ptrCC)->blob->h += (*ptrCC)->blob->y - cnt_y ;
						(*ptrCC)->blob->y = cnt_y ;
					}
					else if((*ptrCC)->blob->h < cnt_y - (*ptrCC)->blob->y + 1)
					{
						(*ptrCC)->blob->h =   cnt_y - (*ptrCC)->blob->y+1  ;
					}

					//base and top coordinates
					if((*ptrCC)->yBase < cnt_y)
					{
						(*ptrCC)->yBase = cnt_y;
						//xbase will be updated later in blob correlation (because we need xCentroid)

						(*ptrCC)->xBase = cnt_x;
					}
					if((*ptrCC)->yTop == -1) //(*ptrCC)->yTop > cnt_y || 
					{
						(*ptrCC)->yTop = cnt_y;

						(*ptrCC)->xTop = cnt_x;
					}



					break;
				}
			}

			if (!found) //if not found, add it
			{
				newCC->blob->x= cnt_x;
				newCC->blob->y = cnt_y;
				newCC->blob->w = 1;
				newCC->blob->h = 1;
				newCC->blob->ID = lbl;
				newCC->matched = false;
				BGSubtractionBlobs.push_back(newCC);
				newCC = new ConnectedComponent();
			}


		}

		//delete the ones on the boarders
		//deleteBGSubtractionBlobs();





}

void BG_codebook_model::deleteBGSubtractionBlobs()
{
	list<ConnectedComponent*>::iterator ptrCC;
	list<ConnectedComponent*>::iterator ptrCC2;
	ptrCC= BGSubtractionBlobs.begin();
	while(ptrCC != BGSubtractionBlobs.end())
	{
		if((*ptrCC)->blob->x <= glbl_var.boarder_x ||(*ptrCC)->blob->y <= glbl_var.boarder_y || (*ptrCC)->blob->y + (*ptrCC)->blob->h  >= h - glbl_var.boarder_y || (*ptrCC)->blob->x + (*ptrCC)->blob->w  >= w - glbl_var.boarder_y )
		{


			ptrCC2 = ptrCC;

			//if( glbl_var.saveFramesToHard)
			//	ObjectProcessingFile << "CC " << (*ptrCC2)->blob->ID << " at the boarder and removed !" << endl;


			ptrCC++;
			BGSubtractionBlobs.erase(ptrCC2);






		}
		else
			ptrCC++;

	}

}

void BG_codebook_model::blobPreprocessing()
{
	list<ConnectedComponent*>::iterator ptrCC;
	int sizes[3] ={16,16,16};
	float range[] = { 0, 255 };
	float* ranges[] = { range, range, range };
	IplImage* Ch[3];
	IplImage* Ch_RGB[3];
	bool useRGBforContour = false;


	//enlarge the frame by offset_contour pixels at each 
	for (int i=0; i < 3; i++)
	{
		Ch[i] = cvCreateImage(cvSize(glbl_var.currentFrame->width+2*offset_contour,glbl_var.currentFrame->height+2*offset_contour),glbl_var.currentFrame->depth,1);
		cvZero(Ch[i]);
		cvSetImageROI(Ch[i],cvRect(offset_contour,offset_contour,glbl_var.currentFrame->width,glbl_var.currentFrame->height));

		//RGB used for contours only
		Ch_RGB[i] = cvCreateImage(cvSize(glbl_var.currentFrame->width+2*offset_contour,glbl_var.currentFrame->height+2*offset_contour),glbl_var.currentFrame->depth,1);
		cvZero(Ch_RGB[i]);
		cvSetImageROI(Ch_RGB[i],cvRect(offset_contour,offset_contour,glbl_var.currentFrame->width,glbl_var.currentFrame->height));


	}
	cvSplit(glbl_var.currentFrame,Ch[0],Ch[1],Ch[2],0);
	cvSplit(glbl_var.OriginalCurrentFrameBeforeConvert,Ch_RGB[0],Ch_RGB[1],Ch_RGB[2],0);
	IplImage* maskOriginal = cvCreateImage(cvSize(original_BG->width+2*offset_contour,original_BG->height+2*offset_contour),original_BG->depth,1);
	IplImage* maskFinal = cvCreateImage(cvSize(original_BG->width+2*offset_contour,original_BG->height+2*offset_contour),original_BG->depth,1);
	cvZero(maskOriginal);
	cvZero(maskFinal);
	cvSetImageROI(maskOriginal,cvRect(offset_contour,offset_contour,glbl_var.currentFrame->width,glbl_var.currentFrame->height));
	cvSetImageROI(maskFinal,cvRect(offset_contour,offset_contour,glbl_var.currentFrame->width,glbl_var.currentFrame->height));
	cvCopyImage(original_BG,maskOriginal);
	cvCopyImage(glbl_var.BGmask,maskFinal);



	ptrCC= BGSubtractionBlobs.begin();
	while(ptrCC != BGSubtractionBlobs.end())
	{

		//hist
		(*ptrCC)->hist = cvCreateHist(3,sizes,CV_HIST_ARRAY,ranges);

		for (int i=0; i < 3; i++)
			cvSetImageROI(Ch[i],cvRect((*ptrCC)->blob->x+offset_contour,(*ptrCC)->blob->y+offset_contour,(*ptrCC)->blob->w,(*ptrCC)->blob->h)); //used for histogram

		cvSetImageROI(maskOriginal,cvRect((*ptrCC)->blob->x+offset_contour,(*ptrCC)->blob->y+offset_contour,(*ptrCC)->blob->w,(*ptrCC)->blob->h)); //used for histogram
		cvSetImageROI(maskFinal,cvRect((*ptrCC)->blob->x,(*ptrCC)->blob->y,(*ptrCC)->blob->w+2*offset_contour,(*ptrCC)->blob->h+2*offset_contour)); //used for contours
		(*ptrCC)->blob_img = cvCreateImage(cvSize((*ptrCC)->blob->w + 2*offset_contour,(*ptrCC)->blob->h + 2*offset_contour),glbl_var.currentFrame->depth,3);
		(*ptrCC)->blob_mask = cvCreateImage(cvSize((*ptrCC)->blob->w + 2*offset_contour,(*ptrCC)->blob->h + 2*offset_contour),glbl_var.currentFrame->depth,1);
		cvSetImageROI((*ptrCC)->blob_mask,cvRect(offset_contour,offset_contour,(*ptrCC)->blob->w,(*ptrCC)->blob->h));
		cvCopyImage(maskOriginal,(*ptrCC)->blob_mask);

		//take labels into account
		int x1 = (*ptrCC)->blob->x;
		int x2 = (*ptrCC)->blob->x + (*ptrCC)->blob->w ;
		int y1 = (*ptrCC)->blob->y;
		int y2 = (*ptrCC)->blob->y + (*ptrCC)->blob->h ;
		for(int i = x1; i < x2; i++)
		{
			for(int j=y1; j < y2; j++)
			{
				int indx1 = i+ glbl_var.BGmask->widthStep*j ;
				int indx2 = (i-x1 + offset_contour) + (j-y1 +offset_contour)*(*ptrCC)->blob_mask->widthStep;
				//if not of the same label, fill the pixel as BG
				if(glbl_var.BGlabels[indx1] > 0 && glbl_var.BGlabels[indx1] != (*ptrCC)->blob->ID)
				{
					(*ptrCC)->blob_mask->imageData[indx2] = 0;
				}
			}
		}
		cvSetImageROI((*ptrCC)->blob_mask,cvRect(offset_contour,offset_contour,(*ptrCC)->blob->w,(*ptrCC)->blob->h)); //used for histogram
		cvCalcHist(Ch,(*ptrCC)->hist,0, (*ptrCC)->blob_mask);

		//moment
		CvMoments moments;
		CvHuMoments humoments;
		cvMoments((*ptrCC)->blob_mask,&moments,1);
		cvGetHuMoments(&moments,&humoments);
		(*ptrCC)->moment = humoments.hu1;
		(*ptrCC)->area = (moments.m00);
		(*ptrCC)->centroid_x = (moments.m10)/(moments.m00) + (*ptrCC)->blob->x;
		(*ptrCC)->centroid_y = (moments.m01)/(moments.m00) + (*ptrCC)->blob->y;

		//xbase
		//(*ptrCC)->xBase = (*ptrCC)->centroid_x;
		//(*ptrCC)->xTop = (*ptrCC)->centroid_x;

		cvResetImageROI((*ptrCC)->blob_mask);
		cvCopyImage(maskFinal,(*ptrCC)->blob_mask);
		//take labels into account
		x1 = (*ptrCC)->blob->x - offset_contour;
		x2 = (*ptrCC)->blob->x + (*ptrCC)->blob->w +offset_contour;
		y1 = (*ptrCC)->blob->y - offset_contour;
		y2 = (*ptrCC)->blob->y + (*ptrCC)->blob->h +offset_contour;
		for(int i = x1; i < x2; i++)
		{
			for(int j=y1; j < y2; j++)
			{
				int indx1 = i+ glbl_var.BGmask->widthStep*j ;
				int indx2 = (i-x1 ) + (j-y1)*(*ptrCC)->blob_mask->widthStep;
				//if not of the same label, fill the pixel as BG
				if((indx1 <= 0 && indx1 >= w*h) || (glbl_var.BGlabels[indx1] > 0 && glbl_var.BGlabels[indx1] != (*ptrCC)->blob->ID))
				{
					(*ptrCC)->blob_mask->imageData[indx2] = 0;
				}
			}
		}

		//capture image
		for (int i=0; i < 3; i++)
		{
			cvSetImageROI(Ch[i],cvRect((*ptrCC)->blob->x,(*ptrCC)->blob->y,(*ptrCC)->blob->w+2*offset_contour,(*ptrCC)->blob->h+2*offset_contour));
			cvSetImageROI(Ch_RGB[i],cvRect((*ptrCC)->blob->x,(*ptrCC)->blob->y,(*ptrCC)->blob->w+2*offset_contour,(*ptrCC)->blob->h+2*offset_contour));
		}
		(*ptrCC)->blob_img = cvCreateImage(cvSize((*ptrCC)->blob->w+2*offset_contour,(*ptrCC)->blob->h+2*offset_contour),glbl_var.currentFrame->depth,3);
		if(!useRGBforContour)
			cvMerge(Ch[0],Ch[1],Ch[2],0,(*ptrCC)->blob_img);
		else
			cvMerge(Ch_RGB[0],Ch_RGB[1],Ch_RGB[2],0,(*ptrCC)->blob_img);

		//Check contours and see if they are ghosts or real objects
		//if they are ghosts, delete the CC
		int contour_width = 2;
		bool normalize = true;
		bool corr = false; 
		bool useWholeObject = false;
		float CONTOUR_HIST_THRESHOLD = 0.53;//0.53 Lab (2006) , 0.60 station, 0.50 loitering
		IplImage* outer = 0;
		IplImage* inner = 0;
		CvHistogram* innerHist;
		CvHistogram* outerHist;

		//morphology
		IplImage* edges = cvCreateImage(cvGetSize((*ptrCC)->blob_mask),8,1);
		cvMorphologyEx((*ptrCC)->blob_mask,edges,NULL,NULL,CV_MOP_GRADIENT ,contour_width);



		//find contours
		CvMemStorage* storage = cvCreateMemStorage();
		CvSeq* first_contour = NULL;
		int nContours = cvFindContours(edges,storage,&first_contour,sizeof(CvContour),CV_RETR_LIST );
		IplImage* accumulatedInner = cvCreateImage(cvGetSize(edges),8,1);
		IplImage* accumulatedOuter = cvCreateImage(cvGetSize(edges),8,1);
		(*ptrCC)->blob_inner_contour = cvCreateImage(cvGetSize(edges),8,3);
		(*ptrCC)->blob_outer_contour = cvCreateImage(cvGetSize(edges),8,3);
		cvZero(accumulatedInner);
		cvZero(accumulatedOuter);
		cvZero((*ptrCC)->blob_inner_contour);
		cvZero((*ptrCC)->blob_outer_contour);
		while (first_contour)
		{
			//inner
			inner = cvCreateImage(cvGetSize((*ptrCC)->blob_mask),(*ptrCC)->blob_mask->depth,1);
			cvZero(inner);
			cvDrawContours(inner,first_contour,cvScalar(0,0,0),cvScalar(255,255,255),0,contour_width);
			cvOr(inner,accumulatedInner,accumulatedInner,inner);
			outer = cvCreateImage(cvGetSize((*ptrCC)->blob_mask),(*ptrCC)->blob_mask->depth,1);
			cvZero(outer);
			cvDrawContours(outer,first_contour,cvScalar(255,255,255),cvScalar(0,0,0),0,contour_width);
			cvOr(outer,accumulatedOuter,accumulatedOuter,outer);

			first_contour=first_contour->h_next;
		}
		cvCopy((*ptrCC)->blob_img,(*ptrCC)->blob_outer_contour,accumulatedOuter);
		cvCopy((*ptrCC)->blob_img,(*ptrCC)->blob_inner_contour,accumulatedInner);
		innerHist = cvCreateHist(3,sizes,CV_HIST_ARRAY,ranges);
		outerHist = cvCreateHist(3,sizes,CV_HIST_ARRAY,ranges);
		IplImage* Ch2[3];
		for (int i=0; i < 3; i++)
			Ch2[i] = cvCreateImage(cvGetSize((*ptrCC)->blob_mask),(*ptrCC)->blob_mask->depth,1);
		cvSplit((*ptrCC)->blob_img,Ch2[0],Ch2[1],Ch2[2],0);
		if(!useWholeObject)
			cvCalcHist(Ch2,innerHist,0,accumulatedInner);
		else
			cvCopyHist((*ptrCC)->hist,&innerHist);
		cvCalcHist(Ch2,outerHist,0,accumulatedOuter);
		if (normalize)
		{
			cvNormalizeHist(innerHist,1);
			cvNormalizeHist(outerHist,1);
		}
		(*ptrCC)->contour_distance = 1 - cvCompareHist(innerHist,outerHist,CV_COMP_INTERSECT);

		if( glbl_var.saveFramesToHard && glbl_var.saveObjectProcessingFile )
			ObjectProcessingFile << "CC " << (*ptrCC)->blob->ID << " has contour contrast of " << (*ptrCC)->contour_distance << endl;

		if(useRGBforContour)
			cvMerge(Ch[0],Ch[1],Ch[2],0,(*ptrCC)->blob_img);

		//save inner and outer contours to hard drive
		if(glbl_var.saveFramesToHard && glbl_var.saveContours )
		{

			char name[100];
			char ObjectProcessingFileName[100]; //used to keep the original name as it will be sued for objects 
			char ObjectProcessingFileName2[100]; //used to keep the original name as it will be sued for objects 
			char ObjectProcessingFileName3[100]; //used to keep the original name as it will be sued for objects 
			char buffer[100];
			strcpy(ObjectProcessingFileName,glbl_var.saveFramesTo);
			strcat(ObjectProcessingFileName,"CC");
			strcpy(ObjectProcessingFileName2,"frame");
			strcat(ObjectProcessingFileName2,itoa(glbl_var.BGmodel->get_t(),buffer,10));
			//list<ConnectedComponent*>::iterator ptrCC;
			int i=0;
			IplImage* concatenateObject; 



			concatenateObject = cvCreateImage(cvSize((*ptrCC)->blob_img->width*3,(*ptrCC)->blob_img->height),glbl_var.currentFrame->depth,glbl_var.currentFrame->nChannels);
			//cvZero(concatenateObject);
			strcpy(name,ObjectProcessingFileName);
			//strcat(ObjectProcessingFileNames[2],itoa(i,buffer,10));
			strcat(name,itoa((*ptrCC)->blob->ID,buffer,10));
			strcat(name,ObjectProcessingFileName2);
			strcat(name,".png");
			cvSetImageROI(concatenateObject,cvRect(0,0,(*ptrCC)->blob_img->width,(*ptrCC)->blob_img->height));
			cvCopyImage((*ptrCC)->blob_inner_contour,concatenateObject);
			cvSetImageROI(concatenateObject,cvRect((*ptrCC)->blob_img->width,0,(*ptrCC)->blob_img->width,(*ptrCC)->blob_img->height));
			cvCopyImage((*ptrCC)->blob_outer_contour,concatenateObject);
			cvSetImageROI(concatenateObject,cvRect((*ptrCC)->blob_img->width*2,0,(*ptrCC)->blob_img->width,(*ptrCC)->blob_img->height));
			cvCopyImage((*ptrCC)->blob_img,concatenateObject);







			cvResetImageROI(concatenateObject);
			cvSaveImage(name,concatenateObject);
			cvReleaseImage(&concatenateObject);



		}

		//delete the CC if its contours histograms are similar
		if ((*ptrCC)->contour_distance < CONTOUR_HIST_THRESHOLD && glbl_var.ghostEnabled) 
		{
			if( glbl_var.saveFramesToHard && glbl_var.saveObjectProcessingFile )
				ObjectProcessingFile << "CC " << (*ptrCC)->blob->ID << " is removed because it's a ghost!" << endl;

			(*ptrCC)->ghost = true;



			ptrCC++;
		}
		else
			ptrCC++;

		//release resources
		cvReleaseImage(&inner);
		cvReleaseImage(&outer);
		cvReleaseImage(&edges);
		//cvReleaseImage(&maskedInner);
		//cvReleaseImage(&maskedOuter);
		cvReleaseImage(&accumulatedInner);
		cvReleaseImage(&accumulatedOuter);
		for (int i=0; i <3;i++)
			cvReleaseImage(&Ch2[i]);
		cvReleaseHist(&innerHist);
		cvReleaseHist(&outerHist);
		cvReleaseMemStorage(&storage);


	}
	cvReleaseImage(&maskOriginal);
	cvReleaseImage(&maskFinal);
	for (int i=0; i < 3; i++)
	{
		cvReleaseImage(&Ch[i]);
		cvReleaseImage(&Ch_RGB[i]);
	}


}

double BG_codebook_model::absenceFrames(TrackerObject* obj)
{
	if (obj)
	{
		if(obj->absence != -1)
			return ((t - 1) - obj->absence);
		else
			return 0;
	}

	return -10; //default value
}



void BG_codebook_model::deleteBoarderObjects()
{
	list<TrackerObject*>::iterator ptrObj;
	list<TrackerObject*>::iterator ptrObj2;
	list<TrackerObject*>::iterator ptrObj3;
	list<TrackerObject*>::iterator ptrObj4;
	int x1,x2,y1,y2; 

	//delete (just got invisible) objects at the boarders (because we keep updating kalman even while they are absent, and so it might deviate from true path after a while)
	ptrObj= trackerBlobs.begin();
	while ( ptrObj != trackerBlobs.end())
	{
		if ( (*ptrObj)->occlusion_child || (*ptrObj)->isVisible  ) //|| (absenceFrames(*ptrObj) != 1) 
		{
			ptrObj++;
			continue;
		}

		//using prediction (could use blob as well)
		////(i) prediction
		//x1 = (*ptrObj)->predictedPosition().x - (*ptrObj)->predictedPosition().w/2;
		//y1 = (*ptrObj)->predictedPosition().y - (*ptrObj)->predictedPosition().h/2;
		//x2 = x1 + (*ptrObj)->predictedPosition().w;
		//y2 = y1 + (*ptrObj)->predictedPosition().h;
		//(ii) blob
		/*x1 = (*ptrObj)->blob->x - (*ptrObj)->blob->w/2;
		y1 = (*ptrObj)->blob->y - (*ptrObj)->blob->h/2;
		x2 = x1 + (*ptrObj)->blob->w;
		y2 = y1 + (*ptrObj)->blob->h;*/
		//(iii) blob completely outside
		x2 = (*ptrObj)->blob->x - (*ptrObj)->blob->w/2;
		y2 = (*ptrObj)->blob->y - (*ptrObj)->blob->h/2;
		x1 = x1 + (*ptrObj)->blob->w;
		y1 = y1 + (*ptrObj)->blob->h;
		if ( x1 <= glbl_var.boarder_x ||  y1 <= glbl_var.boarder_y || x2 >= w-1-glbl_var.boarder_x || y2 >= h-1-glbl_var.boarder_y)
		{
			ptrObj2 = ptrObj;


			ptrObj++;

			if( glbl_var.saveFramesToHard &&glbl_var.saveObjectProcessingFile )
				ObjectProcessingFile << "object " << (*ptrObj2)->blob->ID << " at the boarder and removed with its lone children !" << endl; //

			//deleting lone children
			ptrObj3 = (*ptrObj2)->occlusion_list.begin();
			while( ptrObj3 != (*ptrObj2)->occlusion_list.end())
			{	
				if ((*ptrObj3)->parent.size() == 1) 
				{
					ptrObj4 = ptrObj3;
					ptrObj3++;
					erase_object((*ptrObj4));
				}
				else
					ptrObj3++;
			}

			//also delete its CC
			/*list<ConnectedComponent*>::iterator CCitr;
			for(CCitr == BGSubtractionBlobs.begin() ; CCitr != BGSubtractionBlobs.end();CCitr++)
			{
			if((*ptrObj2)->CC == (*CCitr))
			{
			BGSubtractionBlobs.erase(CCitr);
			break;
			}
			}*/

			erase_object((*ptrObj2));



		}
		else
			ptrObj++;
	}


}

void BG_codebook_model::resetObjectHistory()
{

	list<TrackerObject*>::iterator ptrObj;

	//unmatch all Objects first
	for (ptrObj= trackerBlobs.begin(); ptrObj != trackerBlobs.end(); ptrObj++)
	{
		(*ptrObj)->matched = false;
		(*ptrObj)->CC = NULL;
		(*ptrObj)->splitIndex = -1;
		(*ptrObj)->updateCandidate = NULL;
		(*ptrObj)->updateCandidate_object_classification = UNKNOWN;
		(*ptrObj)->updateCandidate_PersonGroup = 0;
		//(*ptrObj)->candidateResolveHistoryParentList = NULL;
		(*ptrObj)->candidateResolveHistoryParentList.clear();
		//(*ptrObj)->distanceFromCC = double::MaxValue;
		(*ptrObj)->rearrangeParents = NULL;

	}
}



void BG_codebook_model::ObjectSplitting()
{

	list<TrackerObject*>::iterator ptrObj;
	list<TrackerObject*>::iterator ptrObj2;
	list<TrackerObject*>::iterator ptrObj3;
	list<ConnectedComponent*>::iterator ptrCC;
	list<ConnectedComponent*>::iterator ptrCC2;
	list<TrackerObject*> matchedObjects;

	//handle splits
	int splitCounter=0;
	for (ptrObj= trackerBlobs.begin(); ptrObj != trackerBlobs.end(); ptrObj++)
	{

		//only visible objects can participate in occlusion
		if (!(*ptrObj)->isVisible) continue;

		//check spatially which CCs are associated with this obj
		int matches = 0;
		list<ConnectedComponent*> matchedCCs;
		for (ptrCC= BGSubtractionBlobs.begin(); ptrCC != BGSubtractionBlobs.end(); ptrCC++)
		{
			if ((*ptrCC)->ghost) continue;

			//bool as_predicted = intersect(&(*ptrObj)->predictedPosition(),(*ptrCC)->blob);

			bool as_predicted = pixelToPixelIntersect((*ptrObj),(*ptrCC),true);


			//////check if blob centroid is within the object
			//bool x_condition1 = ((*ptrCC)->centroid_x < (*ptrObj)->blob->w/2 + (*ptrObj)->blob->x) && ((*ptrCC)->centroid_x > -(*ptrObj)->blob->w/2 + (*ptrObj)->blob->x);
			//bool y_condition1 = ((*ptrCC)->centroid_y < (*ptrObj)->blob->h/2 + (*ptrObj)->blob->y) && ((*ptrCC)->centroid_y > -(*ptrObj)->blob->h/2 + (*ptrObj)->blob->y);

			////check if blob centroid is within the object prediction
			//bool x_condition2 = ((*ptrCC)->centroid_x < (*ptrObj)->blob->w/2 + (*ptrObj)->predictedPosition().x) && ((*ptrCC)->centroid_x > -(*ptrObj)->blob->w/2 + (*ptrObj)->predictedPosition().x);
			//bool y_condition2 = ((*ptrCC)->centroid_y < (*ptrObj)->blob->h/2 + (*ptrObj)->predictedPosition().y) && ((*ptrCC)->centroid_y > -(*ptrObj)->blob->h/2 + (*ptrObj)->predictedPosition().y);


			if (as_predicted )
				////if(x_condition2 && y_condition2)
				//if((x_condition2 && y_condition2)|| (x_condition1 && y_condition1))
			{
				matches++;
				matchedCCs.push_back((*ptrCC));

				if( glbl_var.saveFramesToHard && glbl_var.saveObjectProcessingFile )
					ObjectProcessingFile << "CC " << (*ptrCC)->blob->ID << " and Obj " << (*ptrObj)->blob->ID << " associated spatially" << endl;
			}
		}

		//if the object from the previous frame is matched with more than a CC from the current frame
		if (matches > 1)
		{

			////check overlap percentages
			//bool redo;
			//do{
			//	redo = false;
			//	ptrCC= matchedCCs.begin();
			//	/*float requiredRatio = (matches+2);
			//	requiredRatio = 1/requiredRatio;*/
			//	float requiredRatio = 0.1;
			//	while ( ptrCC != matchedCCs.end())
			//	{
			//		float ovrlap = pixelIntersectratio( (*ptrObj),(*ptrCC), true);
			//		if (ovrlap < requiredRatio)
			//		{
			//			if( glbl_var.saveFramesToHard)
			//				ObjectProcessingFile << "CC " << (*ptrCC)->blob->ID << " and Obj " << (*ptrObj)->blob->ID << " not overlapping by the required ratio " << ovrlap << " < " << requiredRatio << ".CC " << (*ptrCC)->blob->ID << " removed from matched CCs" << endl;


			//			matches--;
			//			matchedCCs.erase(ptrCC);
			//			redo = true;


			//		}

			//		if (redo) break;
			//		ptrCC++;
			//	}
			//}while(redo);

			//if(matches <=1) 
			//	goto out_of_if;
			//else if( glbl_var.saveFramesToHard)
			//		ObjectProcessingFile << "Matching cancelled" << endl;




			ptrCC= matchedCCs.begin();
			while ( ptrCC != matchedCCs.end())
			{
				double bestColorDistance = double::MaxValue;
				TrackerObject* best_split_object= NULL;

				for (ptrObj2= (*ptrObj)->occlusion_list.begin(); ptrObj2 != (*ptrObj)->occlusion_list.end(); ptrObj2++)
				{
					if ((*ptrObj2)->matched) continue;
					if ((*ptrObj2)->toBeDeleted) continue;
					if ((*ptrObj2)->updateCandidate) continue;


					float ratioW, ratioH=0;
					//if(!glbl_var.comparableInSize((*ptrObj2)->blob, (*ptrCC)->blob,ratioW, ratioH )) //2006
					if(!glbl_var.comparableInSize((*ptrCC)->area,(*ptrObj2)->area, ratioW )) //station
					{
						if( glbl_var.saveFramesToHard && glbl_var.saveObjectProcessingFile )
							ObjectProcessingFile << "Object " << (*ptrObj2)->blob->ID << " and CC " << (*ptrCC)->blob->ID << " are not comparable in size. ratio = " << ratioW << " , " << ratioH <<endl;

						continue; //they should be comparable in size
					}
					else
						ObjectProcessingFile << ". ratio = " << ratioW << " , " << ratioH << " ";

					double distance = colorDistance((*ptrObj2), (*ptrCC), (glbl_var.useHistDifference ? 2:1 ));

					if( glbl_var.saveFramesToHard && glbl_var.saveObjectProcessingFile )
						ObjectProcessingFile << "color distance between " << (*ptrObj2)->blob->ID << " and " << (*ptrCC)->blob->ID << " is " << distance << endl;

					if (distance <= CC_Obj_max_acceptable_distance_onetoone && distance < bestColorDistance)
					{
						bestColorDistance = distance;
						best_split_object = (*ptrObj2);
					}
				}

				//if a best match is found associate it
				if (best_split_object)
				{
					//first set the split index for the object and its children
					(*ptrObj)->splitIndex = splitCounter;
					for (ptrObj2= (*ptrObj)->occlusion_list.begin(); ptrObj2 != (*ptrObj)->occlusion_list.end(); ptrObj2++)
						(*ptrObj2)->splitIndex = splitCounter;
					splitCounter++;


					best_split_object->updateCandidate = (*ptrCC); //to be updated if no merge takes place
					best_split_object->updateCandidate_colorDistance = bestColorDistance;

					//update last correct parent candidate
					best_split_object->updateAllSiblingscandidateResolveHistoryParentList(*ptrObj);

					(*ptrCC)->objects.push_back(best_split_object);
					(*ptrCC)->matched++;

					//object classification of the object after the split based on the classification of the object before the split Bird et al.
					//object_classification_split((*ptrObj),best_split_object,nextPersonGroup);

					//if object is an occlusion child and the parent has all its children matched, then delete that parent.
					if (best_split_object->occlusion_child)
					{

						//ptrObj2= best_split_object->parent.begin();
						//TrackerObject* obj;
						//while ( ptrObj2 != best_split_object->parent.end())
						//{

						//	bool allChildrenMatched = true;
						//	for(ptrObj3 = (*ptrObj2)->occlusion_list.begin() ; ptrObj3 != (*ptrObj2)->occlusion_list.end() ; ptrObj3++)
						//	{
						//		if((*ptrObj3)->updateCandidate || (*ptrObj3)->matched) //if a child is not matched then can't delete this parent yet
						//		{
						//			allChildrenMatched = false;
						//			break;
						//		}
						//	}
						//	if(allChildrenMatched)
						//		(*ptrObj2)->toBeDeleted = 1;


						///*obj = (*ptrObj2);
						//ptrObj2++;
						//erase_object(*ptrObj2);*/

						//	//obj->toBeDeleted = 1;
						(*ptrObj)->toBeDeleted = 1;

						/*	}*/
					}

					if( glbl_var.saveFramesToHard && glbl_var.saveObjectProcessingFile )
						ObjectProcessingFile << "(to be updated after merge) best match for CC " << (*ptrCC)->blob->ID << " is Obj " << best_split_object->blob->ID << endl;

					//remember to rearrange parents later
					best_split_object->rearrangeParents = (*ptrObj);

					ptrCC++;

				}
				else
				{

					ptrCC2 = ptrCC;
					ptrCC++;
					matchedCCs.erase(ptrCC2);
				}
			}

			//create histories for nonmatched split children
			if(!matchedCCs.empty()) 
				(*ptrObj)->prepareHistorySplitChildren(&matchedCCs); 

			//we add the original object to the CCs' matched objects because we might need it in the merge
		}

out_of_if:
		nextPersonGroup++;

		////create histories for nonmatched split children
		//(*ptrObj)->prepareHistorySplitChildren(&matchedCCs); 

		//reset all candidateResolveHistoryParentList information
		/*for (ptrObj2= trackerBlobs.begin(); ptrObj2 != trackerBlobs.end(); ptrObj2++)
		{
		if(glbl_var.saveFramesToHard)
		glbl_var.BGmodel->objectFile << "candidate resolve history has been reset for object" << (*ptrObj2)->blob->ID  <<endl;

		(*ptrObj2)->candidateResolveHistoryParentList = NULL;
		}*/


	}


	rearrangeParentsAfterSplit();

	deleteAllsiblingsMatched();





}

void BG_codebook_model::deleteAllsiblingsMatched()
{
	list<TrackerObject*>::iterator ptrObj;
	list<TrackerObject*>::iterator ptrObj2;

	//delete those to be deleted 
	ptrObj2= trackerBlobs.begin();
	while ( ptrObj2 != trackerBlobs.end())
	{


		if ((*ptrObj2)->toBeDeleted)
		{
			ptrObj = ptrObj2;
			ptrObj2++;

			if( glbl_var.saveFramesToHard && glbl_var.saveObjectProcessingFile )
				ObjectProcessingFile << "object " << (*ptrObj)->blob->ID << "deleted because all siblings matched !" << endl;


			erase_object(*ptrObj);



		}
		else
			ptrObj2++;

	}
}

void BG_codebook_model::rearrangeParentsAfterSplit()
{
	list<TrackerObject*>::iterator ptrObj;
	list<TrackerObject*>::iterator ptrObj2;
	list<TrackerObject*>::iterator ptrObj3;
	list<TrackerObject*>::iterator matchedObj_itr;

	//Rearranging split parents
	for (ptrObj= trackerBlobs.begin(); ptrObj != trackerBlobs.end(); ptrObj++)
	{

		if ((*ptrObj)->rearrangeParents == NULL) continue;



		//(i)
		//replicate children of all the parents
		/*	for (matchedObj_itr= (*ptrObj)->parent.begin(); matchedObj_itr != (*ptrObj)->parent.end(); matchedObj_itr++)
		{
		for (ptrObj2= (*matchedObj_itr) ->occlusion_list.begin(); ptrObj2 != (*matchedObj_itr)->occlusion_list.end(); ptrObj2++)
		{
		if ((*ptrObj2) !=  (*ptrObj) )
		(*ptrObj)->occlusion_list.push_back(*ptrObj2);
		}
		if (!(*ptrObj)->occlusion_list.empty()) 
		{
		(*ptrObj)->occlusion_list.sort();
		(*ptrObj)->occlusion_list.unique();
		}
		}*/
		//(ii) replicate only children of this parent
		for (ptrObj2= (*ptrObj)->rearrangeParents->occlusion_list.begin(); ptrObj2 != (*ptrObj)->rearrangeParents->occlusion_list.end(); ptrObj2++)
		{	
			if ((*ptrObj2) !=  (*ptrObj) )
				(*ptrObj)->occlusion_list.push_back(*ptrObj2);
		}
		if (!(*ptrObj)->occlusion_list.empty()) 
		{
			(*ptrObj)->occlusion_list.sort();
			(*ptrObj)->occlusion_list.unique();
		}

		//remove from occlusion list of the parents
		for (matchedObj_itr= (*ptrObj)->parent.begin(); matchedObj_itr != (*ptrObj)->parent.end(); matchedObj_itr++)
		{
			(*matchedObj_itr)->occlusion_list.remove((*ptrObj));
			/*for (ptrObj2= (*matchedObj_itr) ->occlusion_list.begin(); ptrObj2 != (*matchedObj_itr) ->occlusion_list.end(); ptrObj2++)
			{
			if ((*ptrObj2) == (*ptrObj)) 
			{
			(*matchedObj_itr)->occlusion_list.erase(ptrObj2);
			break;
			}
			}*/
		}


		//has no parents anymore
		(*ptrObj)->parent.clear();

		//not a child anymore
		(*ptrObj)->occlusion_child = false;

		// set it as parent for all its new children
		for (ptrObj2= (*ptrObj)->occlusion_list.begin(); ptrObj2 != (*ptrObj)->occlusion_list.end(); ptrObj2++)
		{
			(*ptrObj2)->parent.push_back(*ptrObj);
			(*ptrObj2)->parent.sort();
			(*ptrObj2)->parent.unique();
		}

		//(*ptrObj)->rearrangeParents = NULL;
	}

	////if any object is left with a single child, replace it with that child
	//ptrObj= trackerBlobs.begin();
	//while ( ptrObj != trackerBlobs.end())
	//{
	//	if((*ptrObj)->occlusion_list.size() == 1)
	//	{
	//		TrackerObject* prnt = (*ptrObj);
	//		TrackerObject* chld = (*ptrObj)->occlusion_list.begin();
	//		for (ptrObj2= chld->parent.begin(); ptrObj2 != chld->parent.end(); ptrObj2++)
	//		{
	//			(*ptrObj2)->occlusion_list.remove(chld);
	//		}

	//		//has no parents anymore
	//		chld->parent.clear();

	//		//not a child anymore
	//		chld->occlusion_child = false;


	//		ptrObj3 = ptrObj;
	//		ptrObj++;
	//		trackerBlobs.erase(ptrObj3);


	//	}
	//	else
	//		ptrObj++;
	//}
}

void BG_codebook_model::ObjectMerging()
{
	list<TrackerObject*>::iterator ptrObj;
	list<TrackerObject*>::iterator ptrObj2;
	list<TrackerObject*>::iterator ptrObj3;
	list<ConnectedComponent*>::iterator ptrCC;

	//handle blobs of occlusions
	for (ptrCC= BGSubtractionBlobs.begin(); ptrCC != BGSubtractionBlobs.end(); ptrCC++)
	{
		if ((*ptrCC)->ghost) continue;
		//only if not matched yet (but because split and merge might be simultanious, then we remove this condition)
		//if ((*ptrCC)->matched) continue;

		//check spatially which objects are associated with this CC
		for (ptrObj= trackerBlobs.begin(); ptrObj != trackerBlobs.end(); ptrObj++)
		{
			//only visible and persistent objects can participate in occlusion
			if (!(*ptrObj)->isVisible) continue;
			//if(t-1-(*ptrObj)->persistence < PERSISTENCE_THRESHOLD) continue;
			//if(t-1-(*ptrObj)->time < PERSISTENCE_THRESHOLD) continue;
			//if (!(*ptrObj)->persistent) continue;
			if(!(*ptrObj)->itOrChildrenPersistent()) continue;

			//if object is already associated in the split section with the same blob, skip it
			bool toContinue = false;
			for (ptrObj2= (*ptrCC)->objects.begin(); ptrObj2 != (*ptrCC)->objects.end(); ptrObj2++)
			{
				if ((*ptrObj2)->splitIndex == (*ptrObj)->splitIndex && (*ptrObj)->splitIndex != -1) 
				{
					toContinue = true;
					break;
				}
			}
			if (toContinue) continue;


			//if (!(*ptrObj)->occlusion) continue;


			//bool as_predicted = intersect((*ptrObj)->blob,(*ptrCC)->blob);
			//bool as_predicted = intersect(&(*ptrObj)->predictedPosition(),(*ptrCC)->blob);



			//associate and update by Kalman 
			//if (as_predicted )
			//{



			if(pixelIntersect((*ptrObj),(*ptrCC),true))
			{


				(*ptrObj)->matched = true;
				(*ptrObj)->CC = (*ptrCC);
				(*ptrCC)->objects.push_back((*ptrObj));
				(*ptrCC)->matched++;

				if( glbl_var.saveFramesToHard && glbl_var.saveObjectProcessingFile )
					ObjectProcessingFile << "Object " << (*ptrObj)->blob->ID << " and CC " << (*ptrCC)->blob->ID << " associated spatially" << endl;
			}

			//}

		}

		//if a CC has more than one object, create a new object that embodies them 
		if ((*ptrCC)->matched >1)
		{
			////remove objects that are not persistent
			//ptrObj2 = (*ptrCC)->objects.begin() ;
			//while(ptrObj2 != (*ptrCC)->objects.end() )
			//{
			//	if(!(*ptrObj2)->itOrChildrenPersistent())
			//	{
			//		ptrObj3 = ptrObj2;
			//		ptrObj2++;
			//		(*ptrObj3)->matched = false;
			//		(*ptrObj3)->CC = NULL;
			//		(*ptrCC)->objects.erase(ptrObj3);
			//		(*ptrCC)->matched--;
			//	}
			//	else
			//		ptrObj2++;
			//}
			////if only one object is left, the update it as a normal onetoone, esle: continue as normal merge
			//if((*ptrCC)->objects.empty())
			//{
			//	continue;
			//}
			//else if((*ptrCC)->objects.size() == 1)
			//{
			//	TrackerObject* onlyObject = (*(*ptrCC)->objects.begin());
			//	update_Obj_CC(onlyObject,*ptrCC,colorDistance(onlyObject,*ptrCC,true),1,0);
			//	//calculate histories and possibilities for the object and its children
			//	onlyObject->updateObjAndChildrenHistories(*ptrCC);

			//	if (onlyObject->moment < TYPE_THRESHOLD)
			//		onlyObject->type = 0; //bag
			//	else
			//		onlyObject->type = 1; //person
			//}
			//else
			//{

			TrackerObject* tempObj = createObject((*ptrCC),nextID);
			//calculate histories and possibilities for the object and its children
			tempObj->updateObjAndChildrenHistories((*ptrCC));

			//give the object the time of the most recent child object
			tempObj->time = -1; //initial value to find the newest time later
			for (ptrObj2= (*ptrCC)->objects.begin(); ptrObj2 != (*ptrCC)->objects.end(); ptrObj2++)
			{
				if ((*ptrObj2)->time > tempObj->time)
					tempObj->time = (*ptrObj2)->time;

				//if these objects participated in split, then dont update the split part later
				if ((*ptrObj2)->updateCandidate) 
				{
					//adjust the related parent to be the update candidate because it's is necessary to get the right history line
					//(*ptrObj2)->adjustItAndChildrenRelatedParentToPrecedingParent();

					(*ptrObj2)->updateCandidate = NULL;
					//(*ptrObj2)->candidateResolveHistoryParentList = NULL;

					if( glbl_var.saveFramesToHard && glbl_var.saveObjectProcessingFile )
						ObjectProcessingFile << "Cancelled from update because of merge participation CC " << (*ptrCC)->blob->ID << " is Obj " << (*ptrObj2)->blob->ID << endl;
				}
			}



			trackerBlobs.push_back(tempObj);

			//assign an occlusion group number to them:
			/*int occlusion_group_number = -1;
			for (ptrObj= (*ptrCC)->objects.begin(); ptrObj != (*ptrCC)->objects.end(); ptrObj++)
			if ((*ptrObj)->occlusion) occlusion_group_number =(*ptrObj)->occlusion;
			if(occlusion_group_number == -1) occlusion_group_number = nextCollisionGroup++;*/

			if( glbl_var.saveFramesToHard && glbl_var.saveObjectProcessingFile )
				ObjectProcessingFile << "new occlusion Object created for CC " << (*ptrCC)->blob->ID << " with object ID " << tempObj->blob->ID << endl; //" occlusion group " << occlusion_group_number <<" including objects "  <<

			tempObj->object_classification = object_classification_merge((*ptrCC)->objects); //refer to Bird et al.

			for (ptrObj= (*ptrCC)->objects.begin(); ptrObj != (*ptrCC)->objects.end(); ptrObj++)
			{
				if (tempObj == (*ptrObj)) continue;//skip the object itself
				(*ptrObj)->matched = false;

				bool removeObject = false;
				if(!(*ptrObj)->occlusion_list.empty() && !(*ptrObj)->rearrangeParents)
					removeObject= true; //if it has children then no need for it anymore

				//insert_object_occlusionlist(tempObj,tempObj,(*ptrObj),occlusion_group_number);
				insert_object_occlusionlist(tempObj,tempObj,(*ptrObj)); 

				if (removeObject )
					erase_object(*ptrObj);

			}




			//handle histories for children
			if(glbl_var.saveFramesToHard && glbl_var.saveObjectFile )
				glbl_var.BGmodel->objectFile << "-Merging: preparing children of" << tempObj->blob->ID  <<endl;
			for (ptrObj2= tempObj->occlusion_list.begin(); ptrObj2 != tempObj->occlusion_list.end(); ptrObj2++)
			{
				(*ptrObj2)->handleHistoryMergeChild(&((*ptrCC)->objects),tempObj, false);
			}





			if( glbl_var.saveFramesToHard && glbl_var.saveObjectProcessingFile )
			{
				for (ptrObj= tempObj->occlusion_list.begin(); ptrObj != tempObj->occlusion_list.end(); ptrObj++)
				{
					ObjectProcessingFile << (*ptrObj)->blob->ID << " , " ;
				}
				ObjectProcessingFile << endl ;
			}

			if( glbl_var.saveFramesToHard && glbl_var.saveObjectProcessingFile )
			{
				for (ptrObj= (*ptrCC)->objects.begin(); ptrObj != (*ptrCC)->objects.end(); ptrObj++)
					ObjectProcessingFile << "object " << (*ptrObj)->blob->ID << " classified as  " << ( (*ptrObj)->object_classification == UNKNOWN ? "U" :((*ptrObj)->object_classification == PERSON ? "P" : ((*ptrObj)->object_classification == STILL_PERSON ? "SP" : "O" )))<<endl;
			}


			//remove other objects from the CC object list
			(*ptrCC)->objects.clear();
			(*ptrCC)->objects.push_back(  tempObj);

			//}







		}
		else //reset matching information to 0 for both CC and obj
		{
			for (ptrObj= (*ptrCC)->objects.begin(); ptrObj != (*ptrCC)->objects.end(); ptrObj++)
			{
				(*ptrObj)->matched = false;
				(*ptrObj)->CC = NULL;
			}
			(*ptrCC)->objects.clear();
			(*ptrCC)->matched = 0;
		}

	}


}

void BG_codebook_model::deleteNonvisibleNonpersistentObjects()
{
	list<TrackerObject*>::iterator ptrObj;
	list<TrackerObject*>::iterator ptrObj2;

	ptrObj= trackerBlobs.begin();
	while ( ptrObj != trackerBlobs.end())
	{
		if((*ptrObj)->isVisible || (*ptrObj)->itOrChildrenPersistent()) 
		{
			ptrObj++;
		}
		else
		{
			ptrObj2 = ptrObj;
			ptrObj++;
			erase_object(*ptrObj2);
		}

	}
}
void BG_codebook_model::fillCandidateOwners(TrackerObject* obj)
{

}




void BG_codebook_model::ApplyPendingSplits()
{


	list<TrackerObject*>::iterator ptrObj;
	list<TrackerObject*>::iterator ptrObj2;

	//handle objects that are waiting for merge results to update their model
	for (ptrObj= trackerBlobs.begin(); ptrObj != trackerBlobs.end(); ptrObj++)
	{
		if ( !(*ptrObj)->updateCandidate)
		{
			//(*ptrObj)->candidateResolveHistoryParentList = NULL; //no need for this list anymore since it is not a result of a split
			continue;
		}


		if( glbl_var.saveFramesToHard && glbl_var.saveObjectProcessingFile )
			ObjectProcessingFile << "resume pending update Obj " << (*ptrObj)->blob->ID << endl;

		if(!(*ptrObj)->occlusion_list.empty()) //it means we will create a new object that covers the object and its children
		{
			ConnectedComponent* tmpCC = (*ptrObj)->updateCandidate;
			TrackerObject* newObject = createObject(tmpCC, nextID);

			//calculate histories and possibilities for the object and its children
			newObject->updateObjAndChildrenHistories(tmpCC);

			trackerBlobs.push_back(newObject);

			//assign an occlusion group number to them:
			/*int occlusion_group_number = -1;
			for (ptrObj= (*ptrCC)->objects.begin(); ptrObj != (*ptrCC)->objects.end(); ptrObj++)
			if ((*ptrObj)->occlusion) occlusion_group_number =(*ptrObj)->occlusion;
			if(occlusion_group_number == -1) occlusion_group_number = nextCollisionGroup++;*/

			if( glbl_var.saveFramesToHard && glbl_var.saveObjectProcessingFile )
				ObjectProcessingFile << "new occlusion Object created for CC " << tmpCC->blob->ID << " with object ID " << newObject->blob->ID << endl; //" occlusion group " << occlusion_group_number <<" including objects "  <<

			//newObject->object_classification = object_classification_merge(tmpCC->objects); //refer to Bird et al.
			//newObject->object_classification = object_classification_merge(newObject->occlusion_list);

			//for (ptrObj2= tmpCC->objects.begin(); ptrObj2 != tmpCC->objects.end(); ptrObj2++)
			//{
			//	if (newObject == (*ptrObj2)) continue;//skip the object itself
			//	(*ptrObj2)->matched = false;
			//	//insert_object_occlusionlist(tempObj,tempObj,(*ptrObj),occlusion_group_number);
			//	insert_object_occlusionlist(newObject,newObject,(*ptrObj2)); 

			//}

			(*ptrObj)->matched = false;
			insert_object_occlusionlist(newObject,newObject,(*ptrObj)); 
			newObject->object_classification = object_classification_merge(newObject->occlusion_list);


			//give the object the time of the most recent child object
			newObject->time = -1; //initial value to find the newest time later
			//for (ptrObj2= tmpCC->objects.begin(); ptrObj2 != tmpCC->objects.end(); ptrObj2++)
			for (ptrObj2= newObject->occlusion_list.begin(); ptrObj2 != newObject->occlusion_list.end(); ptrObj2++)
			{
				if ((*ptrObj2)->time > newObject->time)
					newObject->time = (*ptrObj2)->time;

				//if these objects participated in split, then dont update the split part later
				//if (tmpCC) 
				//{
				//	tmpCC = NULL;
				//	//(*ptrObj2)->candidateResolveHistoryParentList = NULL;

				//	if( glbl_var.saveFramesToHard)
				//		ObjectProcessingFile << "Cancelled from update because of merge participation CC " << (*ptrCC)->blob->ID << " is Obj " << (*ptrObj2)->blob->ID << endl;
				//}

			}

			//handle histories for children
			if(glbl_var.saveFramesToHard && glbl_var.saveObjectFile  )
				glbl_var.BGmodel->objectFile << "-Merging: preparing children of" << newObject->blob->ID  <<endl;
			list<TrackerObject*> matchedObjList;
			matchedObjList.push_front((*ptrObj)->rearrangeParents);
			for (ptrObj2= newObject->occlusion_list.begin(); ptrObj2 != newObject->occlusion_list.end(); ptrObj2++)
			{
				(*ptrObj2)->handleHistoryMergeChild(&matchedObjList,newObject, true);

				//*ptrObj)->updateCandidate;
				//(*ptrObj2)->handleHistoryMergeChild(&,newObject);
			}





			if( glbl_var.saveFramesToHard && glbl_var.saveObjectProcessingFile )
			{
				for (ptrObj2= newObject->occlusion_list.begin(); ptrObj2 != newObject->occlusion_list.end(); ptrObj2++)
				{
					ObjectProcessingFile << (*ptrObj2)->blob->ID << " , " ;
				}
				ObjectProcessingFile << endl ;
			}

			if( glbl_var.saveFramesToHard && glbl_var.saveObjectProcessingFile )
			{
				for (ptrObj2= tmpCC->objects.begin(); ptrObj2 != tmpCC->objects.end(); ptrObj2++)
					ObjectProcessingFile << "object " << (*ptrObj2)->blob->ID << " classified as  " << ( (*ptrObj2)->object_classification == UNKNOWN ? "U" :((*ptrObj2)->object_classification == PERSON ? "P" : ((*ptrObj2)->object_classification == STILL_PERSON ? "SP" : "O" )))<<endl;
			}


			//remove other objects from the CC object list
			tmpCC->objects.clear();
			tmpCC->objects.push_back( newObject);




		}
		else
		{


			//update the model but also reset kalman
			update_Obj_CC((*ptrObj),(*ptrObj)->updateCandidate,(*ptrObj)->updateCandidate_colorDistance,1,1);
			//calculate histories and possibilities for the object and its children
			//(*ptrObj)->updateObjAndChildrenHistories((*ptrObj)->updateCandidate);
			(*ptrObj)->updateObjAndChildrenHistories_split((*ptrObj)->updateCandidate);
			//resolve ambiguities of those objects who became visible after a split
			(*ptrObj)->resolveHistoryAmbiguities();
		}







		//update object classification
		//(*ptrObj)->object_classification = (*ptrObj)->updateCandidate_object_classification;
		//(*ptrObj)->PersonGroup = (*ptrObj)->updateCandidate_PersonGroup;


		if ((*ptrObj)->moment < TYPE_THRESHOLD)
			(*ptrObj)->type = 0; //bag
		else
			(*ptrObj)->type = 1; //person
	}

}



void BG_codebook_model::oneToOne()
{

	list<TrackerObject*>::iterator ptrObj;
	list<TrackerObject*>::iterator ptrObj2;
	list<TrackerObject*>::iterator ptrBestObj;
	list<ConnectedComponent*>::iterator ptrCC;
	list<TrackerObject*> matchedObjects;
	list<TrackerObject*>::iterator matchedObj_itr;

	//we have 3 phases: first we associate with visible good spatially and color
	//second, we associate with invisible and good color
	//third we associate with visible and color.

	int phase = 1;
	int NUMBEROFPHASES = 3;
	while (phase <= NUMBEROFPHASES) //phases 4 discarded for now.
	{
		if( glbl_var.saveFramesToHard && glbl_var.saveObjectProcessingFile )
			ObjectProcessingFile << "Phase " << phase <<endl;

		//correlate
		for (ptrCC= BGSubtractionBlobs.begin(); ptrCC != BGSubtractionBlobs.end(); ptrCC++)
		{
			//only if not matched yet
			if ((*ptrCC)->matched) continue;
			if ((*ptrCC)->ghost) continue;

			matchedObjects.clear();
			bool flag = false; // used to avoid an empty list of objects
			for (ptrObj= trackerBlobs.begin(); ptrObj != trackerBlobs.end(); ptrObj++)
			{
				//only if no occlusion
				//if ((*ptrObj)->occlusion) continue;

				//only if not already matched 
				if ((*ptrObj)->matched) continue;

				//only if not a child of another blob
				if ((*ptrObj)->occlusion_child) continue;

				//1
				TrackerObject* best_object_within_occlusion= NULL;
				//take partial matching
				double distance = colorDistance((*ptrObj), (*ptrCC), (glbl_var.useHistDifference ? 2:1 ));

				if( glbl_var.saveFramesToHard && glbl_var.saveObjectProcessingFile )
					ObjectProcessingFile << "Object " << (*ptrObj)->blob->ID << " and CC " << (*ptrCC)->blob->ID << " have distance " << distance ;

				float ratioW, ratioH=0;
				//if(!glbl_var.comparableInSize((*ptrObj)->blob, (*ptrCC)->blob,ratioW,ratioH )) //2006
				if(!glbl_var.comparableInSize((*ptrObj)->area, (*ptrCC)->area, ratioW )) //station
				{
					if( glbl_var.saveFramesToHard && glbl_var.saveObjectProcessingFile )
						ObjectProcessingFile << "Object " << (*ptrObj)->blob->ID << " and CC " << (*ptrCC)->blob->ID << " are not comparable in size. ratio = " << ratioW << " , " << ratioH <<endl;


					continue; //they should be comparable in size
				}
				else
					ObjectProcessingFile << ". ratio = " << ratioW << " , " << ratioH << endl;



				if (distance <= CC_Obj_max_acceptable_distance_onetoone) //if distance if acceptable
				{
					(*ptrObj)->distanceFromCC = distance;
					//best_object_within_occlusion = (*ptrObj);
					//}

					//if (best_object_within_occlusion)//if a match is found
					//{
					//matchedObjects.insert(matchedObjects.end(),best_object_within_occlusion);
					matchedObjects.push_front((*ptrObj)); //best_object_within_occlusion
					if( glbl_var.saveFramesToHard && glbl_var.saveObjectProcessingFile )
						ObjectProcessingFile << "Object " << (*ptrObj)->blob->ID << " added to candidates of CC " << (*ptrCC)->blob->ID <<endl; //best_object_within_occlusion
				}



			}

			ptrBestObj = matchedObjects.end();
			if (!matchedObjects.empty())
			{

				float min_distance_2 = float::MaxValue;
				float distance_2;
				double best_distance = double::MaxValue;
				float distance_x, distance_y;
				for (matchedObj_itr= matchedObjects.begin(); matchedObj_itr != matchedObjects.end(); matchedObj_itr++)
				{

					if ((*matchedObj_itr)->matched) continue;


					CvBlob pred_pos;
					bool as_predicted;
					bool usePredictionDistance = true; //general : true
					double temp_distance[2];
					switch(phase)
					{

					case 1: //visible object with intersection
						distance_2 = float::MaxValue;
						pred_pos = (*matchedObj_itr)->predictedPosition();
						//as_predicted = intersect(&pred_pos,(*ptrCC)->blob);
						//as_predicted = pixelIntersect((*matchedObj_itr),(*ptrCC),true);//general true
						as_predicted = pixelToPixelIntersect((*matchedObj_itr),(*ptrCC),true);
						
						if (as_predicted && (*matchedObj_itr)->isVisible) //intersection condition
						{
							distance_x = ((*matchedObj_itr)->predictedPosition().x - ((*ptrCC)->blob->x +(*ptrCC)->blob->w/2) );
							distance_y = ((*matchedObj_itr)->predictedPosition().y - ((*ptrCC)->blob->y +(*ptrCC)->blob->h/2) );
							temp_distance[0] = distance_x*distance_x + distance_y*distance_y;

							distance_x = ((*matchedObj_itr)->blob->x - ((*ptrCC)->blob->x +(*ptrCC)->blob->w/2) );
							distance_y = ((*matchedObj_itr)->blob->y - ((*ptrCC)->blob->y +(*ptrCC)->blob->h/2) );
							temp_distance[1] = distance_x*distance_x + distance_y*distance_y;

							//distance_2 = distance_x*distance_x + distance_y*distance_y;
							if(temp_distance[0] < temp_distance[1])
								distance_2 = temp_distance[0];
							else
								distance_2 = temp_distance[1];
						}
						if ((*matchedObj_itr)->isVisible && min_distance_2 > distance_2)  //visible and best spatial distance
						{
							min_distance_2 = distance_2;
							ptrBestObj = matchedObj_itr;
							best_distance = (*ptrBestObj)->distanceFromCC;

							//check if any of the children is a match
							//for (ptrObj2 = (*matchedObj_itr)->occlusion_list.begin(); ptrObj2 != (*matchedObj_itr)->occlusion_list.end(); ptrObj2++) 
							//{
							//	if ((*matchedObj_itr)->distanceFromCC < CC_Obj_max_acceptable_distance)
							//	{
							//			ptrBestObj = ptrObj2;
							//			best_distance = (*ptrBestObj)->distanceFromCC;
							//	}
							//}

						} 
						break;


					case 2: //visible object with  best spatial distance
						break;
						distance_2 = float::MaxValue;
						pred_pos = (*matchedObj_itr)->predictedPosition();
						//as_predicted = intersect(&pred_pos,(*ptrCC)->blob);
						//if (as_predicted && (*matchedObj_itr)->isVisible) //intersection condition
						//{
						distance_x = ((*matchedObj_itr)->predictedPosition().x - ((*ptrCC)->blob->x +(*ptrCC)->blob->w/2) );
						distance_y = ((*matchedObj_itr)->predictedPosition().y - ((*ptrCC)->blob->y +(*ptrCC)->blob->h/2) );
						distance_2 = distance_x*distance_x + distance_y*distance_y;
						//}
						if ((*matchedObj_itr)->isVisible && min_distance_2 > distance_2)  //visible and best spatial distance
						{
							min_distance_2 = distance_2;
							ptrBestObj = matchedObj_itr;
							best_distance = (*ptrBestObj)->distanceFromCC;

							//check if any of the children is a match
							//for (ptrObj2 = (*matchedObj_itr)->occlusion_list.begin(); ptrObj2 != (*matchedObj_itr)->occlusion_list.end(); ptrObj2++) 
							//{
							//	if ((*matchedObj_itr)->distanceFromCC < CC_Obj_max_acceptable_distance)
							//	{
							//			ptrBestObj = ptrObj2;
							//			best_distance = (*ptrBestObj)->distanceFromCC;
							//	}
							//}

						} 
						break;
					case 3: //nonvisible nonchild object with best spatial distance
						distance_2 = float::MaxValue;
						pred_pos = (*matchedObj_itr)->predictedPosition();
						//as_predicted = intersect(&pred_pos,(*ptrCC)->blob);
						as_predicted = pixelIntersect((*matchedObj_itr),(*ptrCC),true); //general true
						if (as_predicted && !(*matchedObj_itr)->isVisible)
						{
							distance_x = ((*matchedObj_itr)->predictedPosition().x - ((*ptrCC)->blob->x +(*ptrCC)->blob->w/2) );
							distance_y = ((*matchedObj_itr)->predictedPosition().y - ((*ptrCC)->blob->y +(*ptrCC)->blob->h/2) );
							distance_2 = distance_x*distance_x + distance_y*distance_y;
						}
						//if (!(*matchedObj_itr)->isVisible && min_distance_2 == float::MaxValue) //if it's an absent object and no visible good one is found so far
						if (!(*matchedObj_itr)->isVisible  && !(*matchedObj_itr)->occlusion_child) //if it's an absent object and no visible good one is found so far  && ( absenceFrames(*matchedObj_itr) > 1)
						{
							if (  min_distance_2 > distance_2) //(*matchedObj_itr)->distanceFromCC  < best_distance &&
							{
								best_distance = (*matchedObj_itr)->distanceFromCC;
								ptrBestObj = matchedObj_itr;
								min_distance_2 = distance_2;
							}
						}
						break;


					case 4: //nonvisible nonchild object with best spatial distance without the absence condition
						distance_2 = float::MaxValue;
						pred_pos = (*matchedObj_itr)->predictedPosition();
						//as_predicted = intersect(&pred_pos,(*ptrCC)->blob);
						as_predicted = pixelIntersect((*matchedObj_itr),(*ptrCC),true);
						if (as_predicted && !(*matchedObj_itr)->isVisible)
						{
							distance_x = ((*matchedObj_itr)->predictedPosition().x - ((*ptrCC)->blob->x +(*ptrCC)->blob->w/2) );
							distance_y = ((*matchedObj_itr)->predictedPosition().y - ((*ptrCC)->blob->y +(*ptrCC)->blob->h/2) );
							distance_2 = distance_x*distance_x + distance_y*distance_y;
						}
						//if (!(*matchedObj_itr)->isVisible && min_distance_2 == float::MaxValue) //if it's an absent object and no visible good one is found so far
						if (!(*matchedObj_itr)->isVisible && !(*matchedObj_itr)->occlusion_child ) //if it's an absent object and no visible good one is found so far //&& ( absenceFrames(*matchedObj_itr) > 1)
						{
							if (  min_distance_2 > distance_2) //(*matchedObj_itr)->distanceFromCC  < best_distance &&
							{
								best_distance = (*matchedObj_itr)->distanceFromCC;
								ptrBestObj = matchedObj_itr;

								min_distance_2 = distance_2;


							}
						}
						break;

					case 5: //nonvisible nonchild object best color match
						//break;

						//if (!(*matchedObj_itr)->isVisible && min_distance_2 == float::MaxValue) //if it's an absent object and no visible good one is found so far
						if (!(*matchedObj_itr)->isVisible && !(*matchedObj_itr)->occlusion_child ) //if it's an absent object and no visible good one is found so far //&& ( absenceFrames(*matchedObj_itr) > 1)
						{
							if ( (*matchedObj_itr)->distanceFromCC  < best_distance ) //&& min_distance_2 > distance_2
							{
								best_distance = (*matchedObj_itr)->distanceFromCC;
								ptrBestObj = matchedObj_itr;

							}
						}
						break;


					case 6: //visible object
						//break;
						//if ((*matchedObj_itr)->isVisible && ptrBestObj == matchedObjects.end() ) //if none of above could be found, then a visible match is OK.
						if ((*matchedObj_itr)->isVisible  ) //if none of above could be found, then a visible match is OK.
						{
							ptrBestObj = matchedObj_itr;
							best_distance = (*ptrBestObj)->distanceFromCC;
						}
						break;

					}

					if( glbl_var.saveFramesToHard && glbl_var.saveObjectProcessingFile )
						ObjectProcessingFile << "Object " << (*matchedObj_itr)->blob->ID << " is " <<((*matchedObj_itr)->isVisible ? "visible" : "invisible") << " with spatial distance " << distance_2 << " and spectral distance " << (*matchedObj_itr)->distanceFromCC  << endl;

				}


				if (ptrBestObj != matchedObjects.end())
				{
					if( glbl_var.saveFramesToHard && glbl_var.saveObjectProcessingFile )
						ObjectProcessingFile << "best Object for CC " << (*ptrCC)->blob->ID << " is Object " << (*ptrBestObj)->blob->ID  <<endl;

					//if it's the parent has only one child, delete it and replace with the child. (ptrBestObj = child)
					bool onlyChild = ((*ptrBestObj)->occlusion_list.size() == 1);
					if(onlyChild)
					{
						TrackerObject* replacingBestObject;
						replacingBestObject = (*(*ptrBestObj)->occlusion_list.begin());
						//you must also delete all history point of this object
						list<historyPoint*>::iterator hpItr;
						if( replacingBestObject->currentHistoryMeasurement && !replacingBestObject->currentHistoryMeasurement->empty() && (*replacingBestObject->currentHistoryMeasurement->begin())->frameNumber == t)
							replacingBestObject->currentHistoryMeasurement->clear();
						//you must also delete all other instances (just like rearrange parents)
						//remove from occlusion list of the parents
						for (matchedObj_itr= replacingBestObject->parent.begin(); matchedObj_itr != replacingBestObject->parent.end(); matchedObj_itr++)
							(*matchedObj_itr)->occlusion_list.remove(replacingBestObject);
						//has no parents anymore
						replacingBestObject->parent.clear();
						//not a child anymore
						replacingBestObject->occlusion_child = false;
						//replace
						replacingBestObject->rearrangeParents = (*ptrBestObj);
						ptrBestObj = find(trackerBlobs.begin(),trackerBlobs.end(),replacingBestObject);//ptrBestObj = replacingBestObject;

					}

					update_Obj_CC(*ptrBestObj,*ptrCC,best_distance,1,(phase == 3) || onlyChild); //resetKalma = 0
					//calculate histories and possibilities for the object and its children
					(*ptrBestObj)->updateObjAndChildrenHistories(*ptrCC);

					if( glbl_var.saveFramesToHard && glbl_var.saveObjectProcessingFile )
						ObjectProcessingFile << "object " << (*ptrBestObj)->blob->ID << " classified as  " <<  ((*ptrBestObj)->object_classification == UNKNOWN ? "U" :((*ptrBestObj)->object_classification == PERSON ? "P" : ((*ptrBestObj)->object_classification == STILL_PERSON ? "SP" : "O" )))<<endl;

					if ((*ptrBestObj)->moment < TYPE_THRESHOLD)
						(*ptrBestObj)->type = 0; //bag
					else
						(*ptrBestObj)->type = 1; //person

				}

			}
		}

		phase++;
	}


}

void BG_codebook_model::createNewObjects()
{
	list<ConnectedComponent*>::iterator ptrCC;

	//Create new objects for unmatched CCs (new subjects in the scene)
	for (ptrCC= BGSubtractionBlobs.begin(); ptrCC != BGSubtractionBlobs.end(); ptrCC++)
	{
		if ((*ptrCC)->ghost) continue;

		if ((*ptrCC)->matched == 0)
		{
			TrackerObject* tempObj = createObject((*ptrCC),nextID);
			tempObj->updateObjAndChildrenHistories((*ptrCC));
			findOwner(tempObj);
			trackerBlobs.push_back(tempObj);

			if( glbl_var.saveFramesToHard && glbl_var.saveObjectProcessingFile )
				ObjectProcessingFile << "new Object created for CC " << (*ptrCC)->blob->ID << " with object ID " << tempObj->blob->ID  <<endl;
		}
	}


}


void BG_codebook_model::deleteAbsentObjects()
{
	list<TrackerObject*>::iterator ptrObj;
	list<TrackerObject*>::iterator ptrObj2;


	//delete tracker blobs that are not associated
	//flag_start = false;
	ptrObj= trackerBlobs.begin();
	while ( ptrObj != trackerBlobs.end())
	{

		bool flag_end = false;
		//ptrObj->absence > MAX_ABSENCE ?
		if (!(*ptrObj)->matched) //if not matched
		{
			if ( ((*ptrObj)->absence != -1) && ( absenceFrames(*ptrObj) > MAX_ABSENCE) && !(*ptrObj)->occlusion_child ) //if for a long time, delete it // delete only non children objects  
			{
				//ptrObj2 = trackerBlobs.end();
				//ptrObj2--;
				////ptrObj2 = trackerBlobs.end();
				////if( (*ptrObj)->blob->ID == (*ptrObj2)->blob->ID) //if end has been reached
				//if (ptrObj == ptrObj2)
				//	flag_end = true;

				//ptrObj2 = ptrObj;

				//if(!flag_end) //if i's not the end
				//{
				//	//ptrObj2 = trackerBlobs.begin();
				//	//if ((*ptrObj)->blob->ID == (*ptrObj2)->blob->ID) //if it's the head of the list
				//	if (ptrObj == trackerBlobs.begin())
				//	{
				//		ptrObj++;
				//		//flag_start = true;
				//	}
				//	else
				//		ptrObj--; //go back by one so you don't miss because of the shift
				//}

				//if( glbl_var.saveFramesToHard)
				//	ObjectProcessingFile << "Object " << (*ptrObj2)->blob->ID << " deleted for absence "  <<endl;

				////trackerBlobs.erase(ptrObj2);

				//erase_object(*ptrObj2);

				if( glbl_var.saveFramesToHard && glbl_var.saveObjectProcessingFile )
					ObjectProcessingFile << "Object " << (*ptrObj)->blob->ID << " and its children deleted for absence "  <<endl;


				erase_object(*ptrObj);
				ptrObj= trackerBlobs.begin();






			}
			else if ((*ptrObj)->absence == -1)
			{
				(*ptrObj)->absence = t-1;
				(*ptrObj)->persistence = -1;
				(*ptrObj)->isVisible = false;

				if( glbl_var.saveFramesToHard && glbl_var.saveObjectProcessingFile )
					ObjectProcessingFile << "Object " << (*ptrObj)->blob->ID << " is now absent "  <<endl;

				ptrObj++;

			}
			else
				ptrObj++;
			//else
			//	(*ptrObj)->isVisible = false; //not deleted but not visible either
			//else //if not, increment absence counter
			//	ptrObj->absence++;
		}
		else //if matched, reset absence
		{
			(*ptrObj)->absence =-1 ;

			if ((*ptrObj)->persistence <=0) 
			{
				if( glbl_var.saveFramesToHard && glbl_var.saveObjectProcessingFile )
					ObjectProcessingFile << "Object " << (*ptrObj)->blob->ID << " is now visible "  <<endl;

				(*ptrObj)->persistence = t -1;
			}

			ptrObj++;

			//(*ptrObj)->isVisible = true;


		}

		//if (flag_end) break;
	}


}

void BG_codebook_model::updateObjectsLastHistory()
{
	list<TrackerObject*>::iterator ptrObj;

	//update last history of all objects
	for(ptrObj= trackerBlobs.begin(); ptrObj != trackerBlobs.end(); ptrObj++)
	{
		//if(!(*ptrObj)->isVisible) continue; //they must be visible

		//update lastVelocityTime
		if (!(*ptrObj)->lastHistoryMeasurement || (*ptrObj)->newMeasurement) //first time
		{
			(*ptrObj)->lastHistoryMeasurement = (*ptrObj)->currentHistoryMeasurement;
			(*ptrObj)->newMeasurement = false;
		}



	}

	/*if(glbl_var.saveFramesToHard)
	{
	if((*ptrObj) && (*ptrObj)->lastHistoryMeasurement && !(*ptrObj)->lastHistoryMeasurement->empty())
	glbl_var.BGmodel->objectFile << "-Last history time for all objects now is : " << (*(*ptrObj)->lastHistoryMeasurement->begin())->time<<endl ;
	}*/


}

void BG_codebook_model::assignIDsToHpsAndMps()
{
	list<TrackerObject*>::iterator ObjItr;
	list<historyPoint*>::iterator HpItr;
	list<motionPossibility*>::iterator MpItr;

	for(ObjItr = trackerBlobs.begin() ; ObjItr != trackerBlobs.end() ; ObjItr++) // for all objects
	{
		int counter_hp = 0;
		for(HpItr = (*ObjItr)->currentHistoryMeasurement->begin() ; HpItr != (*ObjItr)->currentHistoryMeasurement->end() ; HpItr++) //for all history points
		{
			(*HpItr)->ID = counter_hp++;
			int counter_mp = 0;
			for(MpItr = (*HpItr)->motionPossibilities.begin() ; MpItr != (*HpItr)->motionPossibilities.end() ; MpItr++) //for all motion possibilities
			{
				(*MpItr)->ID = counter_mp++;
			}
		}

	}
}


void  BG_codebook_model::updateNonVisibleObjects()
{
	list<TrackerObject*>::iterator ptrObj;

	//update spatially for objects not visible or in occlusion
	for (ptrObj= trackerBlobs.begin(); ptrObj != trackerBlobs.end(); ptrObj++)
	{
		if ((*ptrObj)->isVisible && !(*ptrObj)->occlusion) continue;



		//update previous location
		float past_x = (*ptrObj)->blob->x;
		float past_y = (*ptrObj)->blob->y;
		(*ptrObj)->prev_blob->x = past_x;
		(*ptrObj)->prev_blob->y = past_y;

		float predicted_x = (*ptrObj)->predictedPosition().x;
		float predicted_y = (*ptrObj)->predictedPosition().y;
		(*ptrObj)->blob->x = predicted_x;
		(*ptrObj)->blob->y = predicted_y;

		CvMat* measurment = cvCreateMat(4,1,CV_32FC1);
		cvmSet(measurment,0,0,(*ptrObj)->blob->x);
		cvmSet(measurment,1,0,(*ptrObj)->blob->y);
		cvmSet(measurment,2,0,(*ptrObj)->blob->x - (*ptrObj)->prev_blob->x);
		cvmSet(measurment,3,0,(*ptrObj)->blob->y - (*ptrObj)->prev_blob->y) ;
		cvKalmanCorrect((*ptrObj)->kalman,measurment);
		cvReleaseMat(&measurment);

		if( glbl_var.saveFramesToHard && glbl_var.saveObjectProcessingFile )
			ObjectProcessingFile << "Object " << (*ptrObj)->blob->ID << " is not visible or occlusion and updated to its prediction "  << (*ptrObj)->predictedPosition().x << " , " << (*ptrObj)->predictedPosition().y << endl;


	}
}



void BG_codebook_model::updateObjectsKalman()
{
	list<TrackerObject*>::iterator ptrObj;

	//update kalman for all blobs
	for (ptrObj= trackerBlobs.begin(); ptrObj != trackerBlobs.end(); ptrObj++)
	{
		(*ptrObj)->Predict();

		if( glbl_var.saveFramesToHard && glbl_var.saveObjectProcessingFile )	
		{
			ObjectProcessingFile << "Object " << (*ptrObj)->blob->ID << " is currently at "  << (*ptrObj)->blob->x << " , " << (*ptrObj)->blob->y << " ,dimensions: " << (*ptrObj)->blob->w << " , " << (*ptrObj)->blob->h << ",area = " << (*ptrObj)->area << endl;
			ObjectProcessingFile << "\tis predicted is to be next frame at "  << (*ptrObj)->predictedPosition().x << " , " << (*ptrObj)->predictedPosition().y << endl;
			ObjectProcessingFile << "\tPossibilities of the base:"<<endl;
			list<historyPoint*>::iterator HPptr;
			for(HPptr = (*ptrObj)->currentHistoryMeasurement->begin(); HPptr!= (*ptrObj)->currentHistoryMeasurement->end(); HPptr++)
				ObjectProcessingFile << "\tis based at "  << (*HPptr)->xBase << " , " << (*HPptr)->yBase<< endl;

		}

	}
}

void BG_codebook_model::correlateBlobs()
{


	if(glbl_var.saveFramesToHard)
	{

		char ObjectProcessingFilename[100]; 
		if(glbl_var.saveObjectProcessingFile )
		{

			strcpy(ObjectProcessingFilename,glbl_var.saveFramesTo);
			strcat(ObjectProcessingFilename,"tracker.log");
			glbl_var.BGmodel->ObjectProcessingFile.open(ObjectProcessingFilename,ios::app);
		}

		if(glbl_var.saveObjectFile  )
		{
			strcpy(ObjectProcessingFilename,glbl_var.saveFramesTo);
			strcat(ObjectProcessingFilename,"ObjectBehaviour.log");
			glbl_var.BGmodel->objectFile.open(ObjectProcessingFilename,ios::app);
		}

		if(glbl_var.saveObjectHistoryFiles  )
		{
			strcpy(ObjectProcessingFilename,glbl_var.saveFramesTo);
			strcat(ObjectProcessingFilename,"ObjectHistory.log");
			glbl_var.BGmodel->objectHistoryFile.open(ObjectProcessingFilename,ios::app);
			//objectHistoryFile << "frame " << t<< " time" << glbl_var.sampledAbsoluteTime << endl;
		}

	}

	if(glbl_var.saveFramesToHard && glbl_var.saveObjectProcessingFile )
	{
		ObjectProcessingFile << "frame " << t<< endl;
	}
	if(glbl_var.saveFramesToHard && glbl_var.saveObjectFile )
	{
		objectFile << "frame " << t<< endl;
	}


	list<TrackerObject*>::iterator ptrObj;
	list<TrackerObject*>::iterator ptrObj2;
	list<TrackerObject*>::iterator init_ptrObj2;
	list<TrackerObject*>::iterator ptrBestObj;
	list<ConnectedComponent*>::iterator ptrCC;
	list<ConnectedComponent*>::iterator ptrCC2;
	list<TrackerObject*> matchedObjects;
	list<TrackerObject*>::iterator matchedObj_itr;




	//preprocess the blob
	if( glbl_var.saveFramesToHard && glbl_var.saveObjectProcessingFile )
		ObjectProcessingFile << "-Contour contrast" <<endl;
	blobPreprocessing();


	//delete just-got-invisible boarder objects
	deleteBoarderObjects();
	deleteNonvisibleNonpersistentObjects();

	//reset history
	resetObjectHistory();

	if( glbl_var.saveFramesToHard && glbl_var.saveObjectProcessingFile )
	{
		ObjectProcessingFile << "-Spatial Split handling" <<endl;
	}
	if(glbl_var.saveFramesToHard && glbl_var.saveObjectFile )
	{
		objectFile << "-Spatial Split handling" <<endl;
	}
	ObjectSplitting();



	if( glbl_var.saveFramesToHard && glbl_var.saveObjectProcessingFile )
	{
		ObjectProcessingFile << "-Spatial Merge handling" <<endl;
	}
	if(glbl_var.saveFramesToHard && glbl_var.saveObjectFile )
	{
		objectFile << "-Spatial Merge handling" <<endl;
	}
	ObjectMerging();
	ApplyPendingSplits();


	if( glbl_var.saveFramesToHard && glbl_var.saveObjectProcessingFile )
	{
		ObjectProcessingFile << "-Correlation" <<endl;
	}
	if(glbl_var.saveFramesToHard && glbl_var.saveObjectFile)
	{
		objectFile << "-Correlation" <<endl;
	}
	oneToOne();




	if( glbl_var.saveFramesToHard && glbl_var.saveObjectProcessingFile )
	{
		ObjectProcessingFile << "-Creating New Objects" <<endl;
	}
	if(glbl_var.saveFramesToHard && glbl_var.saveObjectFile)
	{
		objectFile << "-Creating New Objects" <<endl;
	}


	createNewObjects();


	if( glbl_var.saveFramesToHard && glbl_var.saveObjectProcessingFile )
		ObjectProcessingFile << "-Checkeing for Deletion Objects" <<endl;
	deleteAbsentObjects();


	if( glbl_var.saveFramesToHard && glbl_var.saveObjectProcessingFile )
		ObjectProcessingFile << "-Updating Nonvisible Objects" <<endl;
	updateNonVisibleObjects();


	if( glbl_var.saveFramesToHard && glbl_var.saveObjectProcessingFile )
		ObjectProcessingFile << "-Updating Kalman"  <<endl;
	updateObjectsKalman();


	//prediction file
	if( glbl_var.saveFramesToHard && glbl_var.saveObjectPredictions )
	{
		ofstream prediction;
		char ObjectProcessingFilename[100]; 
		strcpy(ObjectProcessingFilename,glbl_var.saveFramesTo);
		strcat(ObjectProcessingFilename,"prediction.csv");
		prediction.open(ObjectProcessingFilename,ios::app);
		bool found = false;
		int index = 0;
		int max = 0;
		for (ptrObj= trackerBlobs.begin(); ptrObj != trackerBlobs.end(); ptrObj++)
		{
			if (max < (*ptrObj)->blob->ID) max = (*ptrObj)->blob->ID;
		}
		for (int i=0; i < max; i++)
		{
			for (ptrObj= trackerBlobs.begin(); ptrObj != trackerBlobs.end(); ptrObj++)
			{
				if ((*ptrObj)->blob->ID == index)
				{
					prediction << (*ptrObj)->blob->x << "," << (*ptrObj)->blob->y << ","<< (*ptrObj)->predictedPosition().x << ","<< (*ptrObj)->predictedPosition().y << ",";
					found = true;
				}
			}
			if (!found)
				prediction << ",,,," ;

			found = false;
			index++;

		}
		prediction << endl;
		prediction.close();
	}











	////check if objects are moving or nonmoving
	//for(ptrObj= trackerBlobs.begin(); ptrObj != trackerBlobs.end(); ptrObj++)
	//{
	//	if((*ptrObj)->movingObject) continue; // if object is already known to be a moving object, then skip
	//	//if(!(*ptrObj)->occlusion_list.empty()) continue; // only for standalone objects or occluded (but not occluders)
	//	if ((*ptrObj)->occlusion_child) continue; //only for standalone objects (but not occluders)
	//	if((*ptrObj)->history.empty()) continue; //if it has only been observed fro one frame, no enough info is available

	//	historyPoint* first = (*(*ptrObj)->history.begin());
	//	double movedDistanceX = (*ptrObj)->xBase - first->xBase;
	//	double movedDistanceY = (*ptrObj)->yBase - first->yBase;

	//	if( glbl_var.saveFramesToHard)
	//			ObjectProcessingFile << "Object " << (*ptrObj)->blob->ID << " moved from first position by " << cvSqrt(movedDistanceX*movedDistanceX + movedDistanceY*movedDistanceY) << endl;

	//	//if it's stand alone and has moved 
	//	if( movedDistanceX*movedDistanceX + movedDistanceY*movedDistanceY > MOTION_TOLERANCE*MOTION_TOLERANCE )
	//	{
	//		(*ptrObj)->movingObject = true;

	//		if( glbl_var.saveFramesToHard)
	//			ObjectProcessingFile << "Object " << (*ptrObj)->blob->ID << " is now set to be a non-moving object " << endl;
	//			
	//	}



	//	//else((*ptrObj)->occlusion_child ) //if part of an occlusion
	//	//{
	//	//	//if none of the parents are covering the last seen position, then it has moved
	//	//	bool found = false;
	//	//	for(ptrObj2= (*ptrObj)->parent.begin(); ptrObj2 != (*ptrObj)->parent.end(); ptrObj2++)
	//	//	{
	//	//		if (glbl_var.BGlabels[cvFloor((*ptrObj)->blob->x) + w*cvFloor((*ptrObj)->blob->y)] == (*ptrObj2)->blob->ID)
	//	//		{
	//	//			found = true;
	//	//			break;
	//	//		}
	//	//	}
	//	//	if (!found) 
	//	//	{
	//	//		(*ptrObj)->movingObject = true;

	//	//		if( glbl_var.saveFramesToHard)
	//	//			ObjectProcessingFile << "Object " << (*ptrObj)->blob->ID << " is now set to be a non-moving object " << endl;
	//	//	}

	//	//}


	//}

	updateObjectsLastHistory();


	////update occlusion object histories
	//for(ptrObj= trackerBlobs.begin(); ptrObj != trackerBlobs.end(); ptrObj++)
	//{
	//	if((*ptrObj)->parent.empty()) continue; //they have to have a parent

	//	//one of the parents should be visible
	//	bool found = false;
	//	for(ptrObj2= (*ptrObj)->parent.begin(); ptrObj2 != (*ptrObj)->parent.end(); ptrObj2++)
	//	{
	//		if ((*ptrObj2)->isVisible)
	//		{
	//			found = true;
	//			break;
	//		}
	//	}
	//	if (!found) continue;

	//	//list<historyPoint*>* newList = new list<historyPoint*>;
	//	//(*ptrObj)->history.insert((*ptrObj)->history.begin(),newList);


	//	// create a history point for each parent
	//	for(ptrObj2= (*ptrObj)->parent.begin(); ptrObj2 != (*ptrObj)->parent.end(); ptrObj2++)
	//	{

	//		if (!(*ptrObj2)->isVisible) continue;

	//		list<historyPoint*>::iterator ptrHP;
	//		list<list<historyPoint*>*>::iterator ptrHP2;

	//		double t1 = glbl_var.sampledAbsoluteTime;
	//		double t2 = (*(*ptrObj)->lastHistoryMeasurement->begin())->time;
	//		historyPoint* newPoint = new historyPoint();
	//		newPoint->xBase = (*(*ptrObj2)->currentHistoryMeasurement->begin())->xBase;
	//		newPoint->yBase = (*(*ptrObj2)->currentHistoryMeasurement->begin())->yBase;
	//		newPoint->relatedParent = (*ptrObj2);
	//		
	//		//updating preceding point
	//		if(1)
	//		{
	//		}
	//		else
	//		{
	//		}

	//		

	//		//calculate motion characteristics every TIME_MEASUREMENT_STEP
	//		double time_difference = t1 - t2;
	//		if ( time_difference >= TIME_MEASUREMENT_STEP)
	//		{
	//			//bool relatedParentFound = false;
	//			for(ptrHP= (*ptrObj)->lastHistoryMeasurement->begin(); ptrHP != (*ptrObj)->lastHistoryMeasurement->end(); ptrHP++) //find related parent if it exists
	//			{
	//				if((*ptrObj2)->blob->ID == (*ptrHP)->relatedParent->blob->ID)
	//				{
	//					//relatedParentFound = true;
	//					newPoint = historyPointOfDifference((*(*ptrObj2)->currentHistoryMeasurement->begin()),(*ptrHP));
	//						break;
	//				}
	//			}

	//			//if(relatedParentFound) //if there is a related parent in the history
	//			//(*ptrObj)->currentHistoryMeasurement->insert((*ptrObj)->currentHistoryMeasurement.begin(),newPoint);
	//			//else //


	//			(*ptrObj)->newMeasurement = true;

	//		}
	//		else
	//		{
	//			ptrHP2 = (*ptrObj)->history.begin();
	//			for(ptrHP= (*ptrHP2)->begin(); ptrHP != (*ptrHP2)->end(); ptrHP++) //find related parent if it exists
	//			{
	//				if((*ptrObj2)->blob->ID == (*ptrHP)->relatedParent->blob->ID)
	//				{
	//					newPoint = new historyPoint(*ptrHP);
	//					break;
	//				}
	//			}


	//			//(*ptrObj)->currentHistoryMeasurement->insert((*ptrObj)->currentHistoryMeasurement.begin(),&historyPoint()) ;
	//		}
	//		(*ptrObj)->currentHistoryMeasurement->insert((*ptrObj)->currentHistoryMeasurement->begin(),newPoint);
	//	}


	//	(*ptrObj)->history.push_front((*ptrObj)->currentHistoryMeasurement);


	//		//if(!(*ptrObj2)->isVisible) continue; //only visible parents

	//		//historyPoint* newPoint;
	//		//newPoint->xBase = (*(*ptrObj2)->currentHistoryMeasurement.begin())->xBase; //the parent will definitely have a single current measurement because it is visible
	//		//newPoint->yBase = (*(*ptrObj2)->currentHistoryMeasurement.begin())->yBase; //the parent will definitely have a single current measurement because it is visible

	//		////find the last history 
	//		//newPoint = &historyPoint((*ptrObj2),true); //passing the parent

	//		//(*ptrObj)->currentHistoryMeasurement.insert((*ptrObj)->currentHistoryMeasurement.begin(),newPoint);

	//	



	//	//historyPoint* newPoint = new historyPoint();
	//	//newPoint->xBase = (*ptrObj)->currentHistoryMeasurement->xBase;
	//	//newPoint->yBase = (*ptrObj)->currentHistoryMeasurement->yBase;
	//	//newPoint->vxBase = (*ptrObj)->currentHistoryMeasurement->vxBase;
	//	//newPoint->vyBase = (*ptrObj)->currentHistoryMeasurement->vyBase;
	//	//newPoint->speed = (*ptrObj)->currentHistoryMeasurement->speed;
	//	//newPoint->xDirection = (*ptrObj)->currentHistoryMeasurement->xDirection;
	//	//newPoint->yDirection = (*ptrObj)->currentHistoryMeasurement->yDirection;
	//	////newPoint->frameNumber =	glbl_var.numberOfReadFrame;
	//	//newPoint->frameNumber =	t;
	//	//newPoint->time = glbl_var.AbsoluteTime;
	//	//newPoint->visibility = (*ptrObj)->isVisible;
	//	//newPoint->falls = hasFallen((*ptrObj));

	//	//insert parents and their children as occlusion IDs
	//	/*for(ptrObj2= (*ptrObj)->parent.begin(); ptrObj2 != (*ptrObj)->parent.end(); ptrObj2++)
	//	{
	//		newPoint->occlusion_IDs.insert(newPoint->occlusion_IDs.begin(),(*ptrObj2)->blob->ID);
	//		for(init_ptrObj2= (*ptrObj2)->occlusion_list.begin(); init_ptrObj2 != (*ptrObj2)->occlusion_list.end(); init_ptrObj2++)
	//			newPoint->occlusion_IDs.insert(newPoint->occlusion_IDs.begin(),(*init_ptrObj2)->blob->ID);
	//	}*/

	//	//insert history point at beginning to preserve chronological order
	//	//(*ptrObj)->history.insert((*ptrObj)->history.begin(),newPoint);

	//	//update lastVelocityTime
	//	if (!(*ptrObj)->lastHistoryMeasurement || (*ptrObj)->newMeasurement)
	//	{
	//		(*ptrObj)->lastHistoryMeasurement = (*ptrObj)->currentHistoryMeasurement;
	//		(*ptrObj)->newMeasurement = false;
	//	}

	//}


	//reporting a summary of objects
	if( glbl_var.saveFramesToHard && glbl_var.saveObjectProcessingFile  )
		ObjectProcessingFile << "-Objects Summary" <<endl;
	if( glbl_var.saveFramesToHard && glbl_var.saveObjectProcessingFile  )
	{
		for (ptrObj= trackerBlobs.begin(); ptrObj != trackerBlobs.end(); ptrObj++)
		{
			ObjectProcessingFile << "Object " << (*ptrObj)->blob->ID << " "  ;
			if ((*ptrObj)->occlusion)
				ObjectProcessingFile << " grp " << (*ptrObj)->occlusion << " " ;
			if ((*ptrObj)->occlusion_child)
				ObjectProcessingFile << " is a child";
			ObjectProcessingFile << endl;
			for (ptrObj2= (*ptrObj)->occlusion_list.begin(); ptrObj2 != (*ptrObj)->occlusion_list.end(); ptrObj2++)
			{
				ObjectProcessingFile << "Sub Object " << (*ptrObj2)->blob->ID << " ";
				for (ptrBestObj= (*ptrObj2)->parent.begin(); ptrBestObj != (*ptrObj2)->parent.end(); ptrBestObj++)
					ObjectProcessingFile << "parent " << (*ptrBestObj)->blob->ID << " ";
			}
			if (!(*ptrObj)->occlusion_list.empty()) ObjectProcessingFile << endl;

		}
	}


	//Connected Component Summary
	if( glbl_var.saveFramesToHard && glbl_var.saveObjectProcessingFile  )
	{
		ObjectProcessingFile << "-CCs Summary" <<endl;
		//reporting a summary of CCs
		for (ptrCC= BGSubtractionBlobs.begin(); ptrCC != BGSubtractionBlobs.end(); ptrCC++)
			ObjectProcessingFile << "CC " << (*ptrCC)->blob->ID << " is currently at "  << (*ptrCC)->blob->x << " , " << (*ptrCC)->blob->y << " ,dimensions: " << (*ptrCC)->blob->w << " , " << (*ptrCC)->blob->h <<", area = " << (*ptrCC)->area << endl;
	}



	//print possibilities
	if( glbl_var.saveFramesToHard && glbl_var.saveObjectFile )
	{
		objectFile << "----Summary "  <<endl;


		for (ptrObj= trackerBlobs.begin(); ptrObj != trackerBlobs.end(); ptrObj++)
		{
			list<historyPoint*>::iterator ptr;
			list<motionPossibility*>::iterator ptr2;
			objectFile << "--Object ID " << (*ptrObj)->blob->ID <<endl;
			objectFile << "Possibile history point: " <<endl;
			for(ptr =  (*ptrObj)->currentHistoryMeasurement->begin() ; ptr != (*ptrObj)->currentHistoryMeasurement->end() ; ptr++ )
			{
				objectFile << "-Base x,y = " << (*ptr)->xBase << "," << (*ptr)->yBase <<  endl; 
				objectFile << ( (*ptr)->visibility ? "VISIBLE " : "NOT visible") <<  endl; 
				objectFile << ( (*ptr)->falls ? "FALLS" : "NOT falling ") <<  endl; 
				objectFile << "related parent " << (*ptr)->relatedParent->blob->ID  <<  endl; 
				objectFile << "occlusion: " ;
				for(ptrObj2 =  (*ptr)->occlusion.begin() ; ptrObj2 != (*ptr)->occlusion.end() ; ptrObj2++)
				{
					objectFile << (*ptrObj2)->blob->ID << ",";
				}
				objectFile << endl;

				for(ptr2 =  (*ptr)->motionPossibilities.begin() ; ptr2 != (*ptr)->motionPossibilities.end() ; ptr2++ )
				{
					objectFile << "Base Vx,Vy = " << (*ptr2)->vxBase << "," << (*ptr2)->vyBase <<  endl; 
					objectFile << "Direction x,y = " << (*ptr2)->xDirection << "," << (*ptr2)->yDirection <<  endl; 
					objectFile << "Speed =  " << (*ptr2)->speed <<  endl; 
					objectFile << ((*ptr2)->valid ? "GENUINE" : "DUMMY" ) <<endl;
				}
				list<TrackerObject*>::iterator ptr3;
				/*objectFile << "In same occlusion group : ";
				for (ptr3= (*ptr)->occlusion.begin(); ptr3 != (*ptr)->occlusion.end(); ptr3++)
				{	
				objectFile << (*ptr3)->blob->ID << ",";
				}
				objectFile << endl;*/
			}
			objectFile << endl;

			if((*ptrObj)->belongs_to) objectFile << "\nObject Belongs to " << (*ptrObj)->belongs_to->blob->ID <<endl;

		}


		//write object history file
		for (ptrObj= trackerBlobs.begin(); ptrObj != trackerBlobs.end(); ptrObj++)
		{

			list<historyPoint*>::iterator ptr;
			list<motionPossibility*>::iterator ptr2;
			objectHistoryFile << "--Object ID " << (*ptrObj)->blob->ID <<endl;
			list<list<historyPoint*>*>::iterator historyIterator;
			for(historyIterator =  (*ptrObj)->history.begin() ; historyIterator != (*ptrObj)->history.end() ; historyIterator++ ) 
			{
				objectHistoryFile << "\tf" << (*(*historyIterator)->begin())->frameNumber << "t" << (*(*historyIterator)->begin())->time <<endl;
				for(ptr =  (*historyIterator)->begin() ; ptr != (*historyIterator)->end() ; ptr++ ) 
				{
					objectHistoryFile << "\t\thp(" << *ptr << ")" << (*ptr)->xBase << "," << (*ptr)->yBase << ";" << ( (*ptr)->visibility ? "VISIBLE " : "NOT visible") << ", " << ( (*ptr)->falls ? "FALLS" : "NOT falling ") << ",Objects: ";
					for(ptrObj2 =  (*ptr)->occlusion.begin() ; ptrObj2 != (*ptr)->occlusion.end() ; ptrObj2++)
					{
						objectHistoryFile << (*ptrObj2)->blob->ID << "-";
					}
					objectHistoryFile << ";->" << (*ptr)->relatedParent->blob->ID  << endl ;
					for(ptr2 =  (*ptr)->motionPossibilities.begin() ; ptr2 != (*ptr)->motionPossibilities.end() ; ptr2++ )
					{
						objectHistoryFile << "\t\t\tmp("<< *ptr2 << ")" << "Base Vx,Vy = " << (*ptr2)->vxBase << "," << (*ptr2)->vyBase; 
						objectHistoryFile << ";Direction x,y = " << (*ptr2)->xDirection << "," << (*ptr2)->yDirection ; 
						objectHistoryFile << ";Speed =  " << (*ptr2)->speed ; 
						objectHistoryFile << ((*ptr2)->valid ? ";GENUINE" : ";DUMMY" ) ;
						objectHistoryFile << ";->" << (*ptr2)->precedingPoint <<endl;
					}
					/*list<TrackerObject*>::iterator ptr3;*/
					/*objectFile << "In same occlusion group : ";
					for (ptr3= (*ptr)->occlusion.begin(); ptr3 != (*ptr)->occlusion.end(); ptr3++)
					{	
					objectFile << (*ptr3)->blob->ID << ",";
					}*/
					objectHistoryFile << endl;
				}
			}
			objectHistoryFile << endl;

			/*if((*ptrObj)->belongs_to) objectFile << "\nObject Belongs to " << (*ptrObj)->belongs_to->blob->ID <<endl;*/

		}
	}










	//close files
	if( glbl_var.saveFramesToHard && glbl_var.saveObjectProcessingFile )
	{

		glbl_var.BGmodel->ObjectProcessingFile << endl <<endl;
		glbl_var.BGmodel->ObjectProcessingFile.close();
	}

	if( glbl_var.saveFramesToHard && glbl_var.saveObjectFile  )
	{
		glbl_var.BGmodel->objectFile << endl <<endl;
		glbl_var.BGmodel->objectFile.close();
	}

	if( glbl_var.saveFramesToHard && glbl_var.saveObjectHistoryFiles  )
	{
		glbl_var.BGmodel->objectHistoryFile <<endl <<endl;
		glbl_var.BGmodel->objectHistoryFile.close();
	}




}

float  BG_codebook_model::calculatePrecentageOverlappingArea(CvBlob* reference, CvBlob* OtherBlob)
{
	int refx2 = reference->x + reference->w;
	int refy2 = reference->y + reference->h;
	int othx2 = OtherBlob->x + OtherBlob->w;
	int othy2 = OtherBlob->y + OtherBlob->h;

	int left = (reference->x < OtherBlob->x) ? reference->x : OtherBlob->x;
	int right = (refx2 > othx2) ? refx2 : othx2;
	int top = (reference->y < OtherBlob->y) ? reference->y : OtherBlob->y;
	int bottom = (refy2 > othy2) ? refy2 : othy2;

	if (right - left > reference->w + OtherBlob->w) return 0;
	if (bottom - top > reference->h + OtherBlob->h) return 0;


	int smallestX = (reference->x < OtherBlob->x) ? OtherBlob->x : reference->x;
	int smallestY = (reference->y < OtherBlob->y) ? OtherBlob->y : reference->y;
	int largestX = (refx2 > othx2) ? othx2 : refx2;
	int largestY = (refy2 > othy2) ? othy2: refy2;

	int refArea = reference->w*reference->h;
	float Overlappingarea = (largestY - smallestY)*(largestX - smallestX);
	return 100*Overlappingarea/refArea;
}

void  BG_codebook_model::feedback() //feeds back from tracker to pixel level
{
	//reset the stationary object matrix
	int limit = w*h;
	for (int i = 0; i < limit; i++)
		stationary_object[i] = false;

	list<ConnectedComponent*>::iterator ptrCC;
	list<TrackerObject*>::iterator ptrObj;
	list<TrackerObject*>::iterator ptrObj2;
	for(ptrCC= BGSubtractionBlobs.begin(); ptrCC != BGSubtractionBlobs.end(); ptrCC++) //for each CC
	{
		if ((*ptrCC)->ghost) continue;

		ptrObj = (*ptrCC)->objects.begin();
		int left = (*ptrCC)->blob->x;
		int right = left + (*ptrCC)->blob->w;
		int top = (*ptrCC)->blob->y;
		int bottom = top + (*ptrCC)->blob->h;
		bool found;
		for (int cnt_y = top; cnt_y < bottom; cnt_y++) //for all the pixels inside the bounding box of the CC
			for (int cnt_x = left; cnt_x < right; cnt_x++)
			{
				int i  =cnt_x + w*cnt_y;
				found = false;
				int CClabel = glbl_var.BGlabels[i];
				if ((*ptrCC)->blob->ID == CClabel) //if the pixel actually belongs to the object
				{
					if (t - (*ptrObj)->time > OBJ_CONFIDENCE_TH) //if the object has been there for a long time
						//if((*ptrObj)->object_classification != UNKNOWN)
						found = true;
					else
					{

						for (ptrObj2 = (*ptrObj)->occlusion_list.begin();ptrObj2 != (*ptrObj)->occlusion_list.end();ptrObj2++) //or if any of the occlusion children has been there for a long time
						{
							if (t - (*ptrObj2)->time > OBJ_CONFIDENCE_TH)
								//if((*ptrObj2)->object_classification != UNKNOWN)
							{
								found = true;
								break;
							}
						}
					}
				}


				if (found ) //if found mark as foreground and delete the corresponding codeword
				{
					if (original_BG->imageData[i])
					{
						stationary_object[i] = true;
						glbl_var.BGmask->imageData[i] = 255;
						codebook* CBptr = CB[i];
						for (int j=0; j<num_of_active_models_per_pixel[i]; j++)
						{
							if (CBptr->getlast_access() == t-1)
							{
								deleteCW(CB[i],j);
								num_of_active_models_per_pixel[i]--;
								if (num_of_active_models_per_pixel[i] == 0)
									CB[i] = force_getCodebook(cnt_x,cnt_y,0);
								break;
							}
							CBptr = CBptr->get_next_CW();
						}
					}
				}
			}
	}


}
