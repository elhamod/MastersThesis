#include "g_variables.h"
#include "BGmodel.h"
#include "Tracker.h"
#include <string>
#include <fstream>
#include <windows.h>
#include <time.h>
#include "ReadFrames.h"
#include <direct.h>
#include "homomorphic.h"
#pragma once

namespace Version3withInterface {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;
	using namespace System::Runtime::InteropServices;
	using namespace System::Threading;
	using namespace std;
	using std::ofstream;
	using std::ifstream;

	/// <summary>
	/// Summary for Form1
	///
	/// WARNING: If you change the name of this class, you will need to change the
	///          'Resource File Name' property for the managed resource compiler tool
	///          associated with all .resx files this class depends on.  Otherwise,
	///          the designers will not be able to interact properly with localized
	///          resources associated with this form.
	/// </summary>






	void mouseHandler(int event, int x, int y, int flags, void* param) 	 /*mouse handler for clicking the video and getting the BG model*/
	{

		//Only works when a video is paused
		if (!glbl_var.video_paused) return;
		if (glbl_var.resize_factor != 1)
		{
			x = x/glbl_var.resize_factor;
			y = y/glbl_var.resize_factor;
		}

		//since the image might be concatenated, the if x is big, subtract w
		if (x >= glbl_var.BGmodel->get_w()) x -= glbl_var.BGmodel->get_w();

		if (event == CV_EVENT_LBUTTONDOWN)
		{
			//prepare Lab frame
			//if (glbl_var.BGmodel->get_used_method() == LabSpherical || glbl_var.BGmodel->get_used_method() == LabCylindrical) //Lab
			//{
			if (glbl_var.LabFrame)
			{
				cvReleaseImage(&glbl_var.LabFrame);
				glbl_var.LabFrame = 0;
			}
			glbl_var.LabFrame = cvCreateImage(cvSize(glbl_var.BGmodel->get_w()
				,glbl_var.BGmodel->get_h())
				,glbl_var.BGmodel->get_depth()
				,glbl_var.BGmodel->get_nCh());
			//cvCvtColor(glbl_var.currentFrame,glbl_var.LabFrame,CV_BGR2Lab);
			cvCopy(glbl_var.currentFrame,glbl_var.LabFrame);
			/*}*/

			glbl_var.codebook_lbls_changed = true;

			glbl_var.mh_prm.pxlNum_x = x;
			glbl_var.mh_prm.pxlNum_y = y;
			glbl_var.DisplayedCB  = 0;
			codebook* selectedCB = glbl_var.BGmodel->getCodebook(x,y,glbl_var.DisplayedCB);
			int indx = y*glbl_var.BGmodel->get_widthStep() + glbl_var.BGmodel->get_nCh()*x;

			switch(glbl_var.BGmodel->get_used_method())
			{
			case KimOriginal: //RGB
				glbl_var.mh_prm.R = (unsigned char)glbl_var.currentFrame->imageData[indx];
				glbl_var.mh_prm.G = (unsigned char)glbl_var.currentFrame->imageData[indx+1];
				glbl_var.mh_prm.B = (unsigned char)glbl_var.currentFrame->imageData[indx+2];
				break;
			case LabSpherical:
			case LabCylindrical: //Lab
				CvScalar cvs = cvScalar((unsigned char) glbl_var.LabFrame->imageData[indx]
				,(unsigned char) glbl_var.LabFrame->imageData[indx+1]
				,(unsigned char) glbl_var.LabFrame->imageData[indx+2]);
				cvs = glbl_var.display2real_Lab(cvs);
				glbl_var.mh_prm.L =  cvs.val[0];
				glbl_var.mh_prm.a =  cvs.val[1];
				glbl_var.mh_prm.b =  cvs.val[2];
				/*glbl_var.mh_prm.L = (unsigned char)glbl_var.currentFrame->imageData[indx];
				glbl_var.mh_prm.a = (unsigned char)glbl_var.currentFrame->imageData[indx+1];
				glbl_var.mh_prm.b = (unsigned char)glbl_var.currentFrame->imageData[indx+2]*/;
				break;
			}

			if (selectedCB != NULL)
			{
				glbl_var.mh_prm.MNRL = selectedCB->getMNRL();
				glbl_var.mh_prm.freq = selectedCB->getCW_freq();
				glbl_var.mh_prm.lastacc = selectedCB->getlast_access();
				glbl_var.mh_prm.firstacc = selectedCB->getfirst_access();
				glbl_var.mh_prm.codebookNumber = glbl_var.DisplayedCB;
				glbl_var.mh_prm.maxCB = (glbl_var.BGmodel->getNum_of_active_models_per_pixel())[y*glbl_var.BGmodel->get_w() + x];
				if (glbl_var.NFPP_checked)
					glbl_var.mh_prm.region_num = glbl_var.BGlabels[y*glbl_var.BGmodel->get_w() + x];
				//find the area of the region
				if (glbl_var.NFPP2_checked)
					glbl_var.mh_prm.region_area = glbl_var.BGmodel->connectivityArray_getArea(abs(glbl_var.mh_prm.region_num));


				switch(glbl_var.BGmodel->get_used_method())
				{
				case KimOriginal: //RGB
					glbl_var.mh_prm.Imax = selectedCB->getImax();
					glbl_var.mh_prm.Imin = selectedCB->getImin();
					glbl_var.mh_prm.R_CB = selectedCB->getRGB().val[0];
					glbl_var.mh_prm.G_CB = selectedCB->getRGB().val[1];
					glbl_var.mh_prm.B_CB = selectedCB->getRGB().val[2];
					break;
				case LabSpherical:
				case LabCylindrical: //Lab
					glbl_var.mh_prm.L_CB = selectedCB->getLab().val[0];
					glbl_var.mh_prm.a_CB = selectedCB->getLab().val[1];
					glbl_var.mh_prm.b_CB = selectedCB->getLab().val[2];
					break;
				}
			}
			else
			{
				//negative values indicate it doesn't exist

				glbl_var.mh_prm.MNRL = -1;
				glbl_var.mh_prm.freq = -1;
				glbl_var.mh_prm.lastacc = -1;
				glbl_var.mh_prm.firstacc = -1;
				glbl_var.mh_prm.codebookNumber = -1;
				glbl_var.mh_prm.maxCB = 0;
				glbl_var.mh_prm.region_num = 0;
				glbl_var.mh_prm.region_area = -1;


				switch(glbl_var.BGmodel->get_used_method())
				{
				case KimOriginal: //RGB
					glbl_var.mh_prm.Imax = -1;
					glbl_var.mh_prm.Imin = -1;
					glbl_var.mh_prm.R_CB = -1;
					glbl_var.mh_prm.G_CB = -1;
					glbl_var.mh_prm.B_CB = -1;
					break;
				case LabSpherical:
					glbl_var.mh_prm.L_CB = -1;
					glbl_var.mh_prm.a_CB = -1;
					glbl_var.mh_prm.b_CB = -1;
					break;
				}

			}

		}



	}
	void onTrackbarSlide(int pos) 
	{

		//if (100*cvGetCaptureProperty(glbl_var.videoStream,CV_CAP_PROP_POS_FRAMES)/ cvGetCaptureProperty(glbl_var.videoStream,CV_CAP_PROP_FRAME_COUNT) != pos)
		//cvSetCaptureProperty(glbl_var.videoStream,CV_CAP_PROP_POS_FRAMES,cvFloor(0.01*pos* cvGetCaptureProperty(glbl_var.videoStream,CV_CAP_PROP_FRAME_COUNT)));
	}










	public ref class Form1 : public System::Windows::Forms::Form
	{
	public:
		Form1(void)
		{

			InitializeComponent();
			//
			//TODO: Add the constructor code here
			//
		}

		//reset Form Controls when video is finished (handle both cases of multithreading and normal)

		void selectSource_grp_SetEnable(bool value)
		{
			// InvokeRequired required compares the thread ID of the
			// calling thread to the thread ID of the creating thread.
			// If these threads are different, it returns true.
			if (selectSource_grp->InvokeRequired)
			{
				SetStateDelegate^ d = 
					gcnew SetStateDelegate(this, &Form1::selectSource_grp_SetEnable);
				Invoke(d, gcnew array<Object^> { value });
			}
			else
			{
				selectSource_grp->Enabled = value;
			}
		}

		void whileRunningControls_grp_SetEnable(bool value)
		{
			// InvokeRequired required compares the thread ID of the
			// calling thread to the thread ID of the creating thread.
			// If these threads are different, it returns true.
			if (whileRunningControls_grp->InvokeRequired)
			{
				SetStateDelegate^ d = 
					gcnew SetStateDelegate(this, &Form1::whileRunningControls_grp_SetEnable);
				Invoke(d, gcnew array<Object^> { value });
			}
			else
			{
				whileRunningControls_grp->Enabled = value;
			}
		}

		void param_grp_SetEnable(bool value)
		{
			// InvokeRequired required compares the thread ID of the
			// calling thread to the thread ID of the creating thread.
			// If these threads are different, it returns true.
			if (param_grp->InvokeRequired)
			{
				SetStateDelegate^ d = 
					gcnew SetStateDelegate(this, &Form1::param_grp_SetEnable);
				Invoke(d, gcnew array<Object^> { value });
			}
			else
			{
				param_grp->Enabled = value;
			}
		}

		void mainAlgorithm_grp_SetEnable(bool value)
		{
			// InvokeRequired required compares the thread ID of the
			// calling thread to the thread ID of the creating thread.
			// If these threads are different, it returns true.
			if (mainAlgorithm_grp->InvokeRequired)
			{
				SetStateDelegate^ d = 
					gcnew SetStateDelegate(this, &Form1::mainAlgorithm_grp_SetEnable);
				Invoke(d, gcnew array<Object^> { value });
			}
			else
			{
				mainAlgorithm_grp->Enabled = value;
			}
		}

		void activatedOptions_grp_SetEnable(bool value)
		{
			// InvokeRequired required compares the thread ID of the
			// calling thread to the thread ID of the creating thread.
			// If these threads are different, it returns true.
			if (activatedOptions_grp->InvokeRequired)
			{
				SetStateDelegate^ d = 
					gcnew SetStateDelegate(this, &Form1::activatedOptions_grp_SetEnable);
				Invoke(d, gcnew array<Object^> { value });
			}
			else
			{
				activatedOptions_grp->Enabled = value;
			}
		}

		void processingtime_grp_SetEnable(bool value)
		{
			// InvokeRequired required compares the thread ID of the
			// calling thread to the thread ID of the creating thread.
			// If these threads are different, it returns true.
			if (processingtime_grp->InvokeRequired)
			{
				SetStateDelegate^ d = 
					gcnew SetStateDelegate(this, &Form1::processingtime_grp_SetEnable);
				Invoke(d, gcnew array<Object^> { value });
			}
			else
			{
				processingtime_grp->Enabled = value;
			}
		}



		void Codebook_grp_SetEnable(bool value)
		{
			// InvokeRequired required compares the thread ID of the
			// calling thread to the thread ID of the creating thread.
			// If these threads are different, it returns true.
			if (Codebook_grp->InvokeRequired)
			{
				SetStateDelegate^ d = 
					gcnew SetStateDelegate(this, &Form1::Codebook_grp_SetEnable);
				Invoke(d, gcnew array<Object^> { value });
			}
			else
			{
				Codebook_grp->Enabled = value;
			}
		}

		void setMethod_btn_SetEnable(bool value)
		{
			// InvokeRequired required compares the thread ID of the
			// calling thread to the thread ID of the creating thread.
			// If these threads are different, it returns true.
			if (setMethod_btn->InvokeRequired)
			{
				SetStateDelegate^ d = 
					gcnew SetStateDelegate(this, &Form1::setMethod_btn_SetEnable);
				Invoke(d, gcnew array<Object^> { value });
			}
			else
			{
				setMethod_btn->Enabled = value;
			}
		}

		void LoadCB_btn_SetEnable(bool value)
		{
			// InvokeRequired required compares the thread ID of the
			// calling thread to the thread ID of the creating thread.
			// If these threads are different, it returns true.
			if (LoadCB_btn->InvokeRequired)
			{
				SetStateDelegate^ d = 
					gcnew SetStateDelegate(this, &Form1::LoadCB_btn_SetEnable);
				Invoke(d, gcnew array<Object^> { value });
			}
			else
			{
				LoadCB_btn->Enabled = value;
			}
		}

		void minArea_chk_SetEnable(bool value)
		{
			// InvokeRequired required compares the thread ID of the
			// calling thread to the thread ID of the creating thread.
			// If these threads are different, it returns true.
			if (minArea_chk->InvokeRequired)
			{
				SetStateDelegate^ d = 
					gcnew SetStateDelegate(this, &Form1::minArea_chk_SetEnable);
				Invoke(d, gcnew array<Object^> { value });
			}
			else
			{
				minArea_chk->Enabled = value;
			}
		}

		void trackaftertrainingonly_chk_SetEnable(bool value)
		{
			// InvokeRequired required compares the thread ID of the
			// calling thread to the thread ID of the creating thread.
			// If these threads are different, it returns true.
			if (trackaftertrainingonly_chk->InvokeRequired)
			{
				SetStateDelegate^ d = 
					gcnew SetStateDelegate(this, &Form1::trackaftertrainingonly_chk_SetEnable);
				Invoke(d, gcnew array<Object^> { value });
			}
			else
			{
				trackaftertrainingonly_chk->Enabled = value;
			}
		}


		void btn_setParam_SetEnable(bool value)
		{
			// InvokeRequired required compares the thread ID of the
			// calling thread to the thread ID of the creating thread.
			// If these threads are different, it returns true.
			if (btn_setParam->InvokeRequired)
			{
				SetStateDelegate^ d = 
					gcnew SetStateDelegate(this, &Form1::btn_setParam_SetEnable);
				Invoke(d, gcnew array<Object^> { value });
			}
			else
			{
				btn_setParam->Enabled = value;
			}
		}

		void saveCB_btn_SetEnable(bool value)
		{
			// InvokeRequired required compares the thread ID of the
			// calling thread to the thread ID of the creating thread.
			// If these threads are different, it returns true.
			if (saveCB_btn->InvokeRequired)
			{
				SetStateDelegate^ d = 
					gcnew SetStateDelegate(this, &Form1::saveCB_btn_SetEnable);
				Invoke(d, gcnew array<Object^> { value });
			}
			else
			{
				saveCB_btn->Enabled = value;
			}
		}

		void savecurrentFrame_btn_SetEnable(bool value)
		{
			// InvokeRequired required compares the thread ID of the
			// calling thread to the thread ID of the creating thread.
			// If these threads are different, it returns true.
			if (savecurrentFrame_btn->InvokeRequired)
			{
				SetStateDelegate^ d = 
					gcnew SetStateDelegate(this, &Form1::savecurrentFrame_btn_SetEnable);
				Invoke(d, gcnew array<Object^> { value });
			}
			else
			{
				savecurrentFrame_btn->Enabled = value;
			}
		}

		void pause_btn_SetEnable(bool value)
		{
			// InvokeRequired required compares the thread ID of the
			// calling thread to the thread ID of the creating thread.
			// If these threads are different, it returns true.
			if (pause_btn->InvokeRequired)
			{
				SetStateDelegate^ d = 
					gcnew SetStateDelegate(this, &Form1::pause_btn_SetEnable);
				Invoke(d, gcnew array<Object^> { value });
			}
			else
			{
				pause_btn->Enabled = value;
			}
		}

		void play_btn_SetEnable(bool value)
		{
			// InvokeRequired required compares the thread ID of the
			// calling thread to the thread ID of the creating thread.
			// If these threads are different, it returns true.
			if (play_btn->InvokeRequired)
			{
				SetStateDelegate^ d = 
					gcnew SetStateDelegate(this, &Form1::play_btn_SetEnable);
				Invoke(d, gcnew array<Object^> { value });
			}
			else
			{
				play_btn->Enabled = value;
			}
		}

		void btn_halt_SetEnable(bool value)
		{
			// InvokeRequired required compares the thread ID of the
			// calling thread to the thread ID of the creating thread.
			// If these threads are different, it returns true.
			if (btn_halt->InvokeRequired)
			{
				SetStateDelegate^ d = 
					gcnew SetStateDelegate(this, &Form1::btn_halt_SetEnable);
				Invoke(d, gcnew array<Object^> { value });
			}
			else
			{
				btn_halt->Enabled = value;
			}
		}

		void minAreaThreshold_txt_SetEnable(bool value)
		{
			// InvokeRequired required compares the thread ID of the
			// calling thread to the thread ID of the creating thread.
			// If these threads are different, it returns true.
			if (minAreaThreshold_txt->InvokeRequired)
			{
				SetStateDelegate^ d = 
					gcnew SetStateDelegate(this, &Form1::minAreaThreshold_txt_SetEnable);
				Invoke(d, gcnew array<Object^> { value });
			}
			else
			{
				minAreaThreshold_txt->Enabled = value;
			}
		}

		void maxAreaThreshold_txt_SetEnable(bool value)
		{
			// InvokeRequired required compares the thread ID of the
			// calling thread to the thread ID of the creating thread.
			// If these threads are different, it returns true.
			if (maxAreaThreshold_txt->InvokeRequired)
			{
				SetStateDelegate^ d = 
					gcnew SetStateDelegate(this, &Form1::maxAreaThreshold_txt_SetEnable);
				Invoke(d, gcnew array<Object^> { value });
			}
			else
			{
				maxAreaThreshold_txt->Enabled = value;
			}
		}

		void OriginalCB_radio_SetChecked(bool value)
		{
			// InvokeRequired required compares the thread ID of the
			// calling thread to the thread ID of the creating thread.
			// If these threads are different, it returns true.
			if (OriginalCB_radio->InvokeRequired)
			{
				SetStateDelegate^ d = 
					gcnew SetStateDelegate(this, &Form1::OriginalCB_radio_SetChecked);
				Invoke(d, gcnew array<Object^> { value });
			}
			else
			{
				OriginalCB_radio->Checked = value;
			}
		}

		void minArea_chk_SetChecked(bool value)
		{
			// InvokeRequired required compares the thread ID of the
			// calling thread to the thread ID of the creating thread.
			// If these threads are different, it returns true.
			if (minArea_chk->InvokeRequired)
			{
				SetStateDelegate^ d = 
					gcnew SetStateDelegate(this, &Form1::minArea_chk_SetChecked);
				Invoke(d, gcnew array<Object^> { value });
			}
			else
			{
				minArea_chk->Checked = value;
			}
		}

		void NFPP_chk_SetChecked(bool value)
		{
			// InvokeRequired required compares the thread ID of the
			// calling thread to the thread ID of the creating thread.
			// If these threads are different, it returns true.
			if (NFPP_chk->InvokeRequired)
			{
				SetStateDelegate^ d = 
					gcnew SetStateDelegate(this, &Form1::NFPP_chk_SetChecked);
				Invoke(d, gcnew array<Object^> { value });
			}
			else
			{
				NFPP_chk->Checked = value;
			}
		}

		void fmin_chk_SetChecked(bool value)
		{
			// InvokeRequired required compares the thread ID of the
			// calling thread to the thread ID of the creating thread.
			// If these threads are different, it returns true.
			if (fmin_chk->InvokeRequired)
			{
				SetStateDelegate^ d = 
					gcnew SetStateDelegate(this, &Form1::fmin_chk_SetChecked);
				Invoke(d, gcnew array<Object^> { value });
			}
			else
			{
				fmin_chk->Checked = value;
			}
		}

		void disableUpdate_chk_SetChecked(bool value)
		{
			// InvokeRequired required compares the thread ID of the
			// calling thread to the thread ID of the creating thread.
			// If these threads are different, it returns true.
			if (disableUpdate_chk->InvokeRequired)
			{
				SetStateDelegate^ d = 
					gcnew SetStateDelegate(this, &Form1::disableUpdate_chk_SetChecked);
				Invoke(d, gcnew array<Object^> { value });
			}
			else
			{
				disableUpdate_chk->Checked = value;
			}
		}

		void trackaftertrainingonly_chk_SetChecked(bool value)
		{
			// InvokeRequired required compares the thread ID of the
			// calling thread to the thread ID of the creating thread.
			// If these threads are different, it returns true.
			if (trackaftertrainingonly_chk->InvokeRequired)
			{
				SetStateDelegate^ d = 
					gcnew SetStateDelegate(this, &Form1::trackaftertrainingonly_chk_SetChecked);
				Invoke(d, gcnew array<Object^> { value });
			}
			else
			{
				trackaftertrainingonly_chk->Checked = value;
			}
		}

		void trackerEnables_chk_SetChecked(bool value)
		{
			// InvokeRequired required compares the thread ID of the
			// calling thread to the thread ID of the creating thread.
			// If these threads are different, it returns true.
			if (trackerEnables_chk->InvokeRequired)
			{
				SetStateDelegate^ d = 
					gcnew SetStateDelegate(this, &Form1::trackerEnables_chk_SetChecked);
				Invoke(d, gcnew array<Object^> { value });
			}
			else
			{
				trackerEnables_chk->Checked = value;
			}
		}



		void realtime_radio_SetChecked(bool value)
		{
			// InvokeRequired required compares the thread ID of the
			// calling thread to the thread ID of the creating thread.
			// If these threads are different, it returns true.
			if (realtime_radio->InvokeRequired)
			{
				SetStateDelegate^ d = 
					gcnew SetStateDelegate(this, &Form1::realtime_radio_SetChecked);
				Invoke(d, gcnew array<Object^> { value });
			}
			else
			{
				realtime_radio->Checked = value;
			}
		}

		void useMorphology_chk_SetChecked(bool value)
		{
			// InvokeRequired required compares the thread ID of the
			// calling thread to the thread ID of the creating thread.
			// If these threads are different, it returns true.
			if (useMorphology_chk->InvokeRequired)
			{
				SetStateDelegate^ d = 
					gcnew SetStateDelegate(this, &Form1::useMorphology_chk_SetChecked);
				Invoke(d, gcnew array<Object^> { value });
			}
			else
			{
				useMorphology_chk->Checked = value;
			}
		}
		void homomorphic_chk_SetEnabled(bool value)
		{
			// InvokeRequired required compares the thread ID of the
			// calling thread to the thread ID of the creating thread.
			// If these threads are different, it returns true.
			if (homomorphic_chk->InvokeRequired)
			{
				SetStateDelegate^ d = 
					gcnew SetStateDelegate(this, &Form1::homomorphic_chk_SetEnabled);
				Invoke(d, gcnew array<Object^> { value });
			}
			else
			{
				homomorphic_chk->Enabled = value;
			}
		}
		void homomorphic_chk_SetChecked(bool value)
		{
			// InvokeRequired required compares the thread ID of the
			// calling thread to the thread ID of the creating thread.
			// If these threads are different, it returns true.
			if (homomorphic_chk->InvokeRequired)
			{
				SetStateDelegate^ d = 
					gcnew SetStateDelegate(this, &Form1::homomorphic_chk_SetChecked);
				Invoke(d, gcnew array<Object^> { value });
			}
			else
			{
				homomorphic_chk->Checked = value;
			}
		}
		void smooth_chk_SetChecked(bool value)
		{
			// InvokeRequired required compares the thread ID of the
			// calling thread to the thread ID of the creating thread.
			// If these threads are different, it returns true.
			if (smooth_chk->InvokeRequired)
			{
				SetStateDelegate^ d = 
					gcnew SetStateDelegate(this, &Form1::smooth_chk_SetChecked);
				Invoke(d, gcnew array<Object^> { value });
			}
			else
			{
				smooth_chk->Checked = value;
			}
		}

		void debug_monitor_SetChecked(bool value)
		{
			// InvokeRequired required compares the thread ID of the
			// calling thread to the thread ID of the creating thread.
			// If these threads are different, it returns true.
			if (debug_monitor_chk->InvokeRequired)
			{
				SetStateDelegate^ d = 
					gcnew SetStateDelegate(this, &Form1::debug_monitor_SetChecked);
				Invoke(d, gcnew array<Object^> { value });
			}
			else
			{
				debug_monitor_chk->Checked = value;
			}
		}


		void desiredfps_txt_SetEnabled(bool value)
		{
			// InvokeRequired required compares the thread ID of the
			// calling thread to the thread ID of the creating thread.
			// If these threads are different, it returns true.
			if (desiredfps_txt->InvokeRequired)
			{
				SetStateDelegate^ d = 
					gcnew SetStateDelegate(this, &Form1::desiredfps_txt_SetEnabled);
				Invoke(d, gcnew array<Object^> { value });
			}
			else
			{
				desiredfps_txt->Enabled = value;
			}
		}

		void Output_grp_SetEnabled(bool value)
		{
			// InvokeRequired required compares the thread ID of the
			// calling thread to the thread ID of the creating thread.
			// If these threads are different, it returns true.
			if (Output_grp->InvokeRequired)
			{
				SetStateDelegate^ d = 
					gcnew SetStateDelegate(this, &Form1::Output_grp_SetEnabled);
				Invoke(d, gcnew array<Object^> { value });
			}
			else
			{
				Output_grp->Enabled = value;
			}
		}

		void seperateTrainTest_chk_SetChecked(bool value)
		{
			// InvokeRequired required compares the thread ID of the
			// calling thread to the thread ID of the creating thread.
			// If these threads are different, it returns true.
			if (traintest_chk->InvokeRequired)
			{
				SetStateDelegate^ d = 
					gcnew SetStateDelegate(this, &Form1::seperateTrainTest_chk_SetChecked);
				Invoke(d, gcnew array<Object^> { value });
			}
			else
			{
				traintest_chk->Checked = value;
			}
		}



		//void rsz_prcs_txt_SetEnabled(bool value)
		//{
		//	// InvokeRequired required compares the thread ID of the
		//	// calling thread to the thread ID of the creating thread.
		//	// If these threads are different, it returns true.
		//	if (rsz_prcs_txt->InvokeRequired)
		//	{
		//		SetStateDelegate^ d = 
		//			gcnew SetStateDelegate(this, &Form1::rsz_prcs_txt_SetEnabled);
		//		Invoke(d, gcnew array<Object^> { value });
		//	}
		//	else
		//	{
		//		rsz_prcs_txt->Enabled = value;
		//	}
		//}



		void resetControls()
		{
			selectSource_grp_SetEnable(true);
			whileRunningControls_grp_SetEnable(false);
			param_grp_SetEnable(false);
			mainAlgorithm_grp_SetEnable(false);
			activatedOptions_grp_SetEnable(false);
			processingtime_grp_SetEnable(false);
			Codebook_grp_SetEnable(false);
			setMethod_btn_SetEnable(true);
			LoadCB_btn_SetEnable(false);
			minArea_chk_SetEnable(false);
			minAreaThreshold_txt_SetEnable(false);
			trackaftertrainingonly_chk_SetEnable(false);
			btn_setParam_SetEnable(true);
			saveCB_btn_SetEnable(false);
			savecurrentFrame_btn_SetEnable(false);
			pause_btn_SetEnable(false);
			play_btn_SetEnable(true);
			btn_halt_SetEnable(false);
			minAreaThreshold_txt_SetEnable(false);
			OriginalCB_radio_SetChecked(true);
			NFPP_chk_SetChecked(false);
			minArea_chk_SetChecked(false);
			fmin_chk_SetChecked(true);
			disableUpdate_chk_SetChecked(false);
			trackaftertrainingonly_chk_SetChecked(false);
			trackerEnables_chk_SetChecked(false);
			realtime_radio_SetChecked(true);
			//rsz_prcs_txt_SetEnabled(true);
			useMorphology_chk_SetChecked(false);
			homomorphic_chk_SetEnabled(false);
			homomorphic_chk_SetChecked(false);
			smooth_chk_SetChecked(false);
			seperateTrainTest_chk_SetChecked(false);
			Output_grp_SetEnabled(true);
			debug_monitor_SetChecked(false);
			desiredfps_txt_SetEnabled(true);




			Imax_lbl_SetText("Imax = ");
			Imin_lbl_SetText("Imin = ");
			Ilow_lbl_SetText("Ilow = ");
			Ihigh_lbl_SetText("Ihigh = ");
			I_lbl_SetText("I = ");
			region_lbl_SetText("Region: ");
			area_lbl_SetText("Area: ");
			G_lbl_SetText("G = ");
			R_lbl_SetText("R = ");
			B_lbl_SetText("B = ");
			G_CB_lbl_SetText("G_CB = ");
			R_CB_lbl_SetText("R_CB = ");
			B_CB_lbl_SetText("B_CB = ");
			delta_lbl_SetText("delta = ");
			delta2_lbl_SetText("delta2 = ");
			epsilon_lbl_SetText("epsilon = ");
			epsilon2_lbl_SetText("epsilon2 = ");
			pxlStatus_lbl_SetText("Status: ");				
			pxlNum_lbl_SetText("Pixel Coordinates: ");
			lastacc_lbl_SetText("last acc = ");
			firstacc_lbl_SetText("1st acc = "); 
			maxCB_lbl_SetText("Max = 0");
			MNRL_lbl_SetText("f = ");
			freq_lbl_SetText("MNRL = ");
			codebookNumber_lbl_SetText("CB # 0");
			desiredfps_txt_SetText("1");

		}

		void ReadFrames()
		{
			//create folder according to areathreshold,speed, N, Th, DeltaL
			/*int dummytimer = (unsigned) time(NULL);
			srand(dummytimer);
			int dummyrand = rand();
			sprintf(glbl_var.foldername,"Area%d Speed%d N%d TH%d Rand%d",glbl_var.BGmodel->get_minArea(),glbl_var.BGmodel->get_N(),glbl_var.BGmodel->get_TM(),glbl_var.speedLimit,dummyrand);
			mkdir(glbl_var.foldername);*/

			//ticks per second to be used for time calculations
			LARGE_INTEGER tickspersec;
			QueryPerformanceFrequency(&tickspersec);

			//int desired_fps =glbl_var.OriginalVideoFrameRate;
			//int desired_fps = glbl_var.sec_to_frame(1);
			int desired_fps;
			if (glbl_var.realtimeProcessing && glbl_var.BGmodel->get_t() > 0)
				desired_fps = floor(glbl_var.numberOfReadFrame / glbl_var.AbsoluteTime);
			else
				desired_fps =  glbl_var.OriginalVideoFrameRate;

			if (desired_fps > 1000/glbl_var.speedLimit) 
				desired_fps = 1000/glbl_var.speedLimit; //speed desired by the user

			//resolve the case of reading from images
			//num_fps = glbl_var.speedLimit;


			while (!glbl_var.video_paused )
			{

				LARGE_INTEGER begin; QueryPerformanceCounter(&begin);



				if (glbl_var.videoReadingMethod == readFromVideo)
				{
					/*for (int k = 0; k <2; k++)
					{*/
					if (!cvGrabFrame(glbl_var.videoStream )) 
					{
						glbl_var.video_finished = 1; 
						break; //no more frames
					}

					if(glbl_var.realtimeProcessing) //real time
					{
						if (glbl_var.frameIsProcessed) //checking if processing module is ready to receive a new frame
						{
							//cvReleaseImage(&glbl_var.queryFrame);
							glbl_var.queryFrame = cvRetrieveFrame( glbl_var.videoStream );
							glbl_var.frameIsProcessed = false;
						}
					}
					else //frame by frame
					{


						//if (glbl_var.queryFrame) 
						//	cvReleaseImage(&glbl_var.queryFrame);
						glbl_var.queryFrame = cvRetrieveFrame( glbl_var.videoStream );
						if(glbl_var.skippedFramesCounter < glbl_var.numberOfFramesToSkip) //check number of frames to skip specified by the user
						{
							glbl_var.skippedFramesCounter++;
						}
						else
						{
							glbl_var.frameIsProcessed = false;
							glbl_var.skippedFramesCounter = 0;
						}
					} 


					//}
					/*	cvNamedWindow("Current");
					cvShowImage("Current",glbl_var.queryFrame);
					cvWaitKey(10);*/


				}
				else //read from images
				{
					/*	 char* pathTest = new char[150];
					readFrame(glbl_var.filename, glbl_var.currentFramePos, glbl_var.numberOfDigits, glbl_var.ImageReadingExtension, pathTest);
					glbl_var.queryFrame =  cvLoadImage(pathTest);
					delete[] pathTest;*/
					//glbl_var.queryFrame = readFrame(glbl_var.filename, glbl_var.currentFramePos, glbl_var.numberOfDigits, glbl_var.ImageReadingExtension);


					if(glbl_var.realtimeProcessing) //real time
					{
						if (glbl_var.frameIsProcessed) //checking if processing module is ready to receive a new frame
						{
							readFrame(glbl_var.filename, glbl_var.currentFramePos, glbl_var.numberOfDigits, glbl_var.ImageReadingExtension, glbl_var.queryFrame);
							glbl_var.frameIsProcessed = false;
						}
					}
					else //frame by frame
					{

						readFrame(glbl_var.filename, glbl_var.currentFramePos, glbl_var.numberOfDigits, glbl_var.ImageReadingExtension, glbl_var.queryFrame);
						if(glbl_var.skippedFramesCounter < glbl_var.numberOfFramesToSkip) //check number of frames to skip specified by the user
						{
							glbl_var.skippedFramesCounter++;
						}
						else
						{
							glbl_var.frameIsProcessed = false;
							glbl_var.skippedFramesCounter = 0;
						}
					}



					glbl_var.currentFramePos++;
					if (!glbl_var.queryFrame) 
					{
						glbl_var.video_finished = 1; 
						break; //no more frames
					}

				}

				LARGE_INTEGER end; QueryPerformanceCounter(&end);

				LONGLONG dif = ((end.QuadPart - begin.QuadPart)*1000/tickspersec.QuadPart);
				cvSetTrackbarPos("Position","video_window",glbl_var.currentFramePos);

				glbl_var.numberOfReadFrame++;

				//if video finished by Halt button
				if (glbl_var.video_finished) break;

				//add to total time
				glbl_var.AbsoluteTime += 1/glbl_var.OriginalVideoFrameRate;
				videoTotalTime_lbl_SetText(cvFloor(glbl_var.AbsoluteTime) + " seconds" );

				//convert from seconds to frames if necessary (just in case of real time + in terms of time)
				if (glbl_var.timeOrFrames == IN_TERMS_OF_SECS && glbl_var.realtimeProcessing && glbl_var.BGmodel->get_t() > 0)
				{
					glbl_var.BGmodel->set_N(glbl_var.sec_to_frame(Single::Parse(N_txt->Text),true));
					glbl_var.BGmodel->set_TM(glbl_var.sec_to_frame(Single::Parse(TM_txt->Text),true));
					glbl_var.BGmodel->set_fmin(glbl_var.sec_to_frame(Single::Parse(fmin_txt->Text),true));

				}

				//thresholds definition
				glbl_var.BGmodel->MAX_ABSENCE = glbl_var.sec_to_frame(6,true);//2006: 6 second 
				glbl_var.BGmodel->PERSISTENCE_THRESHOLD = glbl_var.sec_to_frame(1,true);//1 second, in general : 1 second,  3 occlusion :glbl_var.sec_to_frame(0.5,true) //object persistence
				glbl_var.BehaviourDescriptor.LOITERING_THRESHOLD = glbl_var.sec_to_frame(20,true); // 2006: 20 seconds. Behave: 60
				glbl_var.BGmodel->OBJ_CONFIDENCE_TH = glbl_var.sec_to_frame(1,true); //3 seconds // general : 1 second //used for feedback
				glbl_var.BehaviourDescriptor.ABANDONED_LUGGAGE_DURATION_THRESHOLD= glbl_var.sec_to_frame(4,true);  //5 seconds for 2004
				glbl_var.BehaviourDescriptor. FAINT_DURATION_THRESHOLD= glbl_var.sec_to_frame(2,true);  // 3seconds
				glbl_var.BehaviourDescriptor. FIGHTING_TIME_SPAN= glbl_var.sec_to_frame(7,true); //7 seconds2004, 3 sec 2006
				glbl_var.BehaviourDescriptor. WALKING_TOGETHER_TIME_THRESHOLD= glbl_var.sec_to_frame(0.5,true);  //3 seconds
				glbl_var.BehaviourDescriptor.MEETING_DURATION_THRESHOLD = glbl_var.sec_to_frame(0.5,true); //3 seconds





				int added_time;
				//if (1000/num_fps > glbl_var.speedLimit)
				added_time = 1000/desired_fps - dif;
				//else
				//added_time = glbl_var.speedLimit - dif;


				if (glbl_var.realtimeProcessing)
				{
					if (added_time > 0 )
						Thread::Sleep(added_time);
				}
				else //if not real time just break out of the while
				{
					if (added_time > 0 )
						cvWaitKey(added_time);
					break;
				}



			}

			//remove all windows in case the video is finished
			if (glbl_var.video_finished)
			{
				//destroy capture and windows
				cvReleaseCapture( &glbl_var.videoStream );
				cvDestroyWindow( "video_window" );
				cvDestroyWindow( "map_window" );
				cvDestroyWindow( "Detailed_BG_window" );

				play_btn_SetState(false);
				pause_btn_SetState(false);

				if(glbl_var.saveFramesToHard && glbl_var.saveTimes)
					glbl_var.CPUfile.close();
				resetControls();

			}

			//cvReleaseImage(&glbl_var.queryFrame);

		}

		void ProcessFrame()
		{

			//ticks per second to be used for time calculations
			LARGE_INTEGER tickspersec;
			QueryPerformanceFrequency(&tickspersec);

			while (!glbl_var.video_paused && !glbl_var.video_finished )
			{
				//if (!glbl_var.queryFrame) continue; //if real time processing, wait until there actually is a read frame
				if (glbl_var.frameIsProcessed) goto end_label;



				//glbl_var.copiedQueryFrame = cvCreateImage(cvGetSize(glbl_var.queryFrame),8,3);
				//cvCopyImage(glbl_var.queryFrame,glbl_var.copiedQueryFrame);
				glbl_var.copiedQueryFrame = cvCreateImage(cvSize(glbl_var.BGmodel->get_w(),glbl_var.BGmodel->get_h()),8,3);


				cvResize(glbl_var.queryFrame,glbl_var.copiedQueryFrame);
				//int start_while_time = QueryPerformanceCounter();
				LARGE_INTEGER start_while_time;

				QueryPerformanceCounter(&start_while_time);


				//resize the current frame for processing if must
				LARGE_INTEGER start_resize_time; QueryPerformanceCounter(&start_resize_time);
				if (glbl_var.resize_processing_factor ==true)
					glbl_var.queryframe2currentframe();
				else
				{
					cvReleaseImage(&glbl_var.currentFrame);
					glbl_var.currentFrame = cvCreateImage(cvSize(glbl_var.BGmodel->get_w(),glbl_var.BGmodel->get_h()), glbl_var.BGmodel->get_depth(),glbl_var.BGmodel->get_nCh());
					cvCopy(glbl_var.queryFrame,glbl_var.currentFrame);
				}
				LARGE_INTEGER end_resize_time; QueryPerformanceCounter(&end_resize_time);
				LONGLONG resize_dif = ((end_resize_time.QuadPart - start_resize_time.QuadPart)*1000/tickspersec.QuadPart);
				Resizetimelbl_SetText("Frame resize time = " + resize_dif + " ms");


				////Smooth frame
				//if(glbl_var.GaussianSmooth)
				//	cvSmooth(glbl_var.currentFrame,glbl_var.currentFrame,2,0,0,1,1);


				//resize and show the current frame (for display only)
				if(!glbl_var.displayedFrame && (glbl_var.resize_factor!= 1))
				{
					glbl_var.BGmodel->set_displayed_dimensions(cvFloor(glbl_var.BGmodel->get_w()*glbl_var.resize_factor),
						cvFloor(glbl_var.BGmodel->get_h()*glbl_var.resize_factor));
					glbl_var.displayedFrame = cvCreateImage(cvSize(glbl_var.BGmodel->get_displayed_weight(),
						glbl_var.BGmodel->get_displayed_height()),
						glbl_var.BGmodel->get_depth(),
						glbl_var.BGmodel->get_nCh());
				}

				//save frame to hard before conversion
				if(glbl_var.saveFramesToHard && glbl_var.saveOriginalFrame )
				{
					const int temp_num = 1;
					char* fileNames[temp_num];
					char buffer[100];
					for (int i=0; i < temp_num ; i++)
					{
						fileNames[i] = new char[100];
						strcpy(fileNames[i],glbl_var.saveFramesTo);
					}
				


				


				

					strcat(fileNames[0],"frame");

					for (int i=0; i < temp_num ; i++)
					{
						strcat(fileNames[i],itoa(glbl_var.BGmodel->get_t(),buffer,10));
						strcat(fileNames[i],".png");
					}



					cvSaveImage(fileNames[0],glbl_var.currentFrame);
					for (int i=0; i < temp_num ; i++)
						delete[] fileNames[i];

				}




				//processing starts here
				LONGLONG conversion_dif = 0;
				LONGLONG homomorph_dif = 0;
				if (!glbl_var.OriginalCurrentFrameBeforeConvert)
					glbl_var.OriginalCurrentFrameBeforeConvert = cvCreateImage(cvSize(glbl_var.currentFrame->width,glbl_var.currentFrame->height),glbl_var.currentFrame->depth,glbl_var.currentFrame->nChannels);
				cvCopy(glbl_var.currentFrame,glbl_var.OriginalCurrentFrameBeforeConvert);

				//Smooth frame
				if(glbl_var.GaussianSmooth)
					cvSmooth(glbl_var.currentFrame,glbl_var.currentFrame,2,0,0,1,1);

				switch (glbl_var.BGmodel->get_used_method())
				{
				case LabSpherical:
				case LabCylindrical:
					//decide whether to convert from table or from equations
					//Conver sRGB to Linear RGB

					LARGE_INTEGER start_conversion_time; QueryPerformanceCounter(&start_conversion_time);

					glbl_var.convertCurrentFrameRGB2sRGB();
					cvCvtColor(glbl_var.currentFrame,glbl_var.currentFrame,CV_BGR2Lab);
					//glbl_var.equalizeHistLabFrame();
					
					//NEW ( add 5 to Lightness)
					/*for(int i = 0; i < glbl_var.currentFrame->width; i++)
						for(int j = 0; j < glbl_var.currentFrame->height; j++)
						{
							int tempInt = (int) (unsigned char) glbl_var.currentFrame->imageData[i*glbl_var.currentFrame->nChannels + j*glbl_var.currentFrame->widthStep];
							tempInt+=20;
							glbl_var.currentFrame->imageData[i*glbl_var.currentFrame->nChannels + j*glbl_var.currentFrame->widthStep] = (unsigned char) (int) tempInt;
						}*/

					LARGE_INTEGER end_conversion_time; QueryPerformanceCounter(&end_conversion_time);
					conversion_dif = ((end_conversion_time.QuadPart - start_conversion_time.QuadPart)*1000/tickspersec.QuadPart);
					LabConversionTimelbl_SetText("Conversion time = " + conversion_dif + " ms");

					//homomorphic filter (works only for Lab)
					if (glbl_var.useHomomorphic)
					{
						LARGE_INTEGER start_homomorph_time; QueryPerformanceCounter(&start_homomorph_time);

						glbl_var.homomorphicFiltering();

						LARGE_INTEGER end_homomorph_time; QueryPerformanceCounter(&end_homomorph_time);
						homomorph_dif = ((end_homomorph_time.QuadPart - start_homomorph_time.QuadPart)*1000/tickspersec.QuadPart);
						HomomorphicTimelbl_SetText("Homomorphic time = " + homomorph_dif + " ms");

					}
					//end of homomorphic filter


					break;
				}





				//calculate the epsilons for RGB method
				LONGLONG median_dif = 0;
				if (glbl_var.BGmodel->get_used_method() == KimOriginal)
				{
					if (glbl_var.previousFrame != NULL)
					{
						LARGE_INTEGER start_median_time; QueryPerformanceCounter(&start_median_time);
						glbl_var.BGmodel->update_medians();
						if (glbl_var.BGmodel->get_t() > 1) glbl_var.BGmodel->calc_epsilons();
						LARGE_INTEGER end_median_time; QueryPerformanceCounter(&end_median_time);
						median_dif = ((end_median_time.QuadPart - start_median_time.QuadPart)*1000/tickspersec.QuadPart);
						mdntime_lbl_SetText("median processing time = " + median_dif + " ms");

					}
				}
				if (glbl_var.previousFrame == NULL) //or initialize the previous frame if it's first time
				{
					glbl_var.previousFrame = cvCreateImage(cvSize( glbl_var.BGmodel->get_w(), glbl_var.BGmodel->get_h() ),
						glbl_var.BGmodel->get_depth(), 
						glbl_var.BGmodel->get_nCh() );
				}

				//update the model (training period)
				LARGE_INTEGER start_update_time; QueryPerformanceCounter(&start_update_time);
				glbl_var.BGmodel->update_model(glbl_var.currentFrame);
				LARGE_INTEGER end_update_time; QueryPerformanceCounter(&end_update_time);
				LONGLONG update_dif = ((end_update_time.QuadPart - start_update_time.QuadPart)*1000/tickspersec.QuadPart);
				updatetime_lbl_SetText("update processing time = " + update_dif + " ms");

				//do background subtraction (testing period) (in case of seperate train/test stages
				LARGE_INTEGER start_BG_time; QueryPerformanceCounter(&start_BG_time);
				glbl_var.BGmodel->BG_subtraction(glbl_var.currentFrame);
				LARGE_INTEGER end_BG_time; QueryPerformanceCounter(&end_BG_time);
				LONGLONG BG_dif = ((end_BG_time.QuadPart - start_BG_time.QuadPart)*1000/tickspersec.QuadPart);
				BGtime_lbl_SetText("BG subt processing time = " + BG_dif + " ms");

				/*	if (glbl_var.BGmodel->get_used_method() == LabSpherical || glbl_var.BGmodel->get_used_method() == LabCylindrical)
				cvCvtColor(glbl_var.currentFrame,glbl_var.currentFrame,CV_Lab2BGR);*/

				//mask unwanted areas out
				if(glbl_var.initialMaskFrame)
					cvCopy(glbl_var.blackFrame,glbl_var.BGmask,glbl_var.initialMask);


				//save original BG mask
				//stationary foreground: keep the old foreground
				/*int temph = glbl_var.BGmodel->get_h();
				int tempw = glbl_var.BGmodel->get_w();
				int cnt = 0;
				for (int j=0; j < temph; j++)
				for (int i=0; i < tempw; i++)
				{
				if (glbl_var.BGmask->imageData[cnt])
				glbl_var.BGmodel->original_BG->imageData[cnt] = 255;
				else
				glbl_var.BGmodel->original_BG->imageData[[cnt] = 0;
				cnt++;
				}*/
				if (glbl_var.BGmodel->original_BG) cvReleaseImage(&glbl_var.BGmodel->original_BG);
				glbl_var.BGmodel->original_BG = cvCloneImage(glbl_var.BGmask);


				//save frame to hard before connected component processing
				if(glbl_var.saveFramesToHard && glbl_var.savePostProcessing )
				{
					const int temp_num = 1;
					char* fileNames[temp_num];
					char buffer[100];
					for (int i=0; i < temp_num ; i++)
					{
						fileNames[i] = new char[100];
						strcpy(fileNames[i],glbl_var.saveFramesTo);
					}

					strcat(fileNames[0],"rawBGmask");

					for (int i=0; i < temp_num ; i++)
					{
						strcat(fileNames[i],itoa(glbl_var.BGmodel->get_t(),buffer,10));
						strcat(fileNames[i],".png");
					}

					cvSaveImage(fileNames[0],glbl_var.BGmask);


					//delete characters
					for (int i=0; i < temp_num ; i++)
						delete[] fileNames[i];					


				}

				//correct the BG mask by NFPP if activated
				LARGE_INTEGER start_PP2_time; QueryPerformanceCounter(&start_PP2_time);

				//save before postprocessing to hard drive
				if(glbl_var.saveFramesToHard && glbl_var.savePostProcessing )
				{
					const int temp_num = 1;
					char* fileNames[temp_num];
					char buffer[100];
					for (int i=0; i < temp_num ; i++)
					{
						fileNames[i] = new char[100];
						strcpy(fileNames[i],glbl_var.saveFramesTo);
					}

					strcat(fileNames[0],"postprocessing");

					for (int i=0; i < temp_num ; i++)
					{
						strcat(fileNames[i],itoa(glbl_var.BGmodel->get_t(),buffer,10));
						strcat(fileNames[i],"_stage-1");
						strcat(fileNames[i],".png");
					}

					cvSaveImage(fileNames[0],glbl_var.BGmask);


					//delete characters
					for (int i=0; i < temp_num ; i++)
						delete[] fileNames[i];					


				}


				if (NFPP_chk->Checked)
				{
					glbl_var.numOfRepeating = -1;
					do{
						glbl_var.numOfRepeating++;
						glbl_var.repeatLabeling = 0;
						int h = glbl_var.BGmodel->get_h();
						int w = glbl_var.BGmodel->get_w();

						//initialize BGlabels if not initialized yet
						if (glbl_var.BGlabels == NULL) 
							glbl_var.BGlabels = new int[w* h];

						//initialize BGlabels to all -1 (i.e. no label)
						int cnt =0;
						for (int cnt_y=0; cnt_y<h; cnt_y++)
							for (int cnt_x=0; cnt_x<w; cnt_x++)
							{

								glbl_var.BGlabels[cnt] = -1;
								cnt++;
							}

							//reset connectivity array
							glbl_var.BGmodel->resetConnectivityArray();

							glbl_var.BGmodel->NFPP_update_pass1(); //refer to "An Improved Real-time Blob Detection for Visual Surveillance, by Nguyen and Chung"
							if (minArea_chk->Checked)
								glbl_var.BGmodel->NFPP_update_pass2(); //refer to "An Improved Real-time Blob Detection for Visual Surveillance, by Nguyen and Chung"

							//save stages to hard drive
							if(glbl_var.saveFramesToHard && glbl_var.savePostProcessing )
							{
								const int temp_num = 1;
								char* fileNames[temp_num];
								char buffer[100];
								for (int i=0; i < temp_num ; i++)
								{
									fileNames[i] = new char[100];
									strcpy(fileNames[i],glbl_var.saveFramesTo);
								}

								strcat(fileNames[0],"postprocessing");

								for (int i=0; i < temp_num ; i++)
								{
									strcat(fileNames[i],itoa(glbl_var.BGmodel->get_t(),buffer,10));
									strcat(fileNames[i],"_stage");
									strcat(fileNames[i],itoa(glbl_var.numOfRepeating,buffer,10));
									strcat(fileNames[i],".png");
								}

								cvSaveImage(fileNames[0],glbl_var.BGmask);


								//delete characters
								for (int i=0; i < temp_num ; i++)
									delete[] fileNames[i];		

								int tempIndex = 0;
								IplImage* labeled = cvCloneImage(glbl_var.BGmask);
								for(int j=0; j < glbl_var.BGmodel->get_h(); j++)
									for(int i=0; i < glbl_var.BGmodel->get_w(); i++)
									{
										if(glbl_var.BGlabels[tempIndex] > 0)
											labeled->imageData[tempIndex] = (glbl_var.BGlabels[tempIndex]*10)%256;
										else
											labeled->imageData[tempIndex] = 0;
										tempIndex++;

									}
									char buffer2[100];
									char* tempName = new char[100];
									strcpy(tempName,glbl_var.saveFramesTo);
									strcat(tempName,"labelsframe");
									strcat(tempName,itoa(glbl_var.BGmodel->get_t(),buffer2,10));
									strcat(tempName,"_stage");
									strcat(tempName,itoa(glbl_var.numOfRepeating,buffer2,10));
									strcat(tempName,".png");
									cvSaveImage(tempName,labeled);
									cvReleaseImage(&labeled);
									delete[] tempName;


							}

					}while (glbl_var.repeatLabeling);

					glbl_var.numOfRepeating = 0;

				}
				LARGE_INTEGER end_PP2_time; QueryPerformanceCounter(&end_PP2_time);
				LONGLONG PP2_dif = ((end_PP2_time.QuadPart - start_PP2_time.QuadPart)*1000/tickspersec.QuadPart);
				PP2_lbl_SetText("Post Processing time = " + PP2_dif + " ms");


				/*LARGE_INTEGER stupid1; QueryPerformanceCounter(&stupid1);
				LARGE_INTEGER stupid2; QueryPerformanceCounter(&stupid2);
				LONGLONG stupid_dif = ((stupid2.QuadPart - stupid1.QuadPart)*1000/tickspersec.QuadPart);
				Stupid_lbl->Text = "Convert time = " + stupid_dif+ " ms";*/


				glbl_var.sampledAbsoluteTime = glbl_var.AbsoluteTime;

				//Blob Processing
				LONGLONG Tracker_dif = 0;
				if (glbl_var.trackerEnabled)
				{
					if (!glbl_var.trackerEnabledOnlyAfterTraining || (glbl_var.BGmodel->get_t() > glbl_var.BGmodel->get_N())) //in case tracking is only after training
					{

						LARGE_INTEGER start_Tracker_time; QueryPerformanceCounter(&start_Tracker_time);
						//glbl_var.BGmodel->estimateLocations();
						glbl_var.BGmodel->findBGSubtractionBlobs(glbl_var.BGlabels);
						glbl_var.BGmodel->correlateBlobs();
						glbl_var.BGmodel->feedback();

						LARGE_INTEGER end_Tracker_time; QueryPerformanceCounter(&end_Tracker_time);
						Tracker_dif = ((end_Tracker_time.QuadPart - start_Tracker_time.QuadPart)*1000/tickspersec.QuadPart);
						TrackerTime_lbl_SetText("Tracker time = " + Tracker_dif + " ms");
					}
				}

		

				//feedback from blob processing from previous frame to pixel level info in this frame
				//LONGLONG Feedback_dif = 0;
				////if (glbl_var.feedbackEnabled)
				////{
				////if (!glbl_var.trackerEnabledOnlyAfterTraining || (glbl_var.BGmodel->get_t() > glbl_var.BGmodel->get_N())) //in case tracking is only after training
				////{
				//LARGE_INTEGER start_Feedback_time; QueryPerformanceCounter(&start_Feedback_time);

				//glbl_var.BGmodel->feedback();

				//LARGE_INTEGER end_Feedback_time; QueryPerformanceCounter(&end_Feedback_time);
				//Feedback_dif = ((end_Feedback_time.QuadPart - start_Feedback_time.QuadPart)*1000/tickspersec.QuadPart);
				////TrackerTime_lbl_SetText("Feedback time = " + Feedback_dif + " ms");
				////}
				////}





				/*}*/

				/*if (glbl_var.resize_factor != 1)
				{
				int w2 = glbl_var.displayedFrame->width,h2 = glbl_var.displayedFrame->height;
				glbl_var.concatimage = cvCreateImage(cvSize(w2*2,h2),glbl_var.displayedFrame->depth,glbl_var.displayedFrame->nChannels);
				cvSetImageROI(glbl_var.concatimage,cvRect(0,0,w2,h2));
				cvResize(glbl_var.copiedQueryFrame,glbl_var.displayedFrame,CV_INTER_AREA );
				cvCopy(glbl_var.displayedFrame,glbl_var.concatimage);
				}
				else
				{
				int w = glbl_var.currentFrame->width, h = glbl_var.currentFrame->height;
				glbl_var.concatimage = cvCreateImage(cvSize(w*2,h),glbl_var.currentFrame->depth,glbl_var.currentFrame->nChannels);
				cvSetImageROI(glbl_var.concatimage,cvRect(0,0,w,h));
				cvCopy(glbl_var.copiedQueryFrame,glbl_var.concatimage);
				}*/



				LONGLONG Behaviour_dif = 0;
				if (glbl_var.behaviourEnabled)
				{
					LARGE_INTEGER start_Behaviour_time; QueryPerformanceCounter(&start_Behaviour_time);


					//only call it every TIME_MEASUREMENT_STEP
					double t1 = glbl_var.sampledAbsoluteTime;
					double t2 = glbl_var.BehaviourDescriptor.lastUpdateTime;

					if(glbl_var.saveFramesToHard && glbl_var.saveBehaviourFile)
					{
						char filename[100]; 
						strcpy(filename,glbl_var.saveFramesTo);
						strcat(filename,"behaviour.log");
						glbl_var.BehaviourDescriptor.behaviourFile.open(filename,ios::app);
						glbl_var.BehaviourDescriptor.behaviourFile << "-frame " << glbl_var.BGmodel->get_t() << endl;
						glbl_var.BehaviourDescriptor.behaviourFile << " newt " << t1 <<endl ;
						glbl_var.BehaviourDescriptor.behaviourFile << " oldt " << t2 <<endl ;
					}

					
					if (t2 == 0 || t1 - t2 >= TIME_MEASUREMENT_STEP)
					{
						if(glbl_var.saveFramesToHard && glbl_var.saveBehaviourFile)
							glbl_var.BehaviourDescriptor.behaviourFile << " time for new update ! " <<endl ;

						//update the matrix every time-step
						glbl_var.BehaviourDescriptor.updateMatrix();

						//detect all behaviours and write them down in a log
					}

					// if

					LARGE_INTEGER end_Behaviour_time; QueryPerformanceCounter(&end_Behaviour_time);
					Behaviour_dif = ((end_Behaviour_time.QuadPart - start_Behaviour_time.QuadPart)*1000/tickspersec.QuadPart);
					BehaviourTime_lbl_SetText("Behaviour time = " + Behaviour_dif + " ms");

				}


				//processing ends here

				LARGE_INTEGER start_display_time; QueryPerformanceCounter(&start_display_time);

				cvCopy(glbl_var.currentFrame,glbl_var.previousFrame);
				glbl_var.BGMaskToFrame(); 

				glbl_var.drawTrackerBlobs();
				System::String^ str_temp = gcnew System::String(glbl_var.behaviour_log);
				SingleObjBehaviour_txt_SetText(str_temp);


				if (glbl_var.saveFramesToHard && glbl_var.saveBehaviourFile) 
				{
					glbl_var.BehaviourDescriptor.behaviourFile << endl;
					glbl_var.BehaviourDescriptor.behaviourFile.close();
				}

				//1
				//show frames BG and video
				/*if (glbl_var.resize_factor != 1)
				{
				int w2 = glbl_var.displayedFrame->width,h2 = glbl_var.displayedFrame->height;					
				cvSetImageROI(glbl_var.concatimage,cvRect(w2,0,w2,h2));
				cvResize(glbl_var.BGframe,glbl_var.displayedFrame,CV_INTER_AREA );
				cvCopy(glbl_var.displayedFrame,glbl_var.concatimage); 
				}
				else
				{
				int w = glbl_var.currentFrame->width, h = glbl_var.currentFrame->height;
				cvSetImageROI(glbl_var.concatimage,cvRect(w,0,w,h));
				cvCopy(glbl_var.BGframe,glbl_var.concatimage); 

				}*/
				//cvResetImageROI(glbl_var.concatimage);
				//cvShowImage( "video_window",glbl_var.concatimage );
				//cvReleaseImage(&glbl_var.concatimage);
				//2
				/*int w2 = glbl_var.displayedFrame->width,h2 = glbl_var.displayedFrame->height;					
				if (glbl_var.debugScreen)
				cvResize(glbl_var.BGframe,glbl_var.displayedFrame,CV_INTER_AREA );
				else
				cvResize(glbl_var.copiedQueryFrame,glbl_var.displayedFrame,CV_INTER_AREA );*/
				cvShowImage( "video_window",glbl_var.displayedFrame );



				//cvWaitKey(1);

				//if the user requested a new pixel or codeword
				if (glbl_var.codebook_lbls_changed)
					update_CB_labels();

				frameNumber_lbl_SetText("frame number " + glbl_var.BGmodel->get_t());

				//save BGmask, annotated frame, and and BGframe to hard drive
				if(glbl_var.saveFramesToHard)
				{
					const int temp_num = 4;
					char* fileNames[temp_num];
					char buffer[100];
					for (int i=0; i < temp_num ; i++)
					{
						fileNames[i] = new char[100];
						strcpy(fileNames[i],glbl_var.saveFramesTo);
					}

					strcat(fileNames[0],"BG");
					strcat(fileNames[1],"BG_mask");
					strcat(fileNames[2],"annotated");

					for (int i=0; i < temp_num ; i++)
					{
						strcat(fileNames[i],itoa(glbl_var.BGmodel->get_t(),buffer,10));
						strcat(fileNames[i],".png");
					}

					if (glbl_var.saveBGFrame ) cvSaveImage(fileNames[0],glbl_var.BGframe);
					if (glbl_var.saveBGmask  )cvSaveImage(fileNames[1],glbl_var.BGmask);
					if (glbl_var.saveAnnotatedFrame  )cvSaveImage(fileNames[2],glbl_var.displayedFrame);

					//also save detected objects
					if(glbl_var.saveObjectImages )
					{
						char fileName[100]; //used to keep the original name as it will be sued for objects 
						char fileName2[100]; //used to keep the original name as it will be sued for objects 
						char fileName3[100]; //used to keep the original name as it will be sued for objects 
						strcpy(fileName,glbl_var.saveFramesTo);
						strcat(fileName,"blob");
						strcpy(fileName2,"frame");
						strcat(fileName2,itoa(glbl_var.BGmodel->get_t(),buffer,10));
						//list<ConnectedComponent*>::iterator ptrCC;
						list<TrackerObject*>::iterator ptrCC;
						int i=0;
						IplImage* concatenateObject; 
						IplImage* tempFrameBefore = cvCreateImage(cvSize(glbl_var.OriginalCurrentFrameBeforeConvert->width+glbl_var.BGmodel->offset_contour*2,glbl_var.OriginalCurrentFrameBeforeConvert->height+glbl_var.BGmodel->offset_contour*2),glbl_var.OriginalCurrentFrameBeforeConvert->depth,3);
						cvZero(tempFrameBefore);
						cvSetImageROI(tempFrameBefore,cvRect(glbl_var.BGmodel->offset_contour,glbl_var.BGmodel->offset_contour,glbl_var.BGmodel->get_w(),glbl_var.BGmodel->get_h()));
						cvCopyImage(glbl_var.OriginalCurrentFrameBeforeConvert,tempFrameBefore);
						cvResetImageROI(tempFrameBefore);
						for(ptrCC= glbl_var.BGmodel->trackerBlobs.begin(); ptrCC != glbl_var.BGmodel->trackerBlobs.end(); ptrCC++)
							//for(ptrCC= glbl_var.BGmodel->BGSubtractionBlobs.begin(); ptrCC != glbl_var.BGmodel->BGSubtractionBlobs.end(); ptrCC++)
						{
							if ((*ptrCC)->matched)
							{
								//concatenateObject = cvCreateImage(cvSize((*ptrCC)->blob->w*3,(*ptrCC)->blob->h),glbl_var.currentFrame->depth,glbl_var.currentFrame->nChannels);
								//concatenateObject = cvCreateImage(cvSize((*ptrCC)->CC->blob->w*3,(*ptrCC)->CC->blob->h),glbl_var.currentFrame->depth,glbl_var.currentFrame->nChannels);
								concatenateObject = cvCreateImage(cvSize((*ptrCC)->CC->blob_img->width*3,(*ptrCC)->CC->blob_img->height),glbl_var.currentFrame->depth,glbl_var.currentFrame->nChannels);
								//cvZero(concatenateObject);
								strcpy(fileNames[3],fileName);
								//strcat(fileNames[2],itoa(i,buffer,10));
								strcat(fileNames[3],itoa((*ptrCC)->blob->ID,buffer,10));
								strcat(fileNames[3],fileName2);
								strcat(fileNames[3],".png");
								//save the mask
								//cvSetImageROI(glbl_var.BGmask,cvRect((*ptrCC)->blob->x,(*ptrCC)->blob->y,(*ptrCC)->blob->w,(*ptrCC)->blob->h));
								//cvSetImageROI(glbl_var.BGmask,cvRect((*ptrCC)->CC->blob->x,(*ptrCC)->CC->blob->y,(*ptrCC)->CC->blob->w,(*ptrCC)->CC->blob->h));
								//cvSetImageROI(glbl_var.BGmodel->original_BG,cvRect((*ptrCC)->CC->blob->x,(*ptrCC)->CC->blob->y,(*ptrCC)->CC->blob->w,(*ptrCC)->CC->blob->h));
								//cvSetImageROI(concatenateObject,cvRect(0,0,(*ptrCC)->CC->blob->w,(*ptrCC)->CC->blob->h));
								cvSetImageROI(concatenateObject,cvRect(0,0,(*ptrCC)->CC->blob_img->width,(*ptrCC)->CC->blob_img->height));
								//cvMerge(glbl_var.BGmask,glbl_var.BGmask,glbl_var.BGmask,0,concatenateObject);
								//cvMerge(glbl_var.BGmodel->original_BG,glbl_var.BGmodel->original_BG,glbl_var.BGmodel->original_BG,0,concatenateObject);
								cvMerge((*ptrCC)->CC->blob_mask,(*ptrCC)->CC->blob_mask,(*ptrCC)->CC->blob_mask,0,concatenateObject);
								//save the object
								//cvSetImageROI(glbl_var.currentFrame,cvRect((*ptrCC)->blob->x,(*ptrCC)->blob->y,(*ptrCC)->blob->w,(*ptrCC)->blob->h));
								//cvSetImageROI(glbl_var.currentFrame,cvRect((*ptrCC)->CC->blob->x,(*ptrCC)->CC->blob->y,(*ptrCC)->CC->blob->w,(*ptrCC)->CC->blob->h));
								cvSetImageROI(concatenateObject,cvRect((*ptrCC)->CC->blob_img->width,0,(*ptrCC)->CC->blob_img->width,(*ptrCC)->CC->blob_img->height));
								cvCopy((*ptrCC)->CC->blob_img,concatenateObject);
								//cvCopy(glbl_var.currentFrame,concatenateObject,glbl_var.BGmask);
								//cvCopy(glbl_var.currentFrame,concatenateObject,(*ptrCC)->CC->blob_mask);

								/*cvSetImageROI(glbl_var.OriginalCurrentFrameBeforeConvert,cvRect((*ptrCC)->blob->x,(*ptrCC)->blob->y,(*ptrCC)->blob->w,(*ptrCC)->blob->h));*/
								cvSetImageROI(tempFrameBefore,cvRect((*ptrCC)->CC->blob->x,(*ptrCC)->CC->blob->y,(*ptrCC)->CC->blob_img->width,(*ptrCC)->CC->blob_img->height));
								//cvSetImageROI(concatenateObject,cvRect((*ptrCC)->CC->blob->w*2,0,(*ptrCC)->CC->blob->w,(*ptrCC)->CC->blob->h));
								cvSetImageROI(concatenateObject,cvRect((*ptrCC)->CC->blob_img->width*2,0,(*ptrCC)->CC->blob_img->width,(*ptrCC)->CC->blob_img->height));
								//cvCopy(glbl_var.OriginalCurrentFrameBeforeConvert,concatenateObject,glbl_var.BGmask);
								//cvSetImageROI((*ptrCC)->CC->blob_mask,cvRect((*ptrCC)->CC->blob->x,(*ptrCC)->CC->blob->y,(*ptrCC)->CC->blob->w,(*ptrCC)->CC->blob->h));
								cvCopyImage(tempFrameBefore,concatenateObject);


								cvResetImageROI(concatenateObject);
								cvSaveImage(fileNames[3],concatenateObject);
								//cvResetImageROI(glbl_var.BGmask);
								//cvResetImageROI(glbl_var.BGmodel->original_BG);
								//cvResetImageROI(glbl_var.currentFrame);
								//cvResetImageROI(glbl_var.OriginalCurrentFrameBeforeConvert);
								//cvResetImageROI((*ptrCC)->CC->blob_mask);
								cvReleaseImage(&concatenateObject);

							}

						}

						for (int i=0; i < temp_num ; i++)
							delete[] fileNames[i];
					}
				}

				LARGE_INTEGER end_display_time; QueryPerformanceCounter(&end_display_time);
				LONGLONG display_dif = ((end_display_time.QuadPart - start_display_time.QuadPart)*1000/tickspersec.QuadPart);
				DisplayTime_lbl_SetText("Display overhead = " + display_dif + " ms");




				//int end_while_time = QueryPerformanceCounter();
				LARGE_INTEGER end_while_time;
				QueryPerformanceCounter(&end_while_time);
				LONGLONG all_dif = ((end_while_time.QuadPart - start_while_time.QuadPart)*1000/tickspersec.QuadPart);
				totalTime_lbl_SetText("Total = " + all_dif + " ms");


				//write values to the file
				//if(glbl_var.saveFramesToHard)
				if(glbl_var.saveFramesToHard && glbl_var.saveTimes)
				{
					glbl_var.CPUfile << resize_dif <<"," << conversion_dif <<		 "," << homomorph_dif << ","	<< median_dif << "," << update_dif << ","		<< BG_dif << ","				<< PP2_dif<< ","			<< Tracker_dif<< ","			<< Behaviour_dif<< "," << display_dif<< "," << all_dif << "," <<		 glbl_var.BGmodel->calculateAverageCWsPerPxl() << "," << glbl_var.BGmodel->BGSubtractionBlobs.size() << "," << glbl_var.numberOfReadFrame << "," << glbl_var.BGmodel->get_t() << "," << glbl_var.AbsoluteTime << "," ;

					list<int>::iterator itr_int;
					
					for(itr_int = glbl_var.abandonedLuggaeID.begin(); itr_int != glbl_var.abandonedLuggaeID.end();itr_int++ )
						glbl_var.CPUfile << (*itr_int) << "_";
					glbl_var.CPUfile << ",";
					
					for(itr_int = glbl_var.theftLuggageID.begin(); itr_int != glbl_var.theftLuggageID.end();itr_int++ )
						glbl_var.CPUfile << (*itr_int) << "_";
					glbl_var.CPUfile << ",";
					
					for(itr_int = glbl_var.loiterID.begin(); itr_int != glbl_var.loiterID.end();itr_int++ )
						glbl_var.CPUfile << (*itr_int) << "_";
					glbl_var.CPUfile << ",";
					
					for(itr_int = glbl_var.fightID.begin(); itr_int != glbl_var.fightID.end();itr_int++ )
						glbl_var.CPUfile << (*itr_int) << "_";
					glbl_var.CPUfile << ",";
					
					for(itr_int = glbl_var.faintID.begin(); itr_int != glbl_var.faintID.end();itr_int++ )
						glbl_var.CPUfile << (*itr_int) << "_";
					glbl_var.CPUfile << ",";
					
					for(itr_int = glbl_var.meetID.begin(); itr_int != glbl_var.meetID.end();itr_int++ )
						glbl_var.CPUfile << (*itr_int) << "_";
					glbl_var.CPUfile << ",";
					
					for(itr_int = glbl_var.walkTogetherID.begin(); itr_int != glbl_var.walkTogetherID.end();itr_int++ )
						glbl_var.CPUfile << (*itr_int) << "_";
					glbl_var.CPUfile << endl;
				}






				//saveframes
				/*char tmp[100];
				char tmp2[100];
				strcpy(tmp,glbl_var.foldername);
				sprintf(tmp2,"\\frame%d.png",glbl_var.BGmodel->get_t());
				strcat(tmp, tmp2);
				cvSaveImage(tmp,glbl_var.copiedQueryFrame);*/

				glbl_var.frameIsProcessed = true;

end_label:
				if (!glbl_var.realtimeProcessing)
					break;

			}

			cvReleaseImage(&glbl_var.copiedQueryFrame);
		}

		//a set of functions to set properties of controls in the form while using multithreading
		delegate void SetTextDelegate(String^ text);
		delegate void SetStateDelegate(bool state);
		void totalTime_lbl_SetText(String^ text)
		{
			// InvokeRequired required compares the thread ID of the
			// calling thread to the thread ID of the creating thread.
			// If these threads are different, it returns true.
			if (totalTime_lbl->InvokeRequired)
			{
				SetTextDelegate^ d = 
					gcnew SetTextDelegate(this, &Form1::totalTime_lbl_SetText);
				Invoke(d, gcnew array<Object^> { text });
			}
			else
			{
				totalTime_lbl->Text = text;
			}
		}

		void videoTotalTime_lbl_SetText(String^ text)
		{
			// InvokeRequired required compares the thread ID of the
			// calling thread to the thread ID of the creating thread.
			// If these threads are different, it returns true.
			if (videoTotalTime_lbl->InvokeRequired)
			{
				SetTextDelegate^ d = 
					gcnew SetTextDelegate(this, &Form1::videoTotalTime_lbl_SetText);
				Invoke(d, gcnew array<Object^> { text });
			}
			else
			{
				videoTotalTime_lbl->Text = text;
			}
		}

		void Imax_lbl_SetText(String^ text)
		{
			// InvokeRequired required compares the thread ID of the
			// calling thread to the thread ID of the creating thread.
			// If these threads are different, it returns true.
			if (Imax_lbl->InvokeRequired)
			{
				SetTextDelegate^ d = 
					gcnew SetTextDelegate(this, &Form1::Imax_lbl_SetText);
				Invoke(d, gcnew array<Object^> { text });
			}
			else
			{
				Imax_lbl->Text = text;
			}
		}

		void Imin_lbl_SetText(String^ text)
		{
			// InvokeRequired required compares the thread ID of the
			// calling thread to the thread ID of the creating thread.
			// If these threads are different, it returns true.
			if (Imin_lbl->InvokeRequired)
			{
				SetTextDelegate^ d = 
					gcnew SetTextDelegate(this, &Form1::Imin_lbl_SetText);
				Invoke(d, gcnew array<Object^> { text });
			}
			else
			{
				Imin_lbl->Text = text;
			}
		}

		void Ilow_lbl_SetText(String^ text)
		{
			// InvokeRequired required compares the thread ID of the
			// calling thread to the thread ID of the creating thread.
			// If these threads are different, it returns true.
			if (Ilow_lbl->InvokeRequired)
			{
				SetTextDelegate^ d = 
					gcnew SetTextDelegate(this, &Form1::Ilow_lbl_SetText);
				Invoke(d, gcnew array<Object^> { text });
			}
			else
			{
				Ilow_lbl->Text = text;
			}
		}

		void Ihigh_lbl_SetText(String^ text)
		{
			// InvokeRequired required compares the thread ID of the
			// calling thread to the thread ID of the creating thread.
			// If these threads are different, it returns true.
			if (Ihigh_lbl->InvokeRequired)
			{
				SetTextDelegate^ d = 
					gcnew SetTextDelegate(this, &Form1::Ihigh_lbl_SetText);
				Invoke(d, gcnew array<Object^> { text });
			}
			else
			{
				Ihigh_lbl->Text = text;
			}
		}


		void I_lbl_SetText(String^ text)
		{
			// InvokeRequired required compares the thread ID of the
			// calling thread to the thread ID of the creating thread.
			// If these threads are different, it returns true.
			if (I_lbl->InvokeRequired)
			{
				SetTextDelegate^ d = 
					gcnew SetTextDelegate(this, &Form1::I_lbl_SetText);
				Invoke(d, gcnew array<Object^> { text });
			}
			else
			{
				I_lbl->Text = text;
			}
		}

		void region_lbl_SetText(String^ text)
		{
			// InvokeRequired required compares the thread ID of the
			// calling thread to the thread ID of the creating thread.
			// If these threads are different, it returns true.
			if (region_lbl->InvokeRequired)
			{
				SetTextDelegate^ d = 
					gcnew SetTextDelegate(this, &Form1::region_lbl_SetText);
				Invoke(d, gcnew array<Object^> { text });
			}
			else
			{
				region_lbl->Text = text;
			}
		}

		void area_lbl_SetText(String^ text)
		{
			// InvokeRequired required compares the thread ID of the
			// calling thread to the thread ID of the creating thread.
			// If these threads are different, it returns true.
			if (area_lbl->InvokeRequired)
			{
				SetTextDelegate^ d = 
					gcnew SetTextDelegate(this, &Form1::area_lbl_SetText);
				Invoke(d, gcnew array<Object^> { text });
			}
			else
			{
				area_lbl->Text = text;
			}
		}

		void G_lbl_SetText(String^ text)
		{
			// InvokeRequired required compares the thread ID of the
			// calling thread to the thread ID of the creating thread.
			// If these threads are different, it returns true.
			if (G_lbl->InvokeRequired)
			{
				SetTextDelegate^ d = 
					gcnew SetTextDelegate(this, &Form1::G_lbl_SetText);
				Invoke(d, gcnew array<Object^> { text });
			}
			else
			{
				G_lbl->Text = text;
			}
		}

		void R_lbl_SetText(String^ text)
		{
			// InvokeRequired required compares the thread ID of the
			// calling thread to the thread ID of the creating thread.
			// If these threads are different, it returns true.
			if (R_lbl->InvokeRequired)
			{
				SetTextDelegate^ d = 
					gcnew SetTextDelegate(this, &Form1::R_lbl_SetText);
				Invoke(d, gcnew array<Object^> { text });
			}
			else
			{
				R_lbl->Text = text;
			}
		}

		void B_lbl_SetText(String^ text)
		{
			// InvokeRequired required compares the thread ID of the
			// calling thread to the thread ID of the creating thread.
			// If these threads are different, it returns true.
			if (B_lbl->InvokeRequired)
			{
				SetTextDelegate^ d = 
					gcnew SetTextDelegate(this, &Form1::B_lbl_SetText);
				Invoke(d, gcnew array<Object^> { text });
			}
			else
			{
				B_lbl->Text = text;
			}
		}

		void G_CB_lbl_SetText(String^ text)
		{
			// InvokeRequired required compares the thread ID of the
			// calling thread to the thread ID of the creating thread.
			// If these threads are different, it returns true.
			if (G_CB_lbl->InvokeRequired)
			{
				SetTextDelegate^ d = 
					gcnew SetTextDelegate(this, &Form1::G_CB_lbl_SetText);
				Invoke(d, gcnew array<Object^> { text });
			}
			else
			{
				G_CB_lbl->Text = text;
			}
		}

		void R_CB_lbl_SetText(String^ text)
		{
			// InvokeRequired required compares the thread ID of the
			// calling thread to the thread ID of the creating thread.
			// If these threads are different, it returns true.
			if (R_CB_lbl->InvokeRequired)
			{
				SetTextDelegate^ d = 
					gcnew SetTextDelegate(this, &Form1::R_CB_lbl_SetText);
				Invoke(d, gcnew array<Object^> { text });
			}
			else
			{
				R_CB_lbl->Text = text;
			}
		}

		void B_CB_lbl_SetText(String^ text)
		{
			// InvokeRequired required compares the thread ID of the
			// calling thread to the thread ID of the creating thread.
			// If these threads are different, it returns true.
			if (B_CB_lbl->InvokeRequired)
			{
				SetTextDelegate^ d = 
					gcnew SetTextDelegate(this, &Form1::B_CB_lbl_SetText);
				Invoke(d, gcnew array<Object^> { text });
			}
			else
			{
				B_CB_lbl->Text = text;
			}
		}

		void delta_lbl_SetText(String^ text)
		{
			// InvokeRequired required compares the thread ID of the
			// calling thread to the thread ID of the creating thread.
			// If these threads are different, it returns true.
			if (delta_lbl->InvokeRequired)
			{
				SetTextDelegate^ d = 
					gcnew SetTextDelegate(this, &Form1::delta_lbl_SetText);
				Invoke(d, gcnew array<Object^> { text });
			}
			else
			{
				delta_lbl->Text = text;
			}
		}

		void delta2_lbl_SetText(String^ text)
		{
			// InvokeRequired required compares the thread ID of the
			// calling thread to the thread ID of the creating thread.
			// If these threads are different, it returns true.
			if (delta2_lbl->InvokeRequired)
			{
				SetTextDelegate^ d = 
					gcnew SetTextDelegate(this, &Form1::delta2_lbl_SetText);
				Invoke(d, gcnew array<Object^> { text });
			}
			else
			{
				delta2_lbl->Text = text;
			}
		}

		void epsilon_lbl_SetText(String^ text)
		{
			// InvokeRequired required compares the thread ID of the
			// calling thread to the thread ID of the creating thread.
			// If these threads are different, it returns true.
			if (epsilon_lbl->InvokeRequired)
			{
				SetTextDelegate^ d = 
					gcnew SetTextDelegate(this, &Form1::epsilon_lbl_SetText);
				Invoke(d, gcnew array<Object^> { text });
			}
			else
			{
				epsilon_lbl->Text = text;
			}
		}

		void epsilon2_lbl_SetText(String^ text)
		{
			// InvokeRequired required compares the thread ID of the
			// calling thread to the thread ID of the creating thread.
			// If these threads are different, it returns true.
			if (epsilon2_lbl->InvokeRequired)
			{
				SetTextDelegate^ d = 
					gcnew SetTextDelegate(this, &Form1::epsilon2_lbl_SetText);
				Invoke(d, gcnew array<Object^> { text });
			}
			else
			{
				epsilon2_lbl->Text = text;
			}
		}

		void pxlStatus_lbl_SetText(String^ text)
		{
			// InvokeRequired required compares the thread ID of the
			// calling thread to the thread ID of the creating thread.
			// If these threads are different, it returns true.
			if (pxlStatus_lbl->InvokeRequired)
			{
				SetTextDelegate^ d = 
					gcnew SetTextDelegate(this, &Form1::pxlStatus_lbl_SetText);
				Invoke(d, gcnew array<Object^> { text });
			}
			else
			{
				pxlStatus_lbl->Text = text;
			}
		}

		void pxlNum_lbl_SetText(String^ text)
		{
			// InvokeRequired required compares the thread ID of the
			// calling thread to the thread ID of the creating thread.
			// If these threads are different, it returns true.
			if (pxlNum_lbl->InvokeRequired)
			{
				SetTextDelegate^ d = 
					gcnew SetTextDelegate(this, &Form1::pxlNum_lbl_SetText);
				Invoke(d, gcnew array<Object^> { text });
			}
			else
			{
				pxlNum_lbl->Text = text;
			}
		}

		void lastacc_lbl_SetText(String^ text)
		{
			// InvokeRequired required compares the thread ID of the
			// calling thread to the thread ID of the creating thread.
			// If these threads are different, it returns true.
			if (lastacc_lbl->InvokeRequired)
			{
				SetTextDelegate^ d = 
					gcnew SetTextDelegate(this, &Form1::lastacc_lbl_SetText);
				Invoke(d, gcnew array<Object^> { text });
			}
			else
			{
				lastacc_lbl->Text = text;
			}
		}

		void firstacc_lbl_SetText(String^ text)
		{
			// InvokeRequired required compares the thread ID of the
			// calling thread to the thread ID of the creating thread.
			// If these threads are different, it returns true.
			if (firstacc_lbl->InvokeRequired)
			{
				SetTextDelegate^ d = 
					gcnew SetTextDelegate(this, &Form1::firstacc_lbl_SetText);
				Invoke(d, gcnew array<Object^> { text });
			}
			else
			{
				firstacc_lbl->Text = text;
			}
		}

		void maxCB_lbl_SetText(String^ text)
		{
			// InvokeRequired required compares the thread ID of the
			// calling thread to the thread ID of the creating thread.
			// If these threads are different, it returns true.
			if (maxCB_lbl->InvokeRequired)
			{
				SetTextDelegate^ d = 
					gcnew SetTextDelegate(this, &Form1::maxCB_lbl_SetText);
				Invoke(d, gcnew array<Object^> { text });
			}
			else
			{
				maxCB_lbl->Text = text;
			}
		}

		void MNRL_lbl_SetText(String^ text)
		{
			// InvokeRequired required compares the thread ID of the
			// calling thread to the thread ID of the creating thread.
			// If these threads are different, it returns true.
			if (MNRL_lbl->InvokeRequired)
			{
				SetTextDelegate^ d = 
					gcnew SetTextDelegate(this, &Form1::MNRL_lbl_SetText);
				Invoke(d, gcnew array<Object^> { text });
			}
			else
			{
				MNRL_lbl->Text = text;
			}
		}

		void freq_lbl_SetText(String^ text)
		{
			// InvokeRequired required compares the thread ID of the
			// calling thread to the thread ID of the creating thread.
			// If these threads are different, it returns true.
			if (freq_lbl->InvokeRequired)
			{
				SetTextDelegate^ d = 
					gcnew SetTextDelegate(this, &Form1::freq_lbl_SetText);
				Invoke(d, gcnew array<Object^> { text });
			}
			else
			{
				freq_lbl->Text = text;
			}
		}

		void codebookNumber_lbl_SetText(String^ text)
		{
			// InvokeRequired required compares the thread ID of the
			// calling thread to the thread ID of the creating thread.
			// If these threads are different, it returns true.
			if (codebookNumber_lbl->InvokeRequired)
			{
				SetTextDelegate^ d = 
					gcnew SetTextDelegate(this, &Form1::codebookNumber_lbl_SetText);
				Invoke(d, gcnew array<Object^> { text });
			}
			else
			{
				codebookNumber_lbl->Text = text;
			}
		}


		void desiredfps_txt_SetText(String^ text)
		{
			// InvokeRequired required compares the thread ID of the
			// calling thread to the thread ID of the creating thread.
			// If these threads are different, it returns true.
			if (desiredfps_txt->InvokeRequired)
			{
				SetTextDelegate^ d = 
					gcnew SetTextDelegate(this, &Form1::desiredfps_txt_SetText);
				Invoke(d, gcnew array<Object^> { text });
			}
			else
			{
				desiredfps_txt->Text = text;
			}
		}



		void LabConversionTimelbl_SetText(String^ text)
		{
			// InvokeRequired required compares the thread ID of the
			// calling thread to the thread ID of the creating thread.
			// If these threads are different, it returns true.
			if (LabConversionTimelbl->InvokeRequired)
			{
				SetTextDelegate^ d = 
					gcnew SetTextDelegate(this, &Form1::LabConversionTimelbl_SetText);
				Invoke(d, gcnew array<Object^> { text });
			}
			else
			{
				LabConversionTimelbl->Text = text;
			}
		}

		void Resizetimelbl_SetText(String^ text)
		{
			// InvokeRequired required compares the thread ID of the
			// calling thread to the thread ID of the creating thread.
			// If these threads are different, it returns true.
			if (Resize_time_lbl->InvokeRequired)
			{
				SetTextDelegate^ d = 
					gcnew SetTextDelegate(this, &Form1::Resizetimelbl_SetText);
				Invoke(d, gcnew array<Object^> { text });
			}
			else
			{
				Resize_time_lbl->Text = text;
			}
		}



		void HomomorphicTimelbl_SetText(String^ text)
		{
			// InvokeRequired required compares the thread ID of the
			// calling thread to the thread ID of the creating thread.
			// If these threads are different, it returns true.
			if (homomorphicTime_lbl->InvokeRequired)
			{
				SetTextDelegate^ d = 
					gcnew SetTextDelegate(this, &Form1::HomomorphicTimelbl_SetText);
				Invoke(d, gcnew array<Object^> { text });
			}
			else
			{
				homomorphicTime_lbl->Text = text;
			}
		}







		void frameNumber_lbl_SetText(String^ text)
		{
			// InvokeRequired required compares the thread ID of the
			// calling thread to the thread ID of the creating thread.
			// If these threads are different, it returns true.
			if (frameNumber_lbl->InvokeRequired)
			{
				SetTextDelegate^ d = 
					gcnew SetTextDelegate(this, &Form1::frameNumber_lbl_SetText);
				Invoke(d, gcnew array<Object^> { text });
			}
			else
			{
				frameNumber_lbl->Text = text;
			}
		}
		void PP2_lbl_SetText(String^ text)
		{
			// InvokeRequired required compares the thread ID of the
			// calling thread to the thread ID of the creating thread.
			// If these threads are different, it returns true.
			if (PP2_lbl->InvokeRequired)
			{
				SetTextDelegate^ d = 
					gcnew SetTextDelegate(this, &Form1::PP2_lbl_SetText);
				Invoke(d, gcnew array<Object^> { text });
			}
			else
			{
				PP2_lbl->Text = text;
			}
		}

		void DisplayTime_lbl_SetText(String^ text)
		{
			// InvokeRequired required compares the thread ID of the
			// calling thread to the thread ID of the creating thread.
			// If these threads are different, it returns true.
			if (DisplayTime_lbl->InvokeRequired)
			{
				SetTextDelegate^ d = 
					gcnew SetTextDelegate(this, &Form1::DisplayTime_lbl_SetText);
				Invoke(d, gcnew array<Object^> { text });
			}
			else
			{
				DisplayTime_lbl->Text = text;
			}
		}

		void SingleObjBehaviour_txt_SetText(String^ text)
		{
			// InvokeRequired required compares the thread ID of the
			// calling thread to the thread ID of the creating thread.
			// If these threads are different, it returns true.
			if (SingleObjBehaviour_txt->InvokeRequired)
			{
				SetTextDelegate^ d = 
					gcnew SetTextDelegate(this, &Form1::SingleObjBehaviour_txt_SetText);
				Invoke(d, gcnew array<Object^> { text });
			}
			else
			{
				SingleObjBehaviour_txt->Text = text;
			}
		}

		void TrackerTime_lbl_SetText(String^ text)
		{
			// InvokeRequired required compares the thread ID of the
			// calling thread to the thread ID of the creating thread.
			// If these threads are different, it returns true.
			if (TrackerTime_lbl->InvokeRequired)
			{
				SetTextDelegate^ d = 
					gcnew SetTextDelegate(this, &Form1::TrackerTime_lbl_SetText);
				Invoke(d, gcnew array<Object^> { text });
			}
			else
			{
				TrackerTime_lbl->Text = text;
			}
		}

		void BehaviourTime_lbl_SetText(String^ text)
		{
			// InvokeRequired required compares the thread ID of the
			// calling thread to the thread ID of the creating thread.
			// If these threads are different, it returns true.
			if (BehaviourTime_lbl->InvokeRequired)
			{
				SetTextDelegate^ d = 
					gcnew SetTextDelegate(this, &Form1::BehaviourTime_lbl_SetText);
				Invoke(d, gcnew array<Object^> { text });
			}
			else
			{
				BehaviourTime_lbl->Text = text;
			}
		}




		void updatetime_lbl_SetText(String^ text)
		{
			// InvokeRequired required compares the thread ID of the
			// calling thread to the thread ID of the creating thread.
			// If these threads are different, it returns true.
			if (updatetime_lbl->InvokeRequired)
			{
				SetTextDelegate^ d = 
					gcnew SetTextDelegate(this, &Form1::updatetime_lbl_SetText);
				Invoke(d, gcnew array<Object^> { text });
			}
			else
			{
				updatetime_lbl->Text = text;
			}
		}

		void BGtime_lbl_SetText(String^ text)
		{
			// InvokeRequired required compares the thread ID of the
			// calling thread to the thread ID of the creating thread.
			// If these threads are different, it returns true.
			if (BGtime_lbl->InvokeRequired)
			{
				SetTextDelegate^ d = 
					gcnew SetTextDelegate(this, &Form1::BGtime_lbl_SetText);
				Invoke(d, gcnew array<Object^> { text });
			}
			else
			{
				BGtime_lbl->Text = text;
			}
		}
		void mdntime_lbl_SetText(String^ text)
		{
			// InvokeRequired required compares the thread ID of the
			// calling thread to the thread ID of the creating thread.
			// If these threads are different, it returns true.
			if (mdntime_lbl->InvokeRequired)
			{
				SetTextDelegate^ d = 
					gcnew SetTextDelegate(this, &Form1::mdntime_lbl_SetText);
				Invoke(d, gcnew array<Object^> { text });
			}
			else
			{
				mdntime_lbl->Text = text;
			}
		}
		void pause_btn_SetState(bool state)
		{
			// InvokeRequired required compares the thread ID of the
			// calling thread to the thread ID of the creating thread.
			// If these threads are different, it returns true.
			if (pause_btn->InvokeRequired)
			{
				SetStateDelegate^ d = 
					gcnew SetStateDelegate(this, &Form1::pause_btn_SetState);
				Invoke(d, gcnew array<Object^> { state });
			}
			else
			{
				pause_btn->Enabled = state;
			}
		}
		void play_btn_SetState(bool state)
		{
			// InvokeRequired required compares the thread ID of the
			// calling thread to the thread ID of the creating thread.
			// If these threads are different, it returns true.
			if (mdntime_lbl->InvokeRequired)
			{
				SetStateDelegate^ d = 
					gcnew SetStateDelegate(this, &Form1::play_btn_SetState);
				Invoke(d, gcnew array<Object^> { state });
			}
			else
			{
				pause_btn->Enabled = state;
			}
		}


	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~Form1()
		{
			if (components)
			{
				delete components;
			}
		}

























	private: System::Windows::Forms::GroupBox^  selectSource_grp;



	private: System::Windows::Forms::GroupBox^  activatedOptions_grp;












































































	private: System::Windows::Forms::GroupBox^  Codebook_grp;
	private: System::Windows::Forms::Label^  lastacc_lbl;
	private: System::Windows::Forms::Label^  MNRL_lbl;
	private: System::Windows::Forms::Label^  freq_lbl;
	private: System::Windows::Forms::Label^  Imin_lbl;
	private: System::Windows::Forms::Label^  Imax_lbl;
	private: System::Windows::Forms::Label^  codebookNumber_lbl;
	private: System::Windows::Forms::Label^  pxlNum_lbl;
	private: System::Windows::Forms::Button^  prevCB_lbl;
	private: System::Windows::Forms::Button^  nextCB_lbl;
	private: System::Windows::Forms::Label^  firstacc_lbl;
	private: System::Windows::Forms::Label^  maxCB_lbl;
	private: System::Windows::Forms::Label^  B_lbl;
	private: System::Windows::Forms::Label^  G_lbl;
	private: System::Windows::Forms::Label^  R_lbl;
	private: System::Windows::Forms::GroupBox^  ProcTime_grp;

	private: System::Windows::Forms::Label^  mdntime_lbl;
	private: System::Windows::Forms::Label^  BGtime_lbl;
	private: System::Windows::Forms::Label^  updatetime_lbl;
	private: System::Windows::Forms::Label^  pxlStatus_lbl;
	private: System::Windows::Forms::Label^  epsilon_lbl;
	private: System::Windows::Forms::GroupBox^  param_grp;
	private: System::Windows::Forms::Label^  kdummy_lbl;

	private: System::Windows::Forms::Label^  betadummy_lbl;

	private: System::Windows::Forms::Label^  alphadummy_lbl;
	private: System::Windows::Forms::TextBox^  k_txt;


	private: System::Windows::Forms::TextBox^  beta_txt;

	private: System::Windows::Forms::TextBox^  alpha_txt;
	private: System::Windows::Forms::Label^  k2dummy_lbl;
	private: System::Windows::Forms::TextBox^  k2_txt;
	private: System::Windows::Forms::TextBox^  TM_txt;
	private: System::Windows::Forms::Label^  TMdummy_lbl;
	private: System::Windows::Forms::TextBox^  N_txt;
	private: System::Windows::Forms::Label^  Ndummy_lbl;





	private: System::Windows::Forms::Label^  B_CB_lbl;
	private: System::Windows::Forms::Label^  G_CB_lbl;
	private: System::Windows::Forms::Label^  R_CB_lbl;

	private: System::Windows::Forms::Label^  delta_lbl;
	private: System::Windows::Forms::Label^  Ihigh_lbl;
	private: System::Windows::Forms::Label^  Ilow_lbl;
	private: System::Windows::Forms::Label^  I_lbl;
	private: System::Windows::Forms::Button^  showmap_btn;
	private: System::Windows::Forms::Button^  map_prev_btn;
	private: System::Windows::Forms::Button^  map_next_btn;
	private: System::Windows::Forms::TextBox^  Lmax_txt;

	private: System::Windows::Forms::Label^  Lmax_lbl;
	private: System::Windows::Forms::Label^  Resizebydummy_lbl;
	private: System::Windows::Forms::TextBox^  resize_txt;
	private: System::Windows::Forms::TextBox^  DeltaE_txt;
	private: System::Windows::Forms::Label^  DeltaE_lbl;






	private: System::Windows::Forms::Button^  btn_setParam;
	private: System::Windows::Forms::Label^  CB_color_lbl;


	private: System::Windows::Forms::Label^  pxl_color_lbl;
	private: System::Windows::Forms::Label^  DeltaL_lbl;
	private: System::Windows::Forms::Label^  DeltaC_lbl;
	private: System::Windows::Forms::TextBox^  DeltaL_txt;
	private: System::Windows::Forms::TextBox^  DeltaC_txt;
	private: System::Windows::Forms::Label^  delta2_lbl;
	private: System::Windows::Forms::Label^  epsilon2_lbl;



	private: System::Windows::Forms::Label^  frameNumber_lbl;

	private: System::Windows::Forms::CheckBox^  disableUpdate_chk;
	private: System::Windows::Forms::Label^  DeltaS_lbl;
	private: System::Windows::Forms::TextBox^  DeltaS_txt;
	private: System::Windows::Forms::TextBox^  fmin_txt;
	private: System::Windows::Forms::Label^  fmin_lbl;
	private: System::Windows::Forms::CheckBox^  fmin_chk;
	private: System::Windows::Forms::CheckBox^  NFPP_chk;
	private: System::Windows::Forms::CheckBox^  minArea_chk;


	private: System::Windows::Forms::Label^  PP2_lbl;



	private: System::Windows::Forms::Label^  totalTime_lbl;



	private: System::Windows::Forms::Label^  area_lbl;
	private: System::Windows::Forms::Label^  region_lbl;























	private: System::Windows::Forms::Label^  TrackerTime_lbl;











	private: System::Windows::Forms::GroupBox^  processingtime_grp;
	private: System::Windows::Forms::RadioButton^  frmbyfrm_radio;

	private: System::Windows::Forms::RadioButton^  realtime_radio;

	private: System::Windows::Forms::OpenFileDialog^  openVideo_dlg;
	private: System::Windows::Forms::GroupBox^  Output_grp;



	private: System::Windows::Forms::TextBox^  saveFrames_txt;



	private: System::Windows::Forms::CheckBox^  trackerEnables_chk;
	private: System::Windows::Forms::Label^  minAreaThreshold_chk;
	private: System::Windows::Forms::TextBox^  minAreaThreshold_txt;
	private: System::Windows::Forms::TextBox^  wantedSpeed_txt;
	private: System::Windows::Forms::Label^  Speeddummy_lbl;
	private: System::Windows::Forms::CheckBox^  trackaftertrainingonly_chk;

	private: System::Windows::Forms::GroupBox^  selectVideoSource_grp;


	private: System::Windows::Forms::Button^  selectVideo_btn;
	private: System::Windows::Forms::Button^  video13_btn;

	private: System::Windows::Forms::Button^  video11_btn;


	private: System::Windows::Forms::Button^  video8_btn;
	private: System::Windows::Forms::Button^  video7_btn;
	private: System::Windows::Forms::Button^  video6_btn;
	private: System::Windows::Forms::Button^  video5_btn;
	private: System::Windows::Forms::Button^  video4_btn;
	private: System::Windows::Forms::Button^  video3_btn;
	private: System::Windows::Forms::Button^  video2_btn;
	private: System::Windows::Forms::Button^  video1_btn;
	private: System::Windows::Forms::GroupBox^  selectImagesSource_grp;

	private: System::Windows::Forms::Label^  ImageNumberOfDigits_lbl;
	private: System::Windows::Forms::Label^  label2;
	private: System::Windows::Forms::Label^  ImagesPath_lbl;
	private: System::Windows::Forms::GroupBox^  mainAlgorithm_grp;





	private: System::Windows::Forms::Button^  setMethod_btn;
	private: System::Windows::Forms::RadioButton^  LabCylinder_radio;
	private: System::Windows::Forms::RadioButton^  LabSpace_radio;
	private: System::Windows::Forms::RadioButton^  OriginalCB_radio;
	private: System::Windows::Forms::RadioButton^  vid_rdb;
	private: System::Windows::Forms::RadioButton^  Img_rdb;

	protected private: System::Windows::Forms::GroupBox^  whileRunningControls_grp;
	private: 
	private: System::Windows::Forms::Button^  saveCB_btn;
	protected private: 
	private: System::Windows::Forms::Button^  savecurrentFrame_btn;

	private: System::Windows::Forms::Button^  pause_btn;
	private: System::Windows::Forms::Button^  play_btn;
	private: System::Windows::Forms::Button^  LoadFromimages_btn;
	private: System::Windows::Forms::TextBox^  LoadImagesNumberOfDigits_txt;
	private: System::Windows::Forms::TextBox^  LoadImagesExtension_txt;
	private: System::Windows::Forms::TextBox^  LoadImagesPath_txt;
	private: System::Windows::Forms::Button^  btn_halt;


	private: System::Windows::Forms::Label^  maxAreaThreshold_lbl;
	private: System::Windows::Forms::TextBox^  maxAreaThreshold_txt;
	private: System::Windows::Forms::GroupBox^  timeOrFrames_grp;
	private: System::Windows::Forms::RadioButton^  inTermsOfFrames_radio;

	private: System::Windows::Forms::RadioButton^  inTermsOfTime_radio;
	private: System::Windows::Forms::Label^  videoTotalTime_lbl;
	private: System::Windows::Forms::TextBox^  readfromimagesfps_txt;

	private: System::Windows::Forms::Label^  readfromimagesfps_lbl;
	private: System::Windows::Forms::CheckBox^  saveFrames_rdo;
	private: System::Windows::Forms::TextBox^  ObjProperties_txt;


	private: System::Windows::Forms::TextBox^  CCProperties_txt;
	private: System::Windows::Forms::CheckBox^  homomorphic_chk;
	private: System::Windows::Forms::CheckBox^  smooth_chk;
	private: System::Windows::Forms::CheckBox^  useMorphology_chk;
	private: System::Windows::Forms::CheckBox^  traintest_chk;
	private: System::Windows::Forms::Label^  homomorphicTime_lbl;
	private: System::Windows::Forms::Label^  LabConversionTimelbl;
	private: System::Windows::Forms::Label^  DisplayTime_lbl;
	private: System::Windows::Forms::Label^  Resize_time_lbl;
	private: System::Windows::Forms::CheckBox^  ResizeProcessedbydummy_lbl;
	private: System::Windows::Forms::Button^  LoadCB_btn;
	private: System::Windows::Forms::CheckBox^  debug_monitor_chk;
	private: System::Windows::Forms::Label^  label_startfromdummy;
	private: System::Windows::Forms::TextBox^  startfrom_txt;
	private: System::Windows::Forms::CheckBox^  camera_chk;
	private: System::Windows::Forms::Label^  dummy_fps_lbl;
	private: System::Windows::Forms::TextBox^  desiredfps_txt;
	private: System::Windows::Forms::TextBox^  SingleObjBehaviour_txt;










	private: System::Windows::Forms::Label^  dummyObjectBehaviour_lbl;
	private: System::Windows::Forms::Button^  oneObjectHist_btn;
	private: System::Windows::Forms::TextBox^  ObjectBehaviour_txt;
	private: System::Windows::Forms::GroupBox^  dummySingleObjectBehaviour_grp;

	private: System::Windows::Forms::CheckBox^  BehaviourRecognition_chk;
	private: System::Windows::Forms::Label^  BehaviourTime_lbl;
private: System::Windows::Forms::CheckBox^  loadInitialMask_chk;
private: System::Windows::Forms::CheckBox^  Show_prediction_chk;
private: System::Windows::Forms::CheckBox^  show_speed_height_chk;
private: System::Windows::Forms::CheckBox^  show_dummy_chk;
private: System::Windows::Forms::CheckBox^  show_blob_chk;
private: System::Windows::Forms::CheckBox^  ghost_chk;
private: System::Windows::Forms::CheckBox^  useHistDifference_chk;
private: System::Windows::Forms::CheckBox^  faint_chk;
private: System::Windows::Forms::Button^  button1;
private: System::Windows::Forms::Button^  button2;
private: System::Windows::Forms::Button^  button9;
private: System::Windows::Forms::Button^  button8;
private: System::Windows::Forms::Button^  button7;
private: System::Windows::Forms::Button^  button6;
private: System::Windows::Forms::Button^  button5;
private: System::Windows::Forms::Button^  button4;
private: System::Windows::Forms::Button^  button3;
private: System::Windows::Forms::Button^  button10;
private: System::Windows::Forms::Button^  button11;
private: System::Windows::Forms::Button^  button12;



































































































































	private: System::ComponentModel::IContainer^  components;




	protected: 

	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>


#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			this->selectSource_grp = (gcnew System::Windows::Forms::GroupBox());
			this->vid_rdb = (gcnew System::Windows::Forms::RadioButton());
			this->Img_rdb = (gcnew System::Windows::Forms::RadioButton());
			this->selectVideoSource_grp = (gcnew System::Windows::Forms::GroupBox());
			this->button10 = (gcnew System::Windows::Forms::Button());
			this->button2 = (gcnew System::Windows::Forms::Button());
			this->button1 = (gcnew System::Windows::Forms::Button());
			this->selectVideo_btn = (gcnew System::Windows::Forms::Button());
			this->video13_btn = (gcnew System::Windows::Forms::Button());
			this->video7_btn = (gcnew System::Windows::Forms::Button());
			this->video6_btn = (gcnew System::Windows::Forms::Button());
			this->video5_btn = (gcnew System::Windows::Forms::Button());
			this->video11_btn = (gcnew System::Windows::Forms::Button());
			this->video4_btn = (gcnew System::Windows::Forms::Button());
			this->video2_btn = (gcnew System::Windows::Forms::Button());
			this->video1_btn = (gcnew System::Windows::Forms::Button());
			this->selectImagesSource_grp = (gcnew System::Windows::Forms::GroupBox());
			this->button11 = (gcnew System::Windows::Forms::Button());
			this->button9 = (gcnew System::Windows::Forms::Button());
			this->button8 = (gcnew System::Windows::Forms::Button());
			this->button7 = (gcnew System::Windows::Forms::Button());
			this->button6 = (gcnew System::Windows::Forms::Button());
			this->button5 = (gcnew System::Windows::Forms::Button());
			this->button4 = (gcnew System::Windows::Forms::Button());
			this->button3 = (gcnew System::Windows::Forms::Button());
			this->label_startfromdummy = (gcnew System::Windows::Forms::Label());
			this->startfrom_txt = (gcnew System::Windows::Forms::TextBox());
			this->readfromimagesfps_txt = (gcnew System::Windows::Forms::TextBox());
			this->readfromimagesfps_lbl = (gcnew System::Windows::Forms::Label());
			this->LoadFromimages_btn = (gcnew System::Windows::Forms::Button());
			this->LoadImagesNumberOfDigits_txt = (gcnew System::Windows::Forms::TextBox());
			this->ImageNumberOfDigits_lbl = (gcnew System::Windows::Forms::Label());
			this->LoadImagesExtension_txt = (gcnew System::Windows::Forms::TextBox());
			this->label2 = (gcnew System::Windows::Forms::Label());
			this->video3_btn = (gcnew System::Windows::Forms::Button());
			this->LoadImagesPath_txt = (gcnew System::Windows::Forms::TextBox());
			this->ImagesPath_lbl = (gcnew System::Windows::Forms::Label());
			this->video8_btn = (gcnew System::Windows::Forms::Button());
			this->activatedOptions_grp = (gcnew System::Windows::Forms::GroupBox());
			this->useHistDifference_chk = (gcnew System::Windows::Forms::CheckBox());
			this->ghost_chk = (gcnew System::Windows::Forms::CheckBox());
			this->BehaviourRecognition_chk = (gcnew System::Windows::Forms::CheckBox());
			this->traintest_chk = (gcnew System::Windows::Forms::CheckBox());
			this->useMorphology_chk = (gcnew System::Windows::Forms::CheckBox());
			this->smooth_chk = (gcnew System::Windows::Forms::CheckBox());
			this->homomorphic_chk = (gcnew System::Windows::Forms::CheckBox());
			this->trackaftertrainingonly_chk = (gcnew System::Windows::Forms::CheckBox());
			this->trackerEnables_chk = (gcnew System::Windows::Forms::CheckBox());
			this->minArea_chk = (gcnew System::Windows::Forms::CheckBox());
			this->disableUpdate_chk = (gcnew System::Windows::Forms::CheckBox());
			this->fmin_chk = (gcnew System::Windows::Forms::CheckBox());
			this->NFPP_chk = (gcnew System::Windows::Forms::CheckBox());
			this->timeOrFrames_grp = (gcnew System::Windows::Forms::GroupBox());
			this->inTermsOfFrames_radio = (gcnew System::Windows::Forms::RadioButton());
			this->inTermsOfTime_radio = (gcnew System::Windows::Forms::RadioButton());
			this->Codebook_grp = (gcnew System::Windows::Forms::GroupBox());
			this->area_lbl = (gcnew System::Windows::Forms::Label());
			this->region_lbl = (gcnew System::Windows::Forms::Label());
			this->epsilon2_lbl = (gcnew System::Windows::Forms::Label());
			this->delta2_lbl = (gcnew System::Windows::Forms::Label());
			this->CB_color_lbl = (gcnew System::Windows::Forms::Label());
			this->pxl_color_lbl = (gcnew System::Windows::Forms::Label());
			this->map_prev_btn = (gcnew System::Windows::Forms::Button());
			this->map_next_btn = (gcnew System::Windows::Forms::Button());
			this->showmap_btn = (gcnew System::Windows::Forms::Button());
			this->I_lbl = (gcnew System::Windows::Forms::Label());
			this->Ihigh_lbl = (gcnew System::Windows::Forms::Label());
			this->Ilow_lbl = (gcnew System::Windows::Forms::Label());
			this->delta_lbl = (gcnew System::Windows::Forms::Label());
			this->B_CB_lbl = (gcnew System::Windows::Forms::Label());
			this->G_CB_lbl = (gcnew System::Windows::Forms::Label());
			this->R_CB_lbl = (gcnew System::Windows::Forms::Label());
			this->epsilon_lbl = (gcnew System::Windows::Forms::Label());
			this->pxlStatus_lbl = (gcnew System::Windows::Forms::Label());
			this->B_lbl = (gcnew System::Windows::Forms::Label());
			this->G_lbl = (gcnew System::Windows::Forms::Label());
			this->R_lbl = (gcnew System::Windows::Forms::Label());
			this->maxCB_lbl = (gcnew System::Windows::Forms::Label());
			this->pxlNum_lbl = (gcnew System::Windows::Forms::Label());
			this->prevCB_lbl = (gcnew System::Windows::Forms::Button());
			this->nextCB_lbl = (gcnew System::Windows::Forms::Button());
			this->firstacc_lbl = (gcnew System::Windows::Forms::Label());
			this->lastacc_lbl = (gcnew System::Windows::Forms::Label());
			this->MNRL_lbl = (gcnew System::Windows::Forms::Label());
			this->freq_lbl = (gcnew System::Windows::Forms::Label());
			this->Imin_lbl = (gcnew System::Windows::Forms::Label());
			this->Imax_lbl = (gcnew System::Windows::Forms::Label());
			this->codebookNumber_lbl = (gcnew System::Windows::Forms::Label());
			this->ProcTime_grp = (gcnew System::Windows::Forms::GroupBox());
			this->BehaviourTime_lbl = (gcnew System::Windows::Forms::Label());
			this->Resize_time_lbl = (gcnew System::Windows::Forms::Label());
			this->DisplayTime_lbl = (gcnew System::Windows::Forms::Label());
			this->LabConversionTimelbl = (gcnew System::Windows::Forms::Label());
			this->homomorphicTime_lbl = (gcnew System::Windows::Forms::Label());
			this->TrackerTime_lbl = (gcnew System::Windows::Forms::Label());
			this->totalTime_lbl = (gcnew System::Windows::Forms::Label());
			this->PP2_lbl = (gcnew System::Windows::Forms::Label());
			this->BGtime_lbl = (gcnew System::Windows::Forms::Label());
			this->updatetime_lbl = (gcnew System::Windows::Forms::Label());
			this->mdntime_lbl = (gcnew System::Windows::Forms::Label());
			this->param_grp = (gcnew System::Windows::Forms::GroupBox());
			this->ResizeProcessedbydummy_lbl = (gcnew System::Windows::Forms::CheckBox());
			this->maxAreaThreshold_lbl = (gcnew System::Windows::Forms::Label());
			this->maxAreaThreshold_txt = (gcnew System::Windows::Forms::TextBox());
			this->minAreaThreshold_chk = (gcnew System::Windows::Forms::Label());
			this->minAreaThreshold_txt = (gcnew System::Windows::Forms::TextBox());
			this->wantedSpeed_txt = (gcnew System::Windows::Forms::TextBox());
			this->Speeddummy_lbl = (gcnew System::Windows::Forms::Label());
			this->fmin_txt = (gcnew System::Windows::Forms::TextBox());
			this->fmin_lbl = (gcnew System::Windows::Forms::Label());
			this->DeltaS_lbl = (gcnew System::Windows::Forms::Label());
			this->DeltaS_txt = (gcnew System::Windows::Forms::TextBox());
			this->DeltaL_lbl = (gcnew System::Windows::Forms::Label());
			this->btn_setParam = (gcnew System::Windows::Forms::Button());
			this->DeltaC_lbl = (gcnew System::Windows::Forms::Label());
			this->DeltaL_txt = (gcnew System::Windows::Forms::TextBox());
			this->DeltaC_txt = (gcnew System::Windows::Forms::TextBox());
			this->DeltaE_txt = (gcnew System::Windows::Forms::TextBox());
			this->DeltaE_lbl = (gcnew System::Windows::Forms::Label());
			this->Lmax_txt = (gcnew System::Windows::Forms::TextBox());
			this->Lmax_lbl = (gcnew System::Windows::Forms::Label());
			this->TM_txt = (gcnew System::Windows::Forms::TextBox());
			this->TMdummy_lbl = (gcnew System::Windows::Forms::Label());
			this->resize_txt = (gcnew System::Windows::Forms::TextBox());
			this->N_txt = (gcnew System::Windows::Forms::TextBox());
			this->Ndummy_lbl = (gcnew System::Windows::Forms::Label());
			this->Resizebydummy_lbl = (gcnew System::Windows::Forms::Label());
			this->k2dummy_lbl = (gcnew System::Windows::Forms::Label());
			this->k2_txt = (gcnew System::Windows::Forms::TextBox());
			this->kdummy_lbl = (gcnew System::Windows::Forms::Label());
			this->betadummy_lbl = (gcnew System::Windows::Forms::Label());
			this->alphadummy_lbl = (gcnew System::Windows::Forms::Label());
			this->k_txt = (gcnew System::Windows::Forms::TextBox());
			this->beta_txt = (gcnew System::Windows::Forms::TextBox());
			this->alpha_txt = (gcnew System::Windows::Forms::TextBox());
			this->frameNumber_lbl = (gcnew System::Windows::Forms::Label());
			this->processingtime_grp = (gcnew System::Windows::Forms::GroupBox());
			this->desiredfps_txt = (gcnew System::Windows::Forms::TextBox());
			this->dummy_fps_lbl = (gcnew System::Windows::Forms::Label());
			this->frmbyfrm_radio = (gcnew System::Windows::Forms::RadioButton());
			this->realtime_radio = (gcnew System::Windows::Forms::RadioButton());
			this->openVideo_dlg = (gcnew System::Windows::Forms::OpenFileDialog());
			this->Output_grp = (gcnew System::Windows::Forms::GroupBox());
			this->camera_chk = (gcnew System::Windows::Forms::CheckBox());
			this->saveFrames_rdo = (gcnew System::Windows::Forms::CheckBox());
			this->saveFrames_txt = (gcnew System::Windows::Forms::TextBox());
			this->mainAlgorithm_grp = (gcnew System::Windows::Forms::GroupBox());
			this->setMethod_btn = (gcnew System::Windows::Forms::Button());
			this->LabCylinder_radio = (gcnew System::Windows::Forms::RadioButton());
			this->LabSpace_radio = (gcnew System::Windows::Forms::RadioButton());
			this->OriginalCB_radio = (gcnew System::Windows::Forms::RadioButton());
			this->whileRunningControls_grp = (gcnew System::Windows::Forms::GroupBox());
			this->LoadCB_btn = (gcnew System::Windows::Forms::Button());
			this->saveCB_btn = (gcnew System::Windows::Forms::Button());
			this->savecurrentFrame_btn = (gcnew System::Windows::Forms::Button());
			this->pause_btn = (gcnew System::Windows::Forms::Button());
			this->play_btn = (gcnew System::Windows::Forms::Button());
			this->btn_halt = (gcnew System::Windows::Forms::Button());
			this->videoTotalTime_lbl = (gcnew System::Windows::Forms::Label());
			this->ObjProperties_txt = (gcnew System::Windows::Forms::TextBox());
			this->CCProperties_txt = (gcnew System::Windows::Forms::TextBox());
			this->debug_monitor_chk = (gcnew System::Windows::Forms::CheckBox());
			this->SingleObjBehaviour_txt = (gcnew System::Windows::Forms::TextBox());
			this->dummyObjectBehaviour_lbl = (gcnew System::Windows::Forms::Label());
			this->oneObjectHist_btn = (gcnew System::Windows::Forms::Button());
			this->ObjectBehaviour_txt = (gcnew System::Windows::Forms::TextBox());
			this->dummySingleObjectBehaviour_grp = (gcnew System::Windows::Forms::GroupBox());
			this->loadInitialMask_chk = (gcnew System::Windows::Forms::CheckBox());
			this->Show_prediction_chk = (gcnew System::Windows::Forms::CheckBox());
			this->show_speed_height_chk = (gcnew System::Windows::Forms::CheckBox());
			this->show_dummy_chk = (gcnew System::Windows::Forms::CheckBox());
			this->show_blob_chk = (gcnew System::Windows::Forms::CheckBox());
			this->faint_chk = (gcnew System::Windows::Forms::CheckBox());
			this->button12 = (gcnew System::Windows::Forms::Button());
			this->selectSource_grp->SuspendLayout();
			this->selectVideoSource_grp->SuspendLayout();
			this->selectImagesSource_grp->SuspendLayout();
			this->activatedOptions_grp->SuspendLayout();
			this->timeOrFrames_grp->SuspendLayout();
			this->Codebook_grp->SuspendLayout();
			this->ProcTime_grp->SuspendLayout();
			this->param_grp->SuspendLayout();
			this->processingtime_grp->SuspendLayout();
			this->Output_grp->SuspendLayout();
			this->mainAlgorithm_grp->SuspendLayout();
			this->whileRunningControls_grp->SuspendLayout();
			this->dummySingleObjectBehaviour_grp->SuspendLayout();
			this->SuspendLayout();
			// 
			// selectSource_grp
			// 
			this->selectSource_grp->Controls->Add(this->vid_rdb);
			this->selectSource_grp->Controls->Add(this->Img_rdb);
			this->selectSource_grp->Controls->Add(this->selectVideoSource_grp);
			this->selectSource_grp->Controls->Add(this->selectImagesSource_grp);
			this->selectSource_grp->Location = System::Drawing::Point(14, 12);
			this->selectSource_grp->Name = L"selectSource_grp";
			this->selectSource_grp->Size = System::Drawing::Size(279, 553);
			this->selectSource_grp->TabIndex = 5;
			this->selectSource_grp->TabStop = false;
			this->selectSource_grp->Text = L"Select Video/Images Source";
			this->selectSource_grp->Enter += gcnew System::EventHandler(this, &Form1::groupBox4_Enter);
			// 
			// vid_rdb
			// 
			this->vid_rdb->AutoSize = true;
			this->vid_rdb->Checked = true;
			this->vid_rdb->Location = System::Drawing::Point(162, 19);
			this->vid_rdb->Name = L"vid_rdb";
			this->vid_rdb->Size = System::Drawing::Size(52, 17);
			this->vid_rdb->TabIndex = 44;
			this->vid_rdb->TabStop = true;
			this->vid_rdb->Text = L"Video";
			this->vid_rdb->UseVisualStyleBackColor = true;
			this->vid_rdb->CheckedChanged += gcnew System::EventHandler(this, &Form1::vid_rdb_CheckedChanged);
			// 
			// Img_rdb
			// 
			this->Img_rdb->AutoSize = true;
			this->Img_rdb->Location = System::Drawing::Point(16, 18);
			this->Img_rdb->Name = L"Img_rdb";
			this->Img_rdb->Size = System::Drawing::Size(58, 17);
			this->Img_rdb->TabIndex = 43;
			this->Img_rdb->TabStop = true;
			this->Img_rdb->Text = L"images";
			this->Img_rdb->UseVisualStyleBackColor = true;
			this->Img_rdb->CheckedChanged += gcnew System::EventHandler(this, &Form1::Img_rdb_CheckedChanged);
			// 
			// selectVideoSource_grp
			// 
			this->selectVideoSource_grp->Controls->Add(this->button12);
			this->selectVideoSource_grp->Controls->Add(this->button10);
			this->selectVideoSource_grp->Controls->Add(this->button2);
			this->selectVideoSource_grp->Controls->Add(this->button1);
			this->selectVideoSource_grp->Controls->Add(this->selectVideo_btn);
			this->selectVideoSource_grp->Controls->Add(this->video13_btn);
			this->selectVideoSource_grp->Controls->Add(this->video7_btn);
			this->selectVideoSource_grp->Controls->Add(this->video6_btn);
			this->selectVideoSource_grp->Controls->Add(this->video5_btn);
			this->selectVideoSource_grp->Controls->Add(this->video11_btn);
			this->selectVideoSource_grp->Controls->Add(this->video4_btn);
			this->selectVideoSource_grp->Controls->Add(this->video2_btn);
			this->selectVideoSource_grp->Controls->Add(this->video1_btn);
			this->selectVideoSource_grp->Location = System::Drawing::Point(16, 371);
			this->selectVideoSource_grp->Name = L"selectVideoSource_grp";
			this->selectVideoSource_grp->Size = System::Drawing::Size(257, 164);
			this->selectVideoSource_grp->TabIndex = 42;
			this->selectVideoSource_grp->TabStop = false;
			this->selectVideoSource_grp->Text = L"Videos";
			// 
			// button10
			// 
			this->button10->Location = System::Drawing::Point(183, 112);
			this->button10->Name = L"button10";
			this->button10->Size = System::Drawing::Size(75, 23);
			this->button10->TabIndex = 33;
			this->button10->Text = L"EASY";
			this->button10->UseVisualStyleBackColor = true;
			this->button10->Click += gcnew System::EventHandler(this, &Form1::button10_Click_2);
			// 
			// button2
			// 
			this->button2->Location = System::Drawing::Point(104, 112);
			this->button2->Name = L"button2";
			this->button2->Size = System::Drawing::Size(75, 23);
			this->button2->TabIndex = 32;
			this->button2->Text = L"BHV fight";
			this->button2->UseVisualStyleBackColor = true;
			this->button2->Click += gcnew System::EventHandler(this, &Form1::button2_Click_1);
			// 
			// button1
			// 
			this->button1->Location = System::Drawing::Point(183, 180);
			this->button1->Name = L"button1";
			this->button1->Size = System::Drawing::Size(75, 23);
			this->button1->TabIndex = 31;
			this->button1->Text = L"BHV meet 2";
			this->button1->UseVisualStyleBackColor = true;
			this->button1->Click += gcnew System::EventHandler(this, &Form1::button1_Click);
			// 
			// selectVideo_btn
			// 
			this->selectVideo_btn->Location = System::Drawing::Point(46, 19);
			this->selectVideo_btn->Name = L"selectVideo_btn";
			this->selectVideo_btn->Size = System::Drawing::Size(86, 23);
			this->selectVideo_btn->TabIndex = 30;
			this->selectVideo_btn->Text = L"...";
			this->selectVideo_btn->UseVisualStyleBackColor = true;
			this->selectVideo_btn->Click += gcnew System::EventHandler(this, &Form1::selectVideo_btn_Click);
			// 
			// video13_btn
			// 
			this->video13_btn->Location = System::Drawing::Point(104, 82);
			this->video13_btn->Name = L"video13_btn";
			this->video13_btn->Size = System::Drawing::Size(75, 23);
			this->video13_btn->TabIndex = 29;
			this->video13_btn->Text = L"rest wiggle";
			this->video13_btn->UseVisualStyleBackColor = true;
			this->video13_btn->Click += gcnew System::EventHandler(this, &Form1::video13_btn_Click);
			// 
			// video7_btn
			// 
			this->video7_btn->Location = System::Drawing::Point(182, 54);
			this->video7_btn->Name = L"video7_btn";
			this->video7_btn->Size = System::Drawing::Size(76, 23);
			this->video7_btn->TabIndex = 23;
			this->video7_btn->Text = L"left bag pick up";
			this->video7_btn->UseVisualStyleBackColor = true;
			this->video7_btn->Click += gcnew System::EventHandler(this, &Form1::video7_btn_Click);
			// 
			// video6_btn
			// 
			this->video6_btn->Location = System::Drawing::Point(104, 53);
			this->video6_btn->Name = L"video6_btn";
			this->video6_btn->Size = System::Drawing::Size(75, 23);
			this->video6_btn->TabIndex = 22;
			this->video6_btn->Text = L"left bag";
			this->video6_btn->UseVisualStyleBackColor = true;
			this->video6_btn->Click += gcnew System::EventHandler(this, &Form1::video6_btn_Click);
			// 
			// video5_btn
			// 
			this->video5_btn->Location = System::Drawing::Point(6, 112);
			this->video5_btn->Name = L"video5_btn";
			this->video5_btn->Size = System::Drawing::Size(93, 23);
			this->video5_btn->TabIndex = 21;
			this->video5_btn->Text = L"rest on floor";
			this->video5_btn->UseVisualStyleBackColor = true;
			this->video5_btn->Click += gcnew System::EventHandler(this, &Form1::video5_btn_Click);
			// 
			// video11_btn
			// 
			this->video11_btn->Location = System::Drawing::Point(183, 82);
			this->video11_btn->Name = L"video11_btn";
			this->video11_btn->Size = System::Drawing::Size(75, 23);
			this->video11_btn->TabIndex = 27;
			this->video11_btn->Text = L"walk split";
			this->video11_btn->UseVisualStyleBackColor = true;
			this->video11_btn->Click += gcnew System::EventHandler(this, &Form1::video11_btn_Click);
			// 
			// video4_btn
			// 
			this->video4_btn->Location = System::Drawing::Point(6, 83);
			this->video4_btn->Name = L"video4_btn";
			this->video4_btn->Size = System::Drawing::Size(93, 23);
			this->video4_btn->TabIndex = 20;
			this->video4_btn->Text = L"walk together";
			this->video4_btn->UseVisualStyleBackColor = true;
			this->video4_btn->Click += gcnew System::EventHandler(this, &Form1::video4_btn_Click);
			// 
			// video2_btn
			// 
			this->video2_btn->Location = System::Drawing::Point(6, 53);
			this->video2_btn->Name = L"video2_btn";
			this->video2_btn->Size = System::Drawing::Size(93, 23);
			this->video2_btn->TabIndex = 18;
			this->video2_btn->Text = L"Fight man down";
			this->video2_btn->UseVisualStyleBackColor = true;
			this->video2_btn->Click += gcnew System::EventHandler(this, &Form1::video2_btn_Click);
			// 
			// video1_btn
			// 
			this->video1_btn->Location = System::Drawing::Point(151, 19);
			this->video1_btn->Name = L"video1_btn";
			this->video1_btn->Size = System::Drawing::Size(75, 23);
			this->video1_btn->TabIndex = 17;
			this->video1_btn->Text = L"Video 1";
			this->video1_btn->UseVisualStyleBackColor = true;
			this->video1_btn->Click += gcnew System::EventHandler(this, &Form1::button2_Click);
			// 
			// selectImagesSource_grp
			// 
			this->selectImagesSource_grp->Controls->Add(this->button11);
			this->selectImagesSource_grp->Controls->Add(this->button9);
			this->selectImagesSource_grp->Controls->Add(this->button8);
			this->selectImagesSource_grp->Controls->Add(this->button7);
			this->selectImagesSource_grp->Controls->Add(this->button6);
			this->selectImagesSource_grp->Controls->Add(this->button5);
			this->selectImagesSource_grp->Controls->Add(this->button4);
			this->selectImagesSource_grp->Controls->Add(this->button3);
			this->selectImagesSource_grp->Controls->Add(this->label_startfromdummy);
			this->selectImagesSource_grp->Controls->Add(this->startfrom_txt);
			this->selectImagesSource_grp->Controls->Add(this->readfromimagesfps_txt);
			this->selectImagesSource_grp->Controls->Add(this->readfromimagesfps_lbl);
			this->selectImagesSource_grp->Controls->Add(this->LoadFromimages_btn);
			this->selectImagesSource_grp->Controls->Add(this->LoadImagesNumberOfDigits_txt);
			this->selectImagesSource_grp->Controls->Add(this->ImageNumberOfDigits_lbl);
			this->selectImagesSource_grp->Controls->Add(this->LoadImagesExtension_txt);
			this->selectImagesSource_grp->Controls->Add(this->label2);
			this->selectImagesSource_grp->Controls->Add(this->video3_btn);
			this->selectImagesSource_grp->Controls->Add(this->LoadImagesPath_txt);
			this->selectImagesSource_grp->Controls->Add(this->ImagesPath_lbl);
			this->selectImagesSource_grp->Controls->Add(this->video8_btn);
			this->selectImagesSource_grp->Enabled = false;
			this->selectImagesSource_grp->Location = System::Drawing::Point(9, 43);
			this->selectImagesSource_grp->Name = L"selectImagesSource_grp";
			this->selectImagesSource_grp->Size = System::Drawing::Size(264, 315);
			this->selectImagesSource_grp->TabIndex = 41;
			this->selectImagesSource_grp->TabStop = false;
			this->selectImagesSource_grp->Text = L"Images";
			// 
			// button11
			// 
			this->button11->Location = System::Drawing::Point(85, 227);
			this->button11->Name = L"button11";
			this->button11->Size = System::Drawing::Size(75, 23);
			this->button11->TabIndex = 57;
			this->button11->Text = L"S02/C3";
			this->button11->UseVisualStyleBackColor = true;
			this->button11->Click += gcnew System::EventHandler(this, &Form1::button11_Click_2);
			// 
			// button9
			// 
			this->button9->Location = System::Drawing::Point(4, 198);
			this->button9->Name = L"button9";
			this->button9->Size = System::Drawing::Size(75, 23);
			this->button9->TabIndex = 56;
			this->button9->Text = L"S4/C3";
			this->button9->UseVisualStyleBackColor = true;
			this->button9->Click += gcnew System::EventHandler(this, &Form1::button9_Click_1);
			// 
			// button8
			// 
			this->button8->Location = System::Drawing::Point(85, 197);
			this->button8->Name = L"button8";
			this->button8->Size = System::Drawing::Size(75, 23);
			this->button8->TabIndex = 55;
			this->button8->Text = L"S5/C3";
			this->button8->UseVisualStyleBackColor = true;
			this->button8->Click += gcnew System::EventHandler(this, &Form1::button8_Click_1);
			// 
			// button7
			// 
			this->button7->Location = System::Drawing::Point(166, 197);
			this->button7->Name = L"button7";
			this->button7->Size = System::Drawing::Size(75, 23);
			this->button7->TabIndex = 54;
			this->button7->Text = L"S6/C3";
			this->button7->UseVisualStyleBackColor = true;
			this->button7->Click += gcnew System::EventHandler(this, &Form1::button7_Click_1);
			// 
			// button6
			// 
			this->button6->Location = System::Drawing::Point(166, 168);
			this->button6->Name = L"button6";
			this->button6->Size = System::Drawing::Size(75, 23);
			this->button6->TabIndex = 53;
			this->button6->Text = L"S3/C3";
			this->button6->UseVisualStyleBackColor = true;
			this->button6->Click += gcnew System::EventHandler(this, &Form1::button6_Click);
			// 
			// button5
			// 
			this->button5->Location = System::Drawing::Point(85, 170);
			this->button5->Name = L"button5";
			this->button5->Size = System::Drawing::Size(75, 23);
			this->button5->TabIndex = 52;
			this->button5->Text = L"S2/C3";
			this->button5->UseVisualStyleBackColor = true;
			this->button5->Click += gcnew System::EventHandler(this, &Form1::button5_Click);
			// 
			// button4
			// 
			this->button4->Location = System::Drawing::Point(4, 227);
			this->button4->Name = L"button4";
			this->button4->Size = System::Drawing::Size(75, 23);
			this->button4->TabIndex = 51;
			this->button4->Text = L"S7/C3";
			this->button4->UseVisualStyleBackColor = true;
			this->button4->Click += gcnew System::EventHandler(this, &Form1::button4_Click_1);
			// 
			// button3
			// 
			this->button3->Location = System::Drawing::Point(4, 170);
			this->button3->Name = L"button3";
			this->button3->Size = System::Drawing::Size(75, 23);
			this->button3->TabIndex = 50;
			this->button3->Text = L"S1/C3";
			this->button3->UseVisualStyleBackColor = true;
			this->button3->Click += gcnew System::EventHandler(this, &Form1::button3_Click);
			// 
			// label_startfromdummy
			// 
			this->label_startfromdummy->AutoSize = true;
			this->label_startfromdummy->Location = System::Drawing::Point(9, 107);
			this->label_startfromdummy->Name = L"label_startfromdummy";
			this->label_startfromdummy->Size = System::Drawing::Size(50, 13);
			this->label_startfromdummy->TabIndex = 49;
			this->label_startfromdummy->Text = L"start from";
			// 
			// startfrom_txt
			// 
			this->startfrom_txt->Location = System::Drawing::Point(191, 110);
			this->startfrom_txt->Name = L"startfrom_txt";
			this->startfrom_txt->Size = System::Drawing::Size(66, 20);
			this->startfrom_txt->TabIndex = 48;
			// 
			// readfromimagesfps_txt
			// 
			this->readfromimagesfps_txt->Location = System::Drawing::Point(191, 86);
			this->readfromimagesfps_txt->Name = L"readfromimagesfps_txt";
			this->readfromimagesfps_txt->Size = System::Drawing::Size(66, 20);
			this->readfromimagesfps_txt->TabIndex = 47;
			this->readfromimagesfps_txt->Text = L"24";
			// 
			// readfromimagesfps_lbl
			// 
			this->readfromimagesfps_lbl->AutoSize = true;
			this->readfromimagesfps_lbl->Location = System::Drawing::Point(9, 87);
			this->readfromimagesfps_lbl->Name = L"readfromimagesfps_lbl";
			this->readfromimagesfps_lbl->Size = System::Drawing::Size(21, 13);
			this->readfromimagesfps_lbl->TabIndex = 46;
			this->readfromimagesfps_lbl->Text = L"fps";
			// 
			// LoadFromimages_btn
			// 
			this->LoadFromimages_btn->Location = System::Drawing::Point(171, 286);
			this->LoadFromimages_btn->Name = L"LoadFromimages_btn";
			this->LoadFromimages_btn->Size = System::Drawing::Size(86, 23);
			this->LoadFromimages_btn->TabIndex = 45;
			this->LoadFromimages_btn->Text = L"Load";
			this->LoadFromimages_btn->UseVisualStyleBackColor = true;
			this->LoadFromimages_btn->Click += gcnew System::EventHandler(this, &Form1::LoadFromimages_btn_Click);
			// 
			// LoadImagesNumberOfDigits_txt
			// 
			this->LoadImagesNumberOfDigits_txt->Location = System::Drawing::Point(192, 65);
			this->LoadImagesNumberOfDigits_txt->Name = L"LoadImagesNumberOfDigits_txt";
			this->LoadImagesNumberOfDigits_txt->Size = System::Drawing::Size(66, 20);
			this->LoadImagesNumberOfDigits_txt->TabIndex = 44;
			// 
			// ImageNumberOfDigits_lbl
			// 
			this->ImageNumberOfDigits_lbl->AutoSize = true;
			this->ImageNumberOfDigits_lbl->Location = System::Drawing::Point(9, 67);
			this->ImageNumberOfDigits_lbl->Name = L"ImageNumberOfDigits_lbl";
			this->ImageNumberOfDigits_lbl->Size = System::Drawing::Size(83, 13);
			this->ImageNumberOfDigits_lbl->TabIndex = 30;
			this->ImageNumberOfDigits_lbl->Text = L"Number of digits";
			// 
			// LoadImagesExtension_txt
			// 
			this->LoadImagesExtension_txt->Location = System::Drawing::Point(192, 42);
			this->LoadImagesExtension_txt->Name = L"LoadImagesExtension_txt";
			this->LoadImagesExtension_txt->Size = System::Drawing::Size(66, 20);
			this->LoadImagesExtension_txt->TabIndex = 43;
			// 
			// label2
			// 
			this->label2->AutoSize = true;
			this->label2->Location = System::Drawing::Point(9, 42);
			this->label2->Name = L"label2";
			this->label2->Size = System::Drawing::Size(93, 13);
			this->label2->TabIndex = 29;
			this->label2->Text = L"File extention (.***)";
			// 
			// video3_btn
			// 
			this->video3_btn->Location = System::Drawing::Point(4, 144);
			this->video3_btn->Name = L"video3_btn";
			this->video3_btn->Size = System::Drawing::Size(88, 23);
			this->video3_btn->TabIndex = 19;
			this->video3_btn->Text = L"BHV fight 1";
			this->video3_btn->UseVisualStyleBackColor = true;
			this->video3_btn->Click += gcnew System::EventHandler(this, &Form1::video3_btn_Click);
			// 
			// LoadImagesPath_txt
			// 
			this->LoadImagesPath_txt->Location = System::Drawing::Point(192, 19);
			this->LoadImagesPath_txt->Name = L"LoadImagesPath_txt";
			this->LoadImagesPath_txt->Size = System::Drawing::Size(66, 20);
			this->LoadImagesPath_txt->TabIndex = 42;
			// 
			// ImagesPath_lbl
			// 
			this->ImagesPath_lbl->AutoSize = true;
			this->ImagesPath_lbl->Location = System::Drawing::Point(6, 21);
			this->ImagesPath_lbl->Name = L"ImagesPath_lbl";
			this->ImagesPath_lbl->Size = System::Drawing::Size(180, 13);
			this->ImagesPath_lbl->TabIndex = 28;
			this->ImagesPath_lbl->Text = L"Images Path (include common prefix)";
			// 
			// video8_btn
			// 
			this->video8_btn->Location = System::Drawing::Point(98, 144);
			this->video8_btn->Name = L"video8_btn";
			this->video8_btn->Size = System::Drawing::Size(75, 23);
			this->video8_btn->TabIndex = 24;
			this->video8_btn->Text = L"BHV fight 2";
			this->video8_btn->UseVisualStyleBackColor = true;
			this->video8_btn->Click += gcnew System::EventHandler(this, &Form1::video8_btn_Click);
			// 
			// activatedOptions_grp
			// 
			this->activatedOptions_grp->Controls->Add(this->useHistDifference_chk);
			this->activatedOptions_grp->Controls->Add(this->ghost_chk);
			this->activatedOptions_grp->Controls->Add(this->BehaviourRecognition_chk);
			this->activatedOptions_grp->Controls->Add(this->traintest_chk);
			this->activatedOptions_grp->Controls->Add(this->useMorphology_chk);
			this->activatedOptions_grp->Controls->Add(this->smooth_chk);
			this->activatedOptions_grp->Controls->Add(this->homomorphic_chk);
			this->activatedOptions_grp->Controls->Add(this->trackaftertrainingonly_chk);
			this->activatedOptions_grp->Controls->Add(this->trackerEnables_chk);
			this->activatedOptions_grp->Controls->Add(this->minArea_chk);
			this->activatedOptions_grp->Controls->Add(this->disableUpdate_chk);
			this->activatedOptions_grp->Controls->Add(this->fmin_chk);
			this->activatedOptions_grp->Controls->Add(this->NFPP_chk);
			this->activatedOptions_grp->Enabled = false;
			this->activatedOptions_grp->Location = System::Drawing::Point(308, 188);
			this->activatedOptions_grp->Name = L"activatedOptions_grp";
			this->activatedOptions_grp->Size = System::Drawing::Size(200, 258);
			this->activatedOptions_grp->TabIndex = 12;
			this->activatedOptions_grp->TabStop = false;
			this->activatedOptions_grp->Text = L"Activated Options";
			this->activatedOptions_grp->Enter += gcnew System::EventHandler(this, &Form1::groupBox6_Enter);
			// 
			// useHistDifference_chk
			// 
			this->useHistDifference_chk->AutoSize = true;
			this->useHistDifference_chk->Location = System::Drawing::Point(5, 234);
			this->useHistDifference_chk->Name = L"useHistDifference_chk";
			this->useHistDifference_chk->Size = System::Drawing::Size(125, 17);
			this->useHistDifference_chk->TabIndex = 37;
			this->useHistDifference_chk->Text = L"Histogram Difference";
			this->useHistDifference_chk->UseVisualStyleBackColor = true;
			this->useHistDifference_chk->CheckedChanged += gcnew System::EventHandler(this, &Form1::useHistDifference_chk_CheckedChanged);
			// 
			// ghost_chk
			// 
			this->ghost_chk->AutoSize = true;
			this->ghost_chk->Location = System::Drawing::Point(5, 216);
			this->ghost_chk->Name = L"ghost_chk";
			this->ghost_chk->Size = System::Drawing::Size(99, 17);
			this->ghost_chk->TabIndex = 36;
			this->ghost_chk->Text = L"Ghost Removal";
			this->ghost_chk->UseVisualStyleBackColor = true;
			this->ghost_chk->CheckedChanged += gcnew System::EventHandler(this, &Form1::ghost_chk_CheckedChanged);
			// 
			// BehaviourRecognition_chk
			// 
			this->BehaviourRecognition_chk->AutoSize = true;
			this->BehaviourRecognition_chk->Location = System::Drawing::Point(5, 200);
			this->BehaviourRecognition_chk->Name = L"BehaviourRecognition_chk";
			this->BehaviourRecognition_chk->Size = System::Drawing::Size(134, 17);
			this->BehaviourRecognition_chk->TabIndex = 35;
			this->BehaviourRecognition_chk->Text = L"Behaviour Recognition";
			this->BehaviourRecognition_chk->UseVisualStyleBackColor = true;
			this->BehaviourRecognition_chk->CheckedChanged += gcnew System::EventHandler(this, &Form1::BehaviourRecognition_chk_CheckedChanged);
			// 
			// traintest_chk
			// 
			this->traintest_chk->AutoSize = true;
			this->traintest_chk->Location = System::Drawing::Point(5, 182);
			this->traintest_chk->Name = L"traintest_chk";
			this->traintest_chk->Size = System::Drawing::Size(150, 17);
			this->traintest_chk->TabIndex = 34;
			this->traintest_chk->Text = L"Seperate Training/Testing";
			this->traintest_chk->UseVisualStyleBackColor = true;
			this->traintest_chk->CheckedChanged += gcnew System::EventHandler(this, &Form1::traintest_chk_CheckedChanged);
			// 
			// useMorphology_chk
			// 
			this->useMorphology_chk->AutoSize = true;
			this->useMorphology_chk->Enabled = false;
			this->useMorphology_chk->Location = System::Drawing::Point(5, 52);
			this->useMorphology_chk->Margin = System::Windows::Forms::Padding(2, 3, 2, 3);
			this->useMorphology_chk->Name = L"useMorphology_chk";
			this->useMorphology_chk->Size = System::Drawing::Size(140, 17);
			this->useMorphology_chk->TabIndex = 33;
			this->useMorphology_chk->Text = L"Use Closing Morphology";
			this->useMorphology_chk->UseVisualStyleBackColor = true;
			this->useMorphology_chk->CheckedChanged += gcnew System::EventHandler(this, &Form1::useMorphology_chk_CheckedChanged);
			// 
			// smooth_chk
			// 
			this->smooth_chk->AutoSize = true;
			this->smooth_chk->Location = System::Drawing::Point(5, 165);
			this->smooth_chk->Name = L"smooth_chk";
			this->smooth_chk->Size = System::Drawing::Size(109, 17);
			this->smooth_chk->TabIndex = 32;
			this->smooth_chk->Text = L"Gaussian Smooth";
			this->smooth_chk->UseVisualStyleBackColor = true;
			this->smooth_chk->CheckedChanged += gcnew System::EventHandler(this, &Form1::smooth_chk_CheckedChanged);
			// 
			// homomorphic_chk
			// 
			this->homomorphic_chk->AutoSize = true;
			this->homomorphic_chk->Enabled = false;
			this->homomorphic_chk->Location = System::Drawing::Point(5, 147);
			this->homomorphic_chk->Name = L"homomorphic_chk";
			this->homomorphic_chk->Size = System::Drawing::Size(111, 17);
			this->homomorphic_chk->TabIndex = 31;
			this->homomorphic_chk->Text = L"homomorphic filter";
			this->homomorphic_chk->UseVisualStyleBackColor = true;
			this->homomorphic_chk->CheckedChanged += gcnew System::EventHandler(this, &Form1::homomorphic_chk_CheckedChanged);
			// 
			// trackaftertrainingonly_chk
			// 
			this->trackaftertrainingonly_chk->AutoSize = true;
			this->trackaftertrainingonly_chk->Enabled = false;
			this->trackaftertrainingonly_chk->Location = System::Drawing::Point(5, 128);
			this->trackaftertrainingonly_chk->Name = L"trackaftertrainingonly_chk";
			this->trackaftertrainingonly_chk->Size = System::Drawing::Size(137, 17);
			this->trackaftertrainingonly_chk->TabIndex = 30;
			this->trackaftertrainingonly_chk->Text = L"Track after training only";
			this->trackaftertrainingonly_chk->UseVisualStyleBackColor = true;
			this->trackaftertrainingonly_chk->CheckedChanged += gcnew System::EventHandler(this, &Form1::trackaftertrainingonly_chk_CheckedChanged);
			// 
			// trackerEnables_chk
			// 
			this->trackerEnables_chk->AutoSize = true;
			this->trackerEnables_chk->Location = System::Drawing::Point(5, 109);
			this->trackerEnables_chk->Name = L"trackerEnables_chk";
			this->trackerEnables_chk->Size = System::Drawing::Size(63, 17);
			this->trackerEnables_chk->TabIndex = 29;
			this->trackerEnables_chk->Text = L"Tracker";
			this->trackerEnables_chk->UseVisualStyleBackColor = true;
			this->trackerEnables_chk->CheckedChanged += gcnew System::EventHandler(this, &Form1::trackerEnables_chk_CheckedChanged);
			// 
			// minArea_chk
			// 
			this->minArea_chk->AutoSize = true;
			this->minArea_chk->Enabled = false;
			this->minArea_chk->Location = System::Drawing::Point(5, 35);
			this->minArea_chk->Margin = System::Windows::Forms::Padding(2, 3, 2, 3);
			this->minArea_chk->Name = L"minArea_chk";
			this->minArea_chk->Size = System::Drawing::Size(113, 17);
			this->minArea_chk->TabIndex = 28;
			this->minArea_chk->Text = L"Min Area algorithm";
			this->minArea_chk->UseVisualStyleBackColor = true;
			this->minArea_chk->CheckedChanged += gcnew System::EventHandler(this, &Form1::minArea_chk_CheckedChanged);
			// 
			// disableUpdate_chk
			// 
			this->disableUpdate_chk->AutoSize = true;
			this->disableUpdate_chk->Location = System::Drawing::Point(5, 70);
			this->disableUpdate_chk->Margin = System::Windows::Forms::Padding(2, 3, 2, 3);
			this->disableUpdate_chk->Name = L"disableUpdate_chk";
			this->disableUpdate_chk->Size = System::Drawing::Size(183, 17);
			this->disableUpdate_chk->TabIndex = 24;
			this->disableUpdate_chk->Text = L"Disabel Update during test period";
			this->disableUpdate_chk->UseVisualStyleBackColor = true;
			this->disableUpdate_chk->CheckedChanged += gcnew System::EventHandler(this, &Form1::disableUpdate_chk_CheckedChanged);
			// 
			// fmin_chk
			// 
			this->fmin_chk->AutoSize = true;
			this->fmin_chk->Location = System::Drawing::Point(5, 89);
			this->fmin_chk->Margin = System::Windows::Forms::Padding(2, 3, 2, 3);
			this->fmin_chk->Name = L"fmin_chk";
			this->fmin_chk->Size = System::Drawing::Size(89, 17);
			this->fmin_chk->TabIndex = 26;
			this->fmin_chk->Text = L"f-min enabled";
			this->fmin_chk->UseVisualStyleBackColor = true;
			this->fmin_chk->CheckedChanged += gcnew System::EventHandler(this, &Form1::fmin_chk_CheckedChanged);
			// 
			// NFPP_chk
			// 
			this->NFPP_chk->AutoSize = true;
			this->NFPP_chk->Location = System::Drawing::Point(5, 19);
			this->NFPP_chk->Margin = System::Windows::Forms::Padding(2, 3, 2, 3);
			this->NFPP_chk->Name = L"NFPP_chk";
			this->NFPP_chk->Size = System::Drawing::Size(99, 17);
			this->NFPP_chk->TabIndex = 27;
			this->NFPP_chk->Text = L"NFPP algorithm";
			this->NFPP_chk->UseVisualStyleBackColor = true;
			this->NFPP_chk->CheckedChanged += gcnew System::EventHandler(this, &Form1::NFPP_chk_CheckedChanged);
			// 
			// timeOrFrames_grp
			// 
			this->timeOrFrames_grp->Controls->Add(this->inTermsOfFrames_radio);
			this->timeOrFrames_grp->Controls->Add(this->inTermsOfTime_radio);
			this->timeOrFrames_grp->Enabled = false;
			this->timeOrFrames_grp->Location = System::Drawing::Point(543, 478);
			this->timeOrFrames_grp->Margin = System::Windows::Forms::Padding(2);
			this->timeOrFrames_grp->Name = L"timeOrFrames_grp";
			this->timeOrFrames_grp->Padding = System::Windows::Forms::Padding(2);
			this->timeOrFrames_grp->Size = System::Drawing::Size(153, 55);
			this->timeOrFrames_grp->TabIndex = 31;
			this->timeOrFrames_grp->TabStop = false;
			this->timeOrFrames_grp->Text = L"In Terms Of";
			// 
			// inTermsOfFrames_radio
			// 
			this->inTermsOfFrames_radio->AutoSize = true;
			this->inTermsOfFrames_radio->Location = System::Drawing::Point(11, 33);
			this->inTermsOfFrames_radio->Margin = System::Windows::Forms::Padding(2);
			this->inTermsOfFrames_radio->Name = L"inTermsOfFrames_radio";
			this->inTermsOfFrames_radio->Size = System::Drawing::Size(56, 17);
			this->inTermsOfFrames_radio->TabIndex = 1;
			this->inTermsOfFrames_radio->Text = L"frames";
			this->inTermsOfFrames_radio->UseVisualStyleBackColor = true;
			this->inTermsOfFrames_radio->CheckedChanged += gcnew System::EventHandler(this, &Form1::inTermsOfFrames_radio_CheckedChanged);
			// 
			// inTermsOfTime_radio
			// 
			this->inTermsOfTime_radio->AutoSize = true;
			this->inTermsOfTime_radio->Checked = true;
			this->inTermsOfTime_radio->Location = System::Drawing::Point(11, 16);
			this->inTermsOfTime_radio->Margin = System::Windows::Forms::Padding(2);
			this->inTermsOfTime_radio->Name = L"inTermsOfTime_radio";
			this->inTermsOfTime_radio->Size = System::Drawing::Size(65, 17);
			this->inTermsOfTime_radio->TabIndex = 0;
			this->inTermsOfTime_radio->TabStop = true;
			this->inTermsOfTime_radio->Text = L"seconds";
			this->inTermsOfTime_radio->UseVisualStyleBackColor = true;
			this->inTermsOfTime_radio->CheckedChanged += gcnew System::EventHandler(this, &Form1::inTermsOfTime_radio_CheckedChanged);
			// 
			// Codebook_grp
			// 
			this->Codebook_grp->Controls->Add(this->area_lbl);
			this->Codebook_grp->Controls->Add(this->region_lbl);
			this->Codebook_grp->Controls->Add(this->epsilon2_lbl);
			this->Codebook_grp->Controls->Add(this->delta2_lbl);
			this->Codebook_grp->Controls->Add(this->CB_color_lbl);
			this->Codebook_grp->Controls->Add(this->pxl_color_lbl);
			this->Codebook_grp->Controls->Add(this->map_prev_btn);
			this->Codebook_grp->Controls->Add(this->map_next_btn);
			this->Codebook_grp->Controls->Add(this->showmap_btn);
			this->Codebook_grp->Controls->Add(this->I_lbl);
			this->Codebook_grp->Controls->Add(this->Ihigh_lbl);
			this->Codebook_grp->Controls->Add(this->Ilow_lbl);
			this->Codebook_grp->Controls->Add(this->delta_lbl);
			this->Codebook_grp->Controls->Add(this->B_CB_lbl);
			this->Codebook_grp->Controls->Add(this->G_CB_lbl);
			this->Codebook_grp->Controls->Add(this->R_CB_lbl);
			this->Codebook_grp->Controls->Add(this->epsilon_lbl);
			this->Codebook_grp->Controls->Add(this->pxlStatus_lbl);
			this->Codebook_grp->Controls->Add(this->B_lbl);
			this->Codebook_grp->Controls->Add(this->G_lbl);
			this->Codebook_grp->Controls->Add(this->R_lbl);
			this->Codebook_grp->Controls->Add(this->maxCB_lbl);
			this->Codebook_grp->Controls->Add(this->pxlNum_lbl);
			this->Codebook_grp->Controls->Add(this->prevCB_lbl);
			this->Codebook_grp->Controls->Add(this->nextCB_lbl);
			this->Codebook_grp->Controls->Add(this->firstacc_lbl);
			this->Codebook_grp->Controls->Add(this->lastacc_lbl);
			this->Codebook_grp->Controls->Add(this->MNRL_lbl);
			this->Codebook_grp->Controls->Add(this->freq_lbl);
			this->Codebook_grp->Controls->Add(this->Imin_lbl);
			this->Codebook_grp->Controls->Add(this->Imax_lbl);
			this->Codebook_grp->Controls->Add(this->codebookNumber_lbl);
			this->Codebook_grp->Enabled = false;
			this->Codebook_grp->Location = System::Drawing::Point(702, 293);
			this->Codebook_grp->Name = L"Codebook_grp";
			this->Codebook_grp->Size = System::Drawing::Size(451, 211);
			this->Codebook_grp->TabIndex = 14;
			this->Codebook_grp->TabStop = false;
			this->Codebook_grp->Text = L"Codebook";
			this->Codebook_grp->Enter += gcnew System::EventHandler(this, &Form1::Codebook_grp_Enter);
			// 
			// area_lbl
			// 
			this->area_lbl->AutoSize = true;
			this->area_lbl->Location = System::Drawing::Point(110, 176);
			this->area_lbl->Margin = System::Windows::Forms::Padding(2, 0, 2, 0);
			this->area_lbl->Name = L"area_lbl";
			this->area_lbl->Size = System::Drawing::Size(32, 13);
			this->area_lbl->TabIndex = 32;
			this->area_lbl->Text = L"Area:";
			// 
			// region_lbl
			// 
			this->region_lbl->AutoSize = true;
			this->region_lbl->Location = System::Drawing::Point(34, 176);
			this->region_lbl->Margin = System::Windows::Forms::Padding(2, 0, 2, 0);
			this->region_lbl->Name = L"region_lbl";
			this->region_lbl->Size = System::Drawing::Size(44, 13);
			this->region_lbl->TabIndex = 31;
			this->region_lbl->Text = L"Region:";
			// 
			// epsilon2_lbl
			// 
			this->epsilon2_lbl->AutoSize = true;
			this->epsilon2_lbl->Location = System::Drawing::Point(367, 119);
			this->epsilon2_lbl->Name = L"epsilon2_lbl";
			this->epsilon2_lbl->Size = System::Drawing::Size(58, 13);
			this->epsilon2_lbl->TabIndex = 30;
			this->epsilon2_lbl->Text = L"epsilon2 = ";
			// 
			// delta2_lbl
			// 
			this->delta2_lbl->AutoSize = true;
			this->delta2_lbl->Location = System::Drawing::Point(281, 119);
			this->delta2_lbl->Name = L"delta2_lbl";
			this->delta2_lbl->Size = System::Drawing::Size(48, 13);
			this->delta2_lbl->TabIndex = 29;
			this->delta2_lbl->Text = L"delta2 = ";
			// 
			// CB_color_lbl
			// 
			this->CB_color_lbl->AutoSize = true;
			this->CB_color_lbl->ForeColor = System::Drawing::SystemColors::ButtonFace;
			this->CB_color_lbl->Location = System::Drawing::Point(224, 99);
			this->CB_color_lbl->Margin = System::Windows::Forms::Padding(2, 0, 2, 0);
			this->CB_color_lbl->Name = L"CB_color_lbl";
			this->CB_color_lbl->Size = System::Drawing::Size(30, 13);
			this->CB_color_lbl->TabIndex = 28;
			this->CB_color_lbl->Text = L"color";
			this->CB_color_lbl->Click += gcnew System::EventHandler(this, &Form1::CB_color_lbl_Click);
			// 
			// pxl_color_lbl
			// 
			this->pxl_color_lbl->AutoSize = true;
			this->pxl_color_lbl->ForeColor = System::Drawing::SystemColors::ButtonFace;
			this->pxl_color_lbl->Location = System::Drawing::Point(224, 115);
			this->pxl_color_lbl->Margin = System::Windows::Forms::Padding(2, 0, 2, 0);
			this->pxl_color_lbl->Name = L"pxl_color_lbl";
			this->pxl_color_lbl->Size = System::Drawing::Size(30, 13);
			this->pxl_color_lbl->TabIndex = 27;
			this->pxl_color_lbl->Text = L"color";
			// 
			// map_prev_btn
			// 
			this->map_prev_btn->Location = System::Drawing::Point(297, 154);
			this->map_prev_btn->Margin = System::Windows::Forms::Padding(2, 3, 2, 3);
			this->map_prev_btn->Name = L"map_prev_btn";
			this->map_prev_btn->Size = System::Drawing::Size(19, 24);
			this->map_prev_btn->TabIndex = 26;
			this->map_prev_btn->Text = L"<";
			this->map_prev_btn->UseVisualStyleBackColor = true;
			this->map_prev_btn->Click += gcnew System::EventHandler(this, &Form1::map_prev_btn_Click);
			// 
			// map_next_btn
			// 
			this->map_next_btn->Location = System::Drawing::Point(427, 154);
			this->map_next_btn->Margin = System::Windows::Forms::Padding(2, 3, 2, 3);
			this->map_next_btn->Name = L"map_next_btn";
			this->map_next_btn->Size = System::Drawing::Size(19, 24);
			this->map_next_btn->TabIndex = 25;
			this->map_next_btn->Text = L">";
			this->map_next_btn->UseVisualStyleBackColor = true;
			this->map_next_btn->Click += gcnew System::EventHandler(this, &Form1::map_next_btn_Click);
			// 
			// showmap_btn
			// 
			this->showmap_btn->Location = System::Drawing::Point(315, 154);
			this->showmap_btn->Name = L"showmap_btn";
			this->showmap_btn->Size = System::Drawing::Size(115, 26);
			this->showmap_btn->TabIndex = 24;
			this->showmap_btn->Text = L"Show Map CB 0";
			this->showmap_btn->UseVisualStyleBackColor = true;
			this->showmap_btn->Click += gcnew System::EventHandler(this, &Form1::showmap_btn_Click);
			// 
			// I_lbl
			// 
			this->I_lbl->AutoSize = true;
			this->I_lbl->Location = System::Drawing::Point(89, 140);
			this->I_lbl->Name = L"I_lbl";
			this->I_lbl->Size = System::Drawing::Size(22, 13);
			this->I_lbl->TabIndex = 23;
			this->I_lbl->Text = L"I = ";
			this->I_lbl->Click += gcnew System::EventHandler(this, &Form1::I_lbl_Click);
			// 
			// Ihigh_lbl
			// 
			this->Ihigh_lbl->AutoSize = true;
			this->Ihigh_lbl->Location = System::Drawing::Point(161, 140);
			this->Ihigh_lbl->Name = L"Ihigh_lbl";
			this->Ihigh_lbl->Size = System::Drawing::Size(42, 13);
			this->Ihigh_lbl->TabIndex = 22;
			this->Ihigh_lbl->Text = L"Ihigh = ";
			this->Ihigh_lbl->Click += gcnew System::EventHandler(this, &Form1::Ihigh_lbl_Click);
			// 
			// Ilow_lbl
			// 
			this->Ilow_lbl->AutoSize = true;
			this->Ilow_lbl->Location = System::Drawing::Point(19, 140);
			this->Ilow_lbl->Name = L"Ilow_lbl";
			this->Ilow_lbl->Size = System::Drawing::Size(38, 13);
			this->Ilow_lbl->TabIndex = 21;
			this->Ilow_lbl->Text = L"Ilow = ";
			// 
			// delta_lbl
			// 
			this->delta_lbl->AutoSize = true;
			this->delta_lbl->Location = System::Drawing::Point(281, 104);
			this->delta_lbl->Name = L"delta_lbl";
			this->delta_lbl->Size = System::Drawing::Size(42, 13);
			this->delta_lbl->TabIndex = 20;
			this->delta_lbl->Text = L"delta = ";
			this->delta_lbl->Click += gcnew System::EventHandler(this, &Form1::delta_lbl_Click);
			// 
			// B_CB_lbl
			// 
			this->B_CB_lbl->AutoSize = true;
			this->B_CB_lbl->Location = System::Drawing::Point(161, 102);
			this->B_CB_lbl->Name = L"B_CB_lbl";
			this->B_CB_lbl->Size = System::Drawing::Size(46, 13);
			this->B_CB_lbl->TabIndex = 18;
			this->B_CB_lbl->Text = L"B_CB = ";
			this->B_CB_lbl->Click += gcnew System::EventHandler(this, &Form1::B_CB_lbl_Click);
			// 
			// G_CB_lbl
			// 
			this->G_CB_lbl->AutoSize = true;
			this->G_CB_lbl->Location = System::Drawing::Point(89, 101);
			this->G_CB_lbl->Name = L"G_CB_lbl";
			this->G_CB_lbl->Size = System::Drawing::Size(47, 13);
			this->G_CB_lbl->TabIndex = 17;
			this->G_CB_lbl->Text = L"G_CB = ";
			// 
			// R_CB_lbl
			// 
			this->R_CB_lbl->AutoSize = true;
			this->R_CB_lbl->Location = System::Drawing::Point(19, 99);
			this->R_CB_lbl->Name = L"R_CB_lbl";
			this->R_CB_lbl->Size = System::Drawing::Size(47, 13);
			this->R_CB_lbl->TabIndex = 16;
			this->R_CB_lbl->Text = L"R_CB = ";
			// 
			// epsilon_lbl
			// 
			this->epsilon_lbl->AutoSize = true;
			this->epsilon_lbl->Location = System::Drawing::Point(367, 101);
			this->epsilon_lbl->Name = L"epsilon_lbl";
			this->epsilon_lbl->Size = System::Drawing::Size(52, 13);
			this->epsilon_lbl->TabIndex = 15;
			this->epsilon_lbl->Text = L"epsilon = ";
			// 
			// pxlStatus_lbl
			// 
			this->pxlStatus_lbl->AutoSize = true;
			this->pxlStatus_lbl->Location = System::Drawing::Point(320, 48);
			this->pxlStatus_lbl->Name = L"pxlStatus_lbl";
			this->pxlStatus_lbl->Size = System::Drawing::Size(40, 13);
			this->pxlStatus_lbl->TabIndex = 14;
			this->pxlStatus_lbl->Text = L"Status:";
			// 
			// B_lbl
			// 
			this->B_lbl->AutoSize = true;
			this->B_lbl->Location = System::Drawing::Point(161, 115);
			this->B_lbl->Name = L"B_lbl";
			this->B_lbl->Size = System::Drawing::Size(26, 13);
			this->B_lbl->TabIndex = 13;
			this->B_lbl->Text = L"B = ";
			// 
			// G_lbl
			// 
			this->G_lbl->AutoSize = true;
			this->G_lbl->Location = System::Drawing::Point(89, 115);
			this->G_lbl->Name = L"G_lbl";
			this->G_lbl->Size = System::Drawing::Size(27, 13);
			this->G_lbl->TabIndex = 12;
			this->G_lbl->Text = L"G = ";
			// 
			// R_lbl
			// 
			this->R_lbl->AutoSize = true;
			this->R_lbl->Location = System::Drawing::Point(19, 115);
			this->R_lbl->Name = L"R_lbl";
			this->R_lbl->Size = System::Drawing::Size(27, 13);
			this->R_lbl->TabIndex = 11;
			this->R_lbl->Text = L"R = ";
			// 
			// maxCB_lbl
			// 
			this->maxCB_lbl->AutoSize = true;
			this->maxCB_lbl->Location = System::Drawing::Point(161, 28);
			this->maxCB_lbl->Name = L"maxCB_lbl";
			this->maxCB_lbl->Size = System::Drawing::Size(45, 13);
			this->maxCB_lbl->TabIndex = 10;
			this->maxCB_lbl->Text = L"Max = 0";
			// 
			// pxlNum_lbl
			// 
			this->pxlNum_lbl->AutoSize = true;
			this->pxlNum_lbl->Location = System::Drawing::Point(320, 29);
			this->pxlNum_lbl->Name = L"pxlNum_lbl";
			this->pxlNum_lbl->Size = System::Drawing::Size(94, 13);
			this->pxlNum_lbl->TabIndex = 9;
			this->pxlNum_lbl->Text = L"Pixel Coordinates: ";
			this->pxlNum_lbl->Click += gcnew System::EventHandler(this, &Form1::pxlNum_lbl_Click);
			// 
			// prevCB_lbl
			// 
			this->prevCB_lbl->Location = System::Drawing::Point(77, 23);
			this->prevCB_lbl->Name = L"prevCB_lbl";
			this->prevCB_lbl->Size = System::Drawing::Size(30, 20);
			this->prevCB_lbl->TabIndex = 8;
			this->prevCB_lbl->Text = L"<";
			this->prevCB_lbl->UseVisualStyleBackColor = true;
			this->prevCB_lbl->Click += gcnew System::EventHandler(this, &Form1::prevCB_lbl_Click);
			// 
			// nextCB_lbl
			// 
			this->nextCB_lbl->Location = System::Drawing::Point(113, 23);
			this->nextCB_lbl->Name = L"nextCB_lbl";
			this->nextCB_lbl->Size = System::Drawing::Size(30, 20);
			this->nextCB_lbl->TabIndex = 7;
			this->nextCB_lbl->Text = L">";
			this->nextCB_lbl->UseVisualStyleBackColor = true;
			this->nextCB_lbl->Click += gcnew System::EventHandler(this, &Form1::nextCB_lbl_Click);
			// 
			// firstacc_lbl
			// 
			this->firstacc_lbl->AutoSize = true;
			this->firstacc_lbl->Location = System::Drawing::Point(210, 49);
			this->firstacc_lbl->Name = L"firstacc_lbl";
			this->firstacc_lbl->Size = System::Drawing::Size(54, 13);
			this->firstacc_lbl->TabIndex = 6;
			this->firstacc_lbl->Text = L"1st acc = ";
			// 
			// lastacc_lbl
			// 
			this->lastacc_lbl->AutoSize = true;
			this->lastacc_lbl->Location = System::Drawing::Point(210, 71);
			this->lastacc_lbl->Name = L"lastacc_lbl";
			this->lastacc_lbl->Size = System::Drawing::Size(56, 13);
			this->lastacc_lbl->TabIndex = 5;
			this->lastacc_lbl->Text = L"last acc = ";
			this->lastacc_lbl->Click += gcnew System::EventHandler(this, &Form1::lastacc_lbl_Click);
			// 
			// MNRL_lbl
			// 
			this->MNRL_lbl->AutoSize = true;
			this->MNRL_lbl->Location = System::Drawing::Point(114, 71);
			this->MNRL_lbl->Name = L"MNRL_lbl";
			this->MNRL_lbl->Size = System::Drawing::Size(50, 13);
			this->MNRL_lbl->TabIndex = 4;
			this->MNRL_lbl->Text = L"MNRL = ";
			// 
			// freq_lbl
			// 
			this->freq_lbl->AutoSize = true;
			this->freq_lbl->Location = System::Drawing::Point(114, 49);
			this->freq_lbl->Name = L"freq_lbl";
			this->freq_lbl->Size = System::Drawing::Size(22, 13);
			this->freq_lbl->TabIndex = 3;
			this->freq_lbl->Text = L"f = ";
			this->freq_lbl->TextAlign = System::Drawing::ContentAlignment::TopCenter;
			this->freq_lbl->Click += gcnew System::EventHandler(this, &Form1::freq_lbl_Click);
			// 
			// Imin_lbl
			// 
			this->Imin_lbl->AutoSize = true;
			this->Imin_lbl->Location = System::Drawing::Point(18, 71);
			this->Imin_lbl->Name = L"Imin_lbl";
			this->Imin_lbl->Size = System::Drawing::Size(38, 13);
			this->Imin_lbl->TabIndex = 2;
			this->Imin_lbl->Text = L"Imin = ";
			// 
			// Imax_lbl
			// 
			this->Imax_lbl->AutoSize = true;
			this->Imax_lbl->Location = System::Drawing::Point(18, 49);
			this->Imax_lbl->Name = L"Imax_lbl";
			this->Imax_lbl->Size = System::Drawing::Size(41, 13);
			this->Imax_lbl->TabIndex = 1;
			this->Imax_lbl->Text = L"Imax = ";
			// 
			// codebookNumber_lbl
			// 
			this->codebookNumber_lbl->AutoSize = true;
			this->codebookNumber_lbl->Location = System::Drawing::Point(17, 28);
			this->codebookNumber_lbl->Name = L"codebookNumber_lbl";
			this->codebookNumber_lbl->Size = System::Drawing::Size(40, 13);
			this->codebookNumber_lbl->TabIndex = 0;
			this->codebookNumber_lbl->Text = L"CB # 0";
			// 
			// ProcTime_grp
			// 
			this->ProcTime_grp->Controls->Add(this->BehaviourTime_lbl);
			this->ProcTime_grp->Controls->Add(this->Resize_time_lbl);
			this->ProcTime_grp->Controls->Add(this->DisplayTime_lbl);
			this->ProcTime_grp->Controls->Add(this->LabConversionTimelbl);
			this->ProcTime_grp->Controls->Add(this->homomorphicTime_lbl);
			this->ProcTime_grp->Controls->Add(this->TrackerTime_lbl);
			this->ProcTime_grp->Controls->Add(this->totalTime_lbl);
			this->ProcTime_grp->Controls->Add(this->PP2_lbl);
			this->ProcTime_grp->Controls->Add(this->BGtime_lbl);
			this->ProcTime_grp->Controls->Add(this->updatetime_lbl);
			this->ProcTime_grp->Controls->Add(this->mdntime_lbl);
			this->ProcTime_grp->Location = System::Drawing::Point(896, 11);
			this->ProcTime_grp->Name = L"ProcTime_grp";
			this->ProcTime_grp->Size = System::Drawing::Size(188, 241);
			this->ProcTime_grp->TabIndex = 15;
			this->ProcTime_grp->TabStop = false;
			this->ProcTime_grp->Text = L"Processing Time";
			// 
			// BehaviourTime_lbl
			// 
			this->BehaviourTime_lbl->AutoSize = true;
			this->BehaviourTime_lbl->Location = System::Drawing::Point(11, 154);
			this->BehaviourTime_lbl->Name = L"BehaviourTime_lbl";
			this->BehaviourTime_lbl->Size = System::Drawing::Size(88, 13);
			this->BehaviourTime_lbl->TabIndex = 11;
			this->BehaviourTime_lbl->Text = L"behaviour time = ";
			// 
			// Resize_time_lbl
			// 
			this->Resize_time_lbl->AutoSize = true;
			this->Resize_time_lbl->Location = System::Drawing::Point(11, 25);
			this->Resize_time_lbl->Name = L"Resize_time_lbl";
			this->Resize_time_lbl->Size = System::Drawing::Size(104, 13);
			this->Resize_time_lbl->TabIndex = 10;
			this->Resize_time_lbl->Text = L"Frame resize Time = ";
			// 
			// DisplayTime_lbl
			// 
			this->DisplayTime_lbl->AutoSize = true;
			this->DisplayTime_lbl->Location = System::Drawing::Point(11, 191);
			this->DisplayTime_lbl->Name = L"DisplayTime_lbl";
			this->DisplayTime_lbl->Size = System::Drawing::Size(72, 13);
			this->DisplayTime_lbl->TabIndex = 9;
			this->DisplayTime_lbl->Text = L"Display time =";
			// 
			// LabConversionTimelbl
			// 
			this->LabConversionTimelbl->AutoSize = true;
			this->LabConversionTimelbl->Location = System::Drawing::Point(11, 43);
			this->LabConversionTimelbl->Name = L"LabConversionTimelbl";
			this->LabConversionTimelbl->Size = System::Drawing::Size(118, 13);
			this->LabConversionTimelbl->TabIndex = 8;
			this->LabConversionTimelbl->Text = L"Lab conversion Time = ";
			// 
			// homomorphicTime_lbl
			// 
			this->homomorphicTime_lbl->AutoSize = true;
			this->homomorphicTime_lbl->Location = System::Drawing::Point(11, 169);
			this->homomorphicTime_lbl->Name = L"homomorphicTime_lbl";
			this->homomorphicTime_lbl->Size = System::Drawing::Size(104, 13);
			this->homomorphicTime_lbl->TabIndex = 7;
			this->homomorphicTime_lbl->Text = L"homomorphic time = ";
			// 
			// TrackerTime_lbl
			// 
			this->TrackerTime_lbl->AutoSize = true;
			this->TrackerTime_lbl->Location = System::Drawing::Point(11, 134);
			this->TrackerTime_lbl->Name = L"TrackerTime_lbl";
			this->TrackerTime_lbl->Size = System::Drawing::Size(74, 13);
			this->TrackerTime_lbl->TabIndex = 6;
			this->TrackerTime_lbl->Text = L"tracker time = ";
			this->TrackerTime_lbl->Click += gcnew System::EventHandler(this, &Form1::TrackerTime_lbl_Click);
			// 
			// totalTime_lbl
			// 
			this->totalTime_lbl->AutoSize = true;
			this->totalTime_lbl->Location = System::Drawing::Point(9, 213);
			this->totalTime_lbl->Name = L"totalTime_lbl";
			this->totalTime_lbl->Size = System::Drawing::Size(43, 13);
			this->totalTime_lbl->TabIndex = 5;
			this->totalTime_lbl->Text = L"Total = ";
			// 
			// PP2_lbl
			// 
			this->PP2_lbl->AutoSize = true;
			this->PP2_lbl->Location = System::Drawing::Point(10, 116);
			this->PP2_lbl->Name = L"PP2_lbl";
			this->PP2_lbl->Size = System::Drawing::Size(95, 13);
			this->PP2_lbl->TabIndex = 4;
			this->PP2_lbl->Text = L"Post Processing = ";
			// 
			// BGtime_lbl
			// 
			this->BGtime_lbl->AutoSize = true;
			this->BGtime_lbl->Location = System::Drawing::Point(11, 98);
			this->BGtime_lbl->Name = L"BGtime_lbl";
			this->BGtime_lbl->Size = System::Drawing::Size(110, 13);
			this->BGtime_lbl->TabIndex = 2;
			this->BGtime_lbl->Text = L"BG processing time = ";
			// 
			// updatetime_lbl
			// 
			this->updatetime_lbl->AutoSize = true;
			this->updatetime_lbl->Location = System::Drawing::Point(10, 78);
			this->updatetime_lbl->Name = L"updatetime_lbl";
			this->updatetime_lbl->Size = System::Drawing::Size(128, 13);
			this->updatetime_lbl->TabIndex = 1;
			this->updatetime_lbl->Text = L"update processing time = ";
			// 
			// mdntime_lbl
			// 
			this->mdntime_lbl->AutoSize = true;
			this->mdntime_lbl->Location = System::Drawing::Point(10, 58);
			this->mdntime_lbl->Name = L"mdntime_lbl";
			this->mdntime_lbl->Size = System::Drawing::Size(129, 13);
			this->mdntime_lbl->TabIndex = 0;
			this->mdntime_lbl->Text = L"median processing time = ";
			// 
			// param_grp
			// 
			this->param_grp->Controls->Add(this->ResizeProcessedbydummy_lbl);
			this->param_grp->Controls->Add(this->maxAreaThreshold_lbl);
			this->param_grp->Controls->Add(this->maxAreaThreshold_txt);
			this->param_grp->Controls->Add(this->minAreaThreshold_chk);
			this->param_grp->Controls->Add(this->minAreaThreshold_txt);
			this->param_grp->Controls->Add(this->wantedSpeed_txt);
			this->param_grp->Controls->Add(this->Speeddummy_lbl);
			this->param_grp->Controls->Add(this->fmin_txt);
			this->param_grp->Controls->Add(this->fmin_lbl);
			this->param_grp->Controls->Add(this->DeltaS_lbl);
			this->param_grp->Controls->Add(this->DeltaS_txt);
			this->param_grp->Controls->Add(this->DeltaL_lbl);
			this->param_grp->Controls->Add(this->btn_setParam);
			this->param_grp->Controls->Add(this->DeltaC_lbl);
			this->param_grp->Controls->Add(this->DeltaL_txt);
			this->param_grp->Controls->Add(this->DeltaC_txt);
			this->param_grp->Controls->Add(this->DeltaE_txt);
			this->param_grp->Controls->Add(this->DeltaE_lbl);
			this->param_grp->Controls->Add(this->Lmax_txt);
			this->param_grp->Controls->Add(this->Lmax_lbl);
			this->param_grp->Controls->Add(this->TM_txt);
			this->param_grp->Controls->Add(this->TMdummy_lbl);
			this->param_grp->Controls->Add(this->resize_txt);
			this->param_grp->Controls->Add(this->N_txt);
			this->param_grp->Controls->Add(this->Ndummy_lbl);
			this->param_grp->Controls->Add(this->Resizebydummy_lbl);
			this->param_grp->Controls->Add(this->k2dummy_lbl);
			this->param_grp->Controls->Add(this->k2_txt);
			this->param_grp->Controls->Add(this->kdummy_lbl);
			this->param_grp->Controls->Add(this->betadummy_lbl);
			this->param_grp->Controls->Add(this->alphadummy_lbl);
			this->param_grp->Controls->Add(this->k_txt);
			this->param_grp->Controls->Add(this->beta_txt);
			this->param_grp->Controls->Add(this->alpha_txt);
			this->param_grp->Enabled = false;
			this->param_grp->Location = System::Drawing::Point(531, 26);
			this->param_grp->Name = L"param_grp";
			this->param_grp->Size = System::Drawing::Size(165, 428);
			this->param_grp->TabIndex = 16;
			this->param_grp->TabStop = false;
			this->param_grp->Text = L"Parameters";
			this->param_grp->Enter += gcnew System::EventHandler(this, &Form1::param_grp_Enter);
			// 
			// ResizeProcessedbydummy_lbl
			// 
			this->ResizeProcessedbydummy_lbl->AutoSize = true;
			this->ResizeProcessedbydummy_lbl->Checked = true;
			this->ResizeProcessedbydummy_lbl->CheckState = System::Windows::Forms::CheckState::Checked;
			this->ResizeProcessedbydummy_lbl->Location = System::Drawing::Point(13, 378);
			this->ResizeProcessedbydummy_lbl->Margin = System::Windows::Forms::Padding(2);
			this->ResizeProcessedbydummy_lbl->Name = L"ResizeProcessedbydummy_lbl";
			this->ResizeProcessedbydummy_lbl->Size = System::Drawing::Size(113, 17);
			this->ResizeProcessedbydummy_lbl->TabIndex = 41;
			this->ResizeProcessedbydummy_lbl->Text = L"Resize Processing";
			this->ResizeProcessedbydummy_lbl->UseVisualStyleBackColor = true;
			this->ResizeProcessedbydummy_lbl->CheckedChanged += gcnew System::EventHandler(this, &Form1::ResizeProcessedbydummy_lbl_CheckedChanged);
			// 
			// maxAreaThreshold_lbl
			// 
			this->maxAreaThreshold_lbl->AutoSize = true;
			this->maxAreaThreshold_lbl->Location = System::Drawing::Point(4, 335);
			this->maxAreaThreshold_lbl->Margin = System::Windows::Forms::Padding(2, 0, 2, 0);
			this->maxAreaThreshold_lbl->Name = L"maxAreaThreshold_lbl";
			this->maxAreaThreshold_lbl->Size = System::Drawing::Size(102, 13);
			this->maxAreaThreshold_lbl->TabIndex = 40;
			this->maxAreaThreshold_lbl->Text = L"Max Area Threshold";
			// 
			// maxAreaThreshold_txt
			// 
			this->maxAreaThreshold_txt->Enabled = false;
			this->maxAreaThreshold_txt->Location = System::Drawing::Point(105, 332);
			this->maxAreaThreshold_txt->Margin = System::Windows::Forms::Padding(2, 3, 2, 3);
			this->maxAreaThreshold_txt->Name = L"maxAreaThreshold_txt";
			this->maxAreaThreshold_txt->Size = System::Drawing::Size(31, 20);
			this->maxAreaThreshold_txt->TabIndex = 39;
			// 
			// minAreaThreshold_chk
			// 
			this->minAreaThreshold_chk->AutoSize = true;
			this->minAreaThreshold_chk->Location = System::Drawing::Point(4, 314);
			this->minAreaThreshold_chk->Margin = System::Windows::Forms::Padding(2, 0, 2, 0);
			this->minAreaThreshold_chk->Name = L"minAreaThreshold_chk";
			this->minAreaThreshold_chk->Size = System::Drawing::Size(99, 13);
			this->minAreaThreshold_chk->TabIndex = 36;
			this->minAreaThreshold_chk->Text = L"Min Area Threshold";
			// 
			// minAreaThreshold_txt
			// 
			this->minAreaThreshold_txt->Enabled = false;
			this->minAreaThreshold_txt->Location = System::Drawing::Point(105, 310);
			this->minAreaThreshold_txt->Margin = System::Windows::Forms::Padding(2, 3, 2, 3);
			this->minAreaThreshold_txt->Name = L"minAreaThreshold_txt";
			this->minAreaThreshold_txt->Size = System::Drawing::Size(31, 20);
			this->minAreaThreshold_txt->TabIndex = 35;
			// 
			// wantedSpeed_txt
			// 
			this->wantedSpeed_txt->Location = System::Drawing::Point(91, 289);
			this->wantedSpeed_txt->Margin = System::Windows::Forms::Padding(2, 3, 2, 3);
			this->wantedSpeed_txt->Name = L"wantedSpeed_txt";
			this->wantedSpeed_txt->Size = System::Drawing::Size(66, 20);
			this->wantedSpeed_txt->TabIndex = 34;
			this->wantedSpeed_txt->TextChanged += gcnew System::EventHandler(this, &Form1::wantedSpeed_txt_TextChanged_1);
			// 
			// Speeddummy_lbl
			// 
			this->Speeddummy_lbl->AutoSize = true;
			this->Speeddummy_lbl->Location = System::Drawing::Point(4, 290);
			this->Speeddummy_lbl->Margin = System::Windows::Forms::Padding(2, 0, 2, 0);
			this->Speeddummy_lbl->Name = L"Speeddummy_lbl";
			this->Speeddummy_lbl->Size = System::Drawing::Size(83, 13);
			this->Speeddummy_lbl->TabIndex = 33;
			this->Speeddummy_lbl->Text = L"Max Speed (ms)";
			// 
			// fmin_txt
			// 
			this->fmin_txt->Enabled = false;
			this->fmin_txt->Location = System::Drawing::Point(48, 159);
			this->fmin_txt->Name = L"fmin_txt";
			this->fmin_txt->Size = System::Drawing::Size(66, 20);
			this->fmin_txt->TabIndex = 24;
			this->fmin_txt->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &Form1::fmin_txt_KeyPress);
			// 
			// fmin_lbl
			// 
			this->fmin_lbl->AutoSize = true;
			this->fmin_lbl->Location = System::Drawing::Point(9, 162);
			this->fmin_lbl->Name = L"fmin_lbl";
			this->fmin_lbl->Size = System::Drawing::Size(29, 13);
			this->fmin_lbl->TabIndex = 23;
			this->fmin_lbl->Text = L"f min";
			// 
			// DeltaS_lbl
			// 
			this->DeltaS_lbl->AutoSize = true;
			this->DeltaS_lbl->Location = System::Drawing::Point(5, 267);
			this->DeltaS_lbl->Name = L"DeltaS_lbl";
			this->DeltaS_lbl->Size = System::Drawing::Size(42, 13);
			this->DeltaS_lbl->TabIndex = 22;
			this->DeltaS_lbl->Text = L"Delta S";
			// 
			// DeltaS_txt
			// 
			this->DeltaS_txt->Location = System::Drawing::Point(48, 267);
			this->DeltaS_txt->Name = L"DeltaS_txt";
			this->DeltaS_txt->Size = System::Drawing::Size(66, 20);
			this->DeltaS_txt->TabIndex = 21;
			this->DeltaS_txt->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &Form1::DeltaS_txt_KeyPress);
			// 
			// DeltaL_lbl
			// 
			this->DeltaL_lbl->AutoSize = true;
			this->DeltaL_lbl->Location = System::Drawing::Point(6, 247);
			this->DeltaL_lbl->Name = L"DeltaL_lbl";
			this->DeltaL_lbl->Size = System::Drawing::Size(41, 13);
			this->DeltaL_lbl->TabIndex = 20;
			this->DeltaL_lbl->Text = L"Delta L";
			// 
			// btn_setParam
			// 
			this->btn_setParam->Location = System::Drawing::Point(41, 396);
			this->btn_setParam->Margin = System::Windows::Forms::Padding(2, 3, 2, 3);
			this->btn_setParam->Name = L"btn_setParam";
			this->btn_setParam->Size = System::Drawing::Size(43, 23);
			this->btn_setParam->TabIndex = 16;
			this->btn_setParam->Text = L"Set";
			this->btn_setParam->UseVisualStyleBackColor = true;
			this->btn_setParam->Click += gcnew System::EventHandler(this, &Form1::btn_setParam_Click);
			// 
			// DeltaC_lbl
			// 
			this->DeltaC_lbl->AutoSize = true;
			this->DeltaC_lbl->Location = System::Drawing::Point(4, 227);
			this->DeltaC_lbl->Name = L"DeltaC_lbl";
			this->DeltaC_lbl->Size = System::Drawing::Size(42, 13);
			this->DeltaC_lbl->TabIndex = 19;
			this->DeltaC_lbl->Text = L"Delta C";
			// 
			// DeltaL_txt
			// 
			this->DeltaL_txt->Location = System::Drawing::Point(48, 247);
			this->DeltaL_txt->Name = L"DeltaL_txt";
			this->DeltaL_txt->Size = System::Drawing::Size(66, 20);
			this->DeltaL_txt->TabIndex = 18;
			this->DeltaL_txt->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &Form1::DeltaL_txt_KeyPress);
			// 
			// DeltaC_txt
			// 
			this->DeltaC_txt->Location = System::Drawing::Point(48, 224);
			this->DeltaC_txt->Name = L"DeltaC_txt";
			this->DeltaC_txt->Size = System::Drawing::Size(66, 20);
			this->DeltaC_txt->TabIndex = 17;
			this->DeltaC_txt->TextChanged += gcnew System::EventHandler(this, &Form1::DeltaC_txt_TextChanged);
			this->DeltaC_txt->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &Form1::DeltaC_txt_KeyPress);
			// 
			// DeltaE_txt
			// 
			this->DeltaE_txt->Location = System::Drawing::Point(48, 200);
			this->DeltaE_txt->Name = L"DeltaE_txt";
			this->DeltaE_txt->Size = System::Drawing::Size(66, 20);
			this->DeltaE_txt->TabIndex = 15;
			this->DeltaE_txt->TextChanged += gcnew System::EventHandler(this, &Form1::DeltaE_txt_TextChanged);
			this->DeltaE_txt->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &Form1::DeltaE_txt_KeyPress);
			// 
			// DeltaE_lbl
			// 
			this->DeltaE_lbl->AutoSize = true;
			this->DeltaE_lbl->Location = System::Drawing::Point(6, 203);
			this->DeltaE_lbl->Name = L"DeltaE_lbl";
			this->DeltaE_lbl->Size = System::Drawing::Size(42, 13);
			this->DeltaE_lbl->TabIndex = 14;
			this->DeltaE_lbl->Text = L"Delta E";
			// 
			// Lmax_txt
			// 
			this->Lmax_txt->Location = System::Drawing::Point(48, 179);
			this->Lmax_txt->Name = L"Lmax_txt";
			this->Lmax_txt->Size = System::Drawing::Size(66, 20);
			this->Lmax_txt->TabIndex = 13;
			this->Lmax_txt->TextChanged += gcnew System::EventHandler(this, &Form1::textBox5_TextChanged);
			this->Lmax_txt->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &Form1::Lmax_txt_KeyPress);
			// 
			// Lmax_lbl
			// 
			this->Lmax_lbl->AutoSize = true;
			this->Lmax_lbl->Location = System::Drawing::Point(6, 184);
			this->Lmax_lbl->Name = L"Lmax_lbl";
			this->Lmax_lbl->Size = System::Drawing::Size(38, 13);
			this->Lmax_lbl->TabIndex = 12;
			this->Lmax_lbl->Text = L"L_max";
			this->Lmax_lbl->Click += gcnew System::EventHandler(this, &Form1::label9_Click_1);
			// 
			// TM_txt
			// 
			this->TM_txt->Location = System::Drawing::Point(48, 139);
			this->TM_txt->Name = L"TM_txt";
			this->TM_txt->Size = System::Drawing::Size(66, 20);
			this->TM_txt->TabIndex = 11;
			this->TM_txt->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &Form1::TM_txt_KeyPress);
			// 
			// TMdummy_lbl
			// 
			this->TMdummy_lbl->AutoSize = true;
			this->TMdummy_lbl->Location = System::Drawing::Point(6, 141);
			this->TMdummy_lbl->Name = L"TMdummy_lbl";
			this->TMdummy_lbl->Size = System::Drawing::Size(23, 13);
			this->TMdummy_lbl->TabIndex = 10;
			this->TMdummy_lbl->Text = L"TM";
			// 
			// resize_txt
			// 
			this->resize_txt->Location = System::Drawing::Point(94, 357);
			this->resize_txt->Margin = System::Windows::Forms::Padding(2, 3, 2, 3);
			this->resize_txt->Name = L"resize_txt";
			this->resize_txt->Size = System::Drawing::Size(30, 20);
			this->resize_txt->TabIndex = 19;
			this->resize_txt->Text = L"2";
			this->resize_txt->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &Form1::resize_txt_KeyPress);
			// 
			// N_txt
			// 
			this->N_txt->Location = System::Drawing::Point(48, 117);
			this->N_txt->Name = L"N_txt";
			this->N_txt->Size = System::Drawing::Size(66, 20);
			this->N_txt->TabIndex = 9;
			this->N_txt->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &Form1::N_txt_KeyPress);
			// 
			// Ndummy_lbl
			// 
			this->Ndummy_lbl->AutoSize = true;
			this->Ndummy_lbl->Location = System::Drawing::Point(6, 124);
			this->Ndummy_lbl->Name = L"Ndummy_lbl";
			this->Ndummy_lbl->Size = System::Drawing::Size(15, 13);
			this->Ndummy_lbl->TabIndex = 8;
			this->Ndummy_lbl->Text = L"N";
			// 
			// Resizebydummy_lbl
			// 
			this->Resizebydummy_lbl->AutoSize = true;
			this->Resizebydummy_lbl->Location = System::Drawing::Point(5, 359);
			this->Resizebydummy_lbl->Margin = System::Windows::Forms::Padding(2, 0, 2, 0);
			this->Resizebydummy_lbl->Name = L"Resizebydummy_lbl";
			this->Resizebydummy_lbl->Size = System::Drawing::Size(93, 13);
			this->Resizebydummy_lbl->TabIndex = 18;
			this->Resizebydummy_lbl->Text = L"Resize Display by:";
			// 
			// k2dummy_lbl
			// 
			this->k2dummy_lbl->AutoSize = true;
			this->k2dummy_lbl->Location = System::Drawing::Point(6, 98);
			this->k2dummy_lbl->Name = L"k2dummy_lbl";
			this->k2dummy_lbl->Size = System::Drawing::Size(19, 13);
			this->k2dummy_lbl->TabIndex = 7;
			this->k2dummy_lbl->Text = L"k2";
			// 
			// k2_txt
			// 
			this->k2_txt->Location = System::Drawing::Point(48, 95);
			this->k2_txt->Name = L"k2_txt";
			this->k2_txt->Size = System::Drawing::Size(66, 20);
			this->k2_txt->TabIndex = 6;
			this->k2_txt->TextChanged += gcnew System::EventHandler(this, &Form1::k2_txt_TextChanged);
			this->k2_txt->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &Form1::k2_txt_KeyPress);
			// 
			// kdummy_lbl
			// 
			this->kdummy_lbl->AutoSize = true;
			this->kdummy_lbl->Location = System::Drawing::Point(4, 74);
			this->kdummy_lbl->Name = L"kdummy_lbl";
			this->kdummy_lbl->Size = System::Drawing::Size(13, 13);
			this->kdummy_lbl->TabIndex = 5;
			this->kdummy_lbl->Text = L"k";
			this->kdummy_lbl->Click += gcnew System::EventHandler(this, &Form1::label19_Click);
			// 
			// betadummy_lbl
			// 
			this->betadummy_lbl->AutoSize = true;
			this->betadummy_lbl->Location = System::Drawing::Point(3, 50);
			this->betadummy_lbl->Name = L"betadummy_lbl";
			this->betadummy_lbl->Size = System::Drawing::Size(28, 13);
			this->betadummy_lbl->TabIndex = 4;
			this->betadummy_lbl->Text = L"beta";
			// 
			// alphadummy_lbl
			// 
			this->alphadummy_lbl->AutoSize = true;
			this->alphadummy_lbl->Location = System::Drawing::Point(3, 24);
			this->alphadummy_lbl->Name = L"alphadummy_lbl";
			this->alphadummy_lbl->Size = System::Drawing::Size(33, 13);
			this->alphadummy_lbl->TabIndex = 3;
			this->alphadummy_lbl->Text = L"alpha";
			this->alphadummy_lbl->Click += gcnew System::EventHandler(this, &Form1::label9_Click);
			// 
			// k_txt
			// 
			this->k_txt->Location = System::Drawing::Point(48, 74);
			this->k_txt->Name = L"k_txt";
			this->k_txt->Size = System::Drawing::Size(66, 20);
			this->k_txt->TabIndex = 2;
			this->k_txt->TextChanged += gcnew System::EventHandler(this, &Form1::textBox7_TextChanged);
			this->k_txt->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &Form1::k_txt_KeyPress);
			// 
			// beta_txt
			// 
			this->beta_txt->Location = System::Drawing::Point(48, 48);
			this->beta_txt->Name = L"beta_txt";
			this->beta_txt->Size = System::Drawing::Size(66, 20);
			this->beta_txt->TabIndex = 1;
			this->beta_txt->TextChanged += gcnew System::EventHandler(this, &Form1::textBox6_TextChanged);
			this->beta_txt->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &Form1::beta_txt_KeyPress);
			// 
			// alpha_txt
			// 
			this->alpha_txt->Location = System::Drawing::Point(48, 22);
			this->alpha_txt->Name = L"alpha_txt";
			this->alpha_txt->Size = System::Drawing::Size(66, 20);
			this->alpha_txt->TabIndex = 0;
			this->alpha_txt->TextChanged += gcnew System::EventHandler(this, &Form1::alpha_txt_TextChanged);
			this->alpha_txt->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &Form1::alpha_txt_KeyPress);
			// 
			// frameNumber_lbl
			// 
			this->frameNumber_lbl->AutoSize = true;
			this->frameNumber_lbl->Location = System::Drawing::Point(905, 256);
			this->frameNumber_lbl->Margin = System::Windows::Forms::Padding(2, 0, 2, 0);
			this->frameNumber_lbl->Name = L"frameNumber_lbl";
			this->frameNumber_lbl->Size = System::Drawing::Size(36, 13);
			this->frameNumber_lbl->TabIndex = 22;
			this->frameNumber_lbl->Text = L"frame ";
			// 
			// processingtime_grp
			// 
			this->processingtime_grp->Controls->Add(this->desiredfps_txt);
			this->processingtime_grp->Controls->Add(this->dummy_fps_lbl);
			this->processingtime_grp->Controls->Add(this->frmbyfrm_radio);
			this->processingtime_grp->Controls->Add(this->realtime_radio);
			this->processingtime_grp->Enabled = false;
			this->processingtime_grp->Location = System::Drawing::Point(702, 226);
			this->processingtime_grp->Name = L"processingtime_grp";
			this->processingtime_grp->Size = System::Drawing::Size(188, 60);
			this->processingtime_grp->TabIndex = 38;
			this->processingtime_grp->TabStop = false;
			this->processingtime_grp->Text = L"processing time";
			// 
			// desiredfps_txt
			// 
			this->desiredfps_txt->Location = System::Drawing::Point(131, 33);
			this->desiredfps_txt->Name = L"desiredfps_txt";
			this->desiredfps_txt->Size = System::Drawing::Size(19, 20);
			this->desiredfps_txt->TabIndex = 2;
			this->desiredfps_txt->TextChanged += gcnew System::EventHandler(this, &Form1::desiredfps_txt_TextChanged);
			this->desiredfps_txt->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &Form1::desiredfps_txt_KeyPress);
			this->desiredfps_txt->Enter += gcnew System::EventHandler(this, &Form1::desiredfps_txt_Enter);
			// 
			// dummy_fps_lbl
			// 
			this->dummy_fps_lbl->AutoSize = true;
			this->dummy_fps_lbl->Location = System::Drawing::Point(101, 38);
			this->dummy_fps_lbl->Name = L"dummy_fps_lbl";
			this->dummy_fps_lbl->Size = System::Drawing::Size(88, 13);
			this->dummy_fps_lbl->TabIndex = 3;
			this->dummy_fps_lbl->Text = L"every        frames";
			// 
			// frmbyfrm_radio
			// 
			this->frmbyfrm_radio->AutoSize = true;
			this->frmbyfrm_radio->Location = System::Drawing::Point(10, 37);
			this->frmbyfrm_radio->Name = L"frmbyfrm_radio";
			this->frmbyfrm_radio->Size = System::Drawing::Size(97, 17);
			this->frmbyfrm_radio->TabIndex = 1;
			this->frmbyfrm_radio->TabStop = true;
			this->frmbyfrm_radio->Text = L"Frame by frame";
			this->frmbyfrm_radio->UseVisualStyleBackColor = true;
			this->frmbyfrm_radio->CheckedChanged += gcnew System::EventHandler(this, &Form1::frmbyfrm_radio_CheckedChanged);
			// 
			// realtime_radio
			// 
			this->realtime_radio->AutoSize = true;
			this->realtime_radio->Checked = true;
			this->realtime_radio->Location = System::Drawing::Point(10, 20);
			this->realtime_radio->Name = L"realtime_radio";
			this->realtime_radio->Size = System::Drawing::Size(69, 17);
			this->realtime_radio->TabIndex = 0;
			this->realtime_radio->TabStop = true;
			this->realtime_radio->Text = L"Real-time";
			this->realtime_radio->UseVisualStyleBackColor = true;
			this->realtime_radio->CheckedChanged += gcnew System::EventHandler(this, &Form1::realtime_radio_CheckedChanged);
			// 
			// Output_grp
			// 
			this->Output_grp->Controls->Add(this->camera_chk);
			this->Output_grp->Controls->Add(this->saveFrames_rdo);
			this->Output_grp->Controls->Add(this->saveFrames_txt);
			this->Output_grp->Enabled = false;
			this->Output_grp->Location = System::Drawing::Point(15, 571);
			this->Output_grp->Name = L"Output_grp";
			this->Output_grp->Size = System::Drawing::Size(266, 91);
			this->Output_grp->TabIndex = 39;
			this->Output_grp->TabStop = false;
			this->Output_grp->Text = L"Output Form";
			// 
			// camera_chk
			// 
			this->camera_chk->AutoSize = true;
			this->camera_chk->Location = System::Drawing::Point(8, 55);
			this->camera_chk->Margin = System::Windows::Forms::Padding(2);
			this->camera_chk->Name = L"camera_chk";
			this->camera_chk->Size = System::Drawing::Size(183, 17);
			this->camera_chk->TabIndex = 5;
			this->camera_chk->Text = L"Load Calibration File (camera.xml)";
			this->camera_chk->UseVisualStyleBackColor = true;
			this->camera_chk->CheckedChanged += gcnew System::EventHandler(this, &Form1::camera_chk_CheckedChanged);
			// 
			// saveFrames_rdo
			// 
			this->saveFrames_rdo->AutoSize = true;
			this->saveFrames_rdo->Location = System::Drawing::Point(8, 29);
			this->saveFrames_rdo->Margin = System::Windows::Forms::Padding(2);
			this->saveFrames_rdo->Name = L"saveFrames_rdo";
			this->saveFrames_rdo->Size = System::Drawing::Size(94, 17);
			this->saveFrames_rdo->TabIndex = 4;
			this->saveFrames_rdo->Text = L"Save frames \?";
			this->saveFrames_rdo->UseVisualStyleBackColor = true;
			this->saveFrames_rdo->CheckedChanged += gcnew System::EventHandler(this, &Form1::saveFrames_rdo_CheckedChanged);
			// 
			// saveFrames_txt
			// 
			this->saveFrames_txt->Location = System::Drawing::Point(107, 29);
			this->saveFrames_txt->Name = L"saveFrames_txt";
			this->saveFrames_txt->Size = System::Drawing::Size(144, 20);
			this->saveFrames_txt->TabIndex = 2;
			this->saveFrames_txt->Text = L"D:\\test2\\";
			// 
			// mainAlgorithm_grp
			// 
			this->mainAlgorithm_grp->Controls->Add(this->setMethod_btn);
			this->mainAlgorithm_grp->Controls->Add(this->LabCylinder_radio);
			this->mainAlgorithm_grp->Controls->Add(this->LabSpace_radio);
			this->mainAlgorithm_grp->Controls->Add(this->OriginalCB_radio);
			this->mainAlgorithm_grp->Enabled = false;
			this->mainAlgorithm_grp->Location = System::Drawing::Point(308, 26);
			this->mainAlgorithm_grp->Name = L"mainAlgorithm_grp";
			this->mainAlgorithm_grp->Size = System::Drawing::Size(220, 96);
			this->mainAlgorithm_grp->TabIndex = 40;
			this->mainAlgorithm_grp->TabStop = false;
			this->mainAlgorithm_grp->Text = L"Main Algorithm";
			// 
			// setMethod_btn
			// 
			this->setMethod_btn->Location = System::Drawing::Point(177, 48);
			this->setMethod_btn->Margin = System::Windows::Forms::Padding(2, 3, 2, 3);
			this->setMethod_btn->Name = L"setMethod_btn";
			this->setMethod_btn->Size = System::Drawing::Size(38, 22);
			this->setMethod_btn->TabIndex = 5;
			this->setMethod_btn->Text = L"Set";
			this->setMethod_btn->UseVisualStyleBackColor = true;
			this->setMethod_btn->Click += gcnew System::EventHandler(this, &Form1::setMethod_btn_Click);
			// 
			// LabCylinder_radio
			// 
			this->LabCylinder_radio->AutoSize = true;
			this->LabCylinder_radio->Location = System::Drawing::Point(26, 67);
			this->LabCylinder_radio->Name = L"LabCylinder_radio";
			this->LabCylinder_radio->Size = System::Drawing::Size(146, 17);
			this->LabCylinder_radio->TabIndex = 2;
			this->LabCylinder_radio->TabStop = true;
			this->LabCylinder_radio->Text = L"Kim + Cylinder Lab Space";
			this->LabCylinder_radio->UseVisualStyleBackColor = true;
			this->LabCylinder_radio->CheckedChanged += gcnew System::EventHandler(this, &Form1::LabCylinder_radio_CheckedChanged);
			// 
			// LabSpace_radio
			// 
			this->LabSpace_radio->AutoSize = true;
			this->LabSpace_radio->Location = System::Drawing::Point(26, 45);
			this->LabSpace_radio->Name = L"LabSpace_radio";
			this->LabSpace_radio->Size = System::Drawing::Size(143, 17);
			this->LabSpace_radio->TabIndex = 1;
			this->LabSpace_radio->Text = L"Kim + Sphere Lab Space";
			this->LabSpace_radio->UseVisualStyleBackColor = true;
			this->LabSpace_radio->CheckedChanged += gcnew System::EventHandler(this, &Form1::LabSpace_radio_CheckedChanged);
			// 
			// OriginalCB_radio
			// 
			this->OriginalCB_radio->AutoSize = true;
			this->OriginalCB_radio->Checked = true;
			this->OriginalCB_radio->Location = System::Drawing::Point(26, 23);
			this->OriginalCB_radio->Name = L"OriginalCB_radio";
			this->OriginalCB_radio->Size = System::Drawing::Size(141, 17);
			this->OriginalCB_radio->TabIndex = 0;
			this->OriginalCB_radio->TabStop = true;
			this->OriginalCB_radio->Text = L"Original CB method (Kim)";
			this->OriginalCB_radio->UseVisualStyleBackColor = true;
			this->OriginalCB_radio->CheckedChanged += gcnew System::EventHandler(this, &Form1::OriginalCB_radio_CheckedChanged);
			// 
			// whileRunningControls_grp
			// 
			this->whileRunningControls_grp->Controls->Add(this->LoadCB_btn);
			this->whileRunningControls_grp->Controls->Add(this->saveCB_btn);
			this->whileRunningControls_grp->Controls->Add(this->savecurrentFrame_btn);
			this->whileRunningControls_grp->Controls->Add(this->pause_btn);
			this->whileRunningControls_grp->Controls->Add(this->play_btn);
			this->whileRunningControls_grp->Enabled = false;
			this->whileRunningControls_grp->Location = System::Drawing::Point(720, 26);
			this->whileRunningControls_grp->Name = L"whileRunningControls_grp";
			this->whileRunningControls_grp->Size = System::Drawing::Size(117, 159);
			this->whileRunningControls_grp->TabIndex = 41;
			this->whileRunningControls_grp->TabStop = false;
			this->whileRunningControls_grp->Text = L"Run Controls";
			// 
			// LoadCB_btn
			// 
			this->LoadCB_btn->Enabled = false;
			this->LoadCB_btn->Location = System::Drawing::Point(2, 87);
			this->LoadCB_btn->Margin = System::Windows::Forms::Padding(2, 3, 2, 3);
			this->LoadCB_btn->Name = L"LoadCB_btn";
			this->LoadCB_btn->Size = System::Drawing::Size(110, 37);
			this->LoadCB_btn->TabIndex = 40;
			this->LoadCB_btn->Text = L"Load Codebook (CB.cb)";
			this->LoadCB_btn->UseVisualStyleBackColor = true;
			this->LoadCB_btn->Click += gcnew System::EventHandler(this, &Form1::LoadCB_btn_Click);
			// 
			// saveCB_btn
			// 
			this->saveCB_btn->Enabled = false;
			this->saveCB_btn->Location = System::Drawing::Point(3, 55);
			this->saveCB_btn->Margin = System::Windows::Forms::Padding(2, 3, 2, 3);
			this->saveCB_btn->Name = L"saveCB_btn";
			this->saveCB_btn->Size = System::Drawing::Size(110, 29);
			this->saveCB_btn->TabIndex = 39;
			this->saveCB_btn->Text = L"Save Codebook";
			this->saveCB_btn->UseVisualStyleBackColor = true;
			this->saveCB_btn->Click += gcnew System::EventHandler(this, &Form1::saveCB_btn_Click);
			// 
			// savecurrentFrame_btn
			// 
			this->savecurrentFrame_btn->Enabled = false;
			this->savecurrentFrame_btn->Location = System::Drawing::Point(3, 21);
			this->savecurrentFrame_btn->Margin = System::Windows::Forms::Padding(2, 3, 2, 3);
			this->savecurrentFrame_btn->Name = L"savecurrentFrame_btn";
			this->savecurrentFrame_btn->Size = System::Drawing::Size(110, 29);
			this->savecurrentFrame_btn->TabIndex = 38;
			this->savecurrentFrame_btn->Text = L"Save Frames";
			this->savecurrentFrame_btn->UseVisualStyleBackColor = true;
			this->savecurrentFrame_btn->Click += gcnew System::EventHandler(this, &Form1::savecurrentFrame_btn_Click);
			// 
			// pause_btn
			// 
			this->pause_btn->Enabled = false;
			this->pause_btn->Location = System::Drawing::Point(19, 126);
			this->pause_btn->Name = L"pause_btn";
			this->pause_btn->Size = System::Drawing::Size(34, 23);
			this->pause_btn->TabIndex = 36;
			this->pause_btn->Text = L"||";
			this->pause_btn->UseVisualStyleBackColor = true;
			this->pause_btn->Click += gcnew System::EventHandler(this, &Form1::pauseBtn_Click);
			// 
			// play_btn
			// 
			this->play_btn->Location = System::Drawing::Point(59, 126);
			this->play_btn->Name = L"play_btn";
			this->play_btn->Size = System::Drawing::Size(34, 23);
			this->play_btn->TabIndex = 35;
			this->play_btn->Text = L">";
			this->play_btn->UseVisualStyleBackColor = true;
			this->play_btn->Click += gcnew System::EventHandler(this, &Form1::play_btn_Click);
			// 
			// btn_halt
			// 
			this->btn_halt->Enabled = false;
			this->btn_halt->Location = System::Drawing::Point(723, 188);
			this->btn_halt->Margin = System::Windows::Forms::Padding(2, 3, 2, 3);
			this->btn_halt->Name = L"btn_halt";
			this->btn_halt->Size = System::Drawing::Size(110, 29);
			this->btn_halt->TabIndex = 42;
			this->btn_halt->Text = L"Halt !";
			this->btn_halt->UseVisualStyleBackColor = true;
			this->btn_halt->Click += gcnew System::EventHandler(this, &Form1::btn_halt_Click);
			// 
			// videoTotalTime_lbl
			// 
			this->videoTotalTime_lbl->AutoSize = true;
			this->videoTotalTime_lbl->Location = System::Drawing::Point(1014, 256);
			this->videoTotalTime_lbl->Margin = System::Windows::Forms::Padding(2, 0, 2, 0);
			this->videoTotalTime_lbl->Name = L"videoTotalTime_lbl";
			this->videoTotalTime_lbl->Size = System::Drawing::Size(26, 13);
			this->videoTotalTime_lbl->TabIndex = 43;
			this->videoTotalTime_lbl->Text = L"time";
			// 
			// ObjProperties_txt
			// 
			this->ObjProperties_txt->Location = System::Drawing::Point(544, 542);
			this->ObjProperties_txt->Margin = System::Windows::Forms::Padding(2);
			this->ObjProperties_txt->Multiline = true;
			this->ObjProperties_txt->Name = L"ObjProperties_txt";
			this->ObjProperties_txt->ReadOnly = true;
			this->ObjProperties_txt->ScrollBars = System::Windows::Forms::ScrollBars::Vertical;
			this->ObjProperties_txt->Size = System::Drawing::Size(345, 125);
			this->ObjProperties_txt->TabIndex = 46;
			this->ObjProperties_txt->Text = L"Object:\r\n";
			this->ObjProperties_txt->TextChanged += gcnew System::EventHandler(this, &Form1::ObjProperties_txt_TextChanged);
			this->ObjProperties_txt->MouseClick += gcnew System::Windows::Forms::MouseEventHandler(this, &Form1::ObjProperties_lbl_MouseClick);
			// 
			// CCProperties_txt
			// 
			this->CCProperties_txt->Location = System::Drawing::Point(900, 542);
			this->CCProperties_txt->Margin = System::Windows::Forms::Padding(2);
			this->CCProperties_txt->Multiline = true;
			this->CCProperties_txt->Name = L"CCProperties_txt";
			this->CCProperties_txt->ReadOnly = true;
			this->CCProperties_txt->ScrollBars = System::Windows::Forms::ScrollBars::Vertical;
			this->CCProperties_txt->Size = System::Drawing::Size(264, 124);
			this->CCProperties_txt->TabIndex = 47;
			this->CCProperties_txt->Text = L"CC\r";
			this->CCProperties_txt->TextChanged += gcnew System::EventHandler(this, &Form1::useMorphology_chk_CheckedChanged);
			this->CCProperties_txt->MouseClick += gcnew System::Windows::Forms::MouseEventHandler(this, &Form1::CCProperties_txt_MouseClick);
			// 
			// debug_monitor_chk
			// 
			this->debug_monitor_chk->AutoSize = true;
			this->debug_monitor_chk->Location = System::Drawing::Point(318, 124);
			this->debug_monitor_chk->Name = L"debug_monitor_chk";
			this->debug_monitor_chk->Size = System::Drawing::Size(115, 17);
			this->debug_monitor_chk->TabIndex = 48;
			this->debug_monitor_chk->Text = L"Debugging monitor";
			this->debug_monitor_chk->UseVisualStyleBackColor = true;
			this->debug_monitor_chk->CheckedChanged += gcnew System::EventHandler(this, &Form1::debug_monitor_chk_CheckedChanged);
			// 
			// SingleObjBehaviour_txt
			// 
			this->SingleObjBehaviour_txt->Location = System::Drawing::Point(5, 44);
			this->SingleObjBehaviour_txt->Margin = System::Windows::Forms::Padding(2);
			this->SingleObjBehaviour_txt->Multiline = true;
			this->SingleObjBehaviour_txt->Name = L"SingleObjBehaviour_txt";
			this->SingleObjBehaviour_txt->ReadOnly = true;
			this->SingleObjBehaviour_txt->ScrollBars = System::Windows::Forms::ScrollBars::Vertical;
			this->SingleObjBehaviour_txt->Size = System::Drawing::Size(208, 125);
			this->SingleObjBehaviour_txt->TabIndex = 49;
			// 
			// dummyObjectBehaviour_lbl
			// 
			this->dummyObjectBehaviour_lbl->AutoSize = true;
			this->dummyObjectBehaviour_lbl->Location = System::Drawing::Point(39, 21);
			this->dummyObjectBehaviour_lbl->Name = L"dummyObjectBehaviour_lbl";
			this->dummyObjectBehaviour_lbl->Size = System::Drawing::Size(38, 13);
			this->dummyObjectBehaviour_lbl->TabIndex = 58;
			this->dummyObjectBehaviour_lbl->Text = L"Object";
			// 
			// oneObjectHist_btn
			// 
			this->oneObjectHist_btn->Location = System::Drawing::Point(161, 13);
			this->oneObjectHist_btn->Name = L"oneObjectHist_btn";
			this->oneObjectHist_btn->Size = System::Drawing::Size(47, 24);
			this->oneObjectHist_btn->TabIndex = 57;
			this->oneObjectHist_btn->Text = L"history";
			this->oneObjectHist_btn->UseVisualStyleBackColor = true;
			this->oneObjectHist_btn->Click += gcnew System::EventHandler(this, &Form1::oneObjectHist_btn_Click);
			// 
			// ObjectBehaviour_txt
			// 
			this->ObjectBehaviour_txt->Location = System::Drawing::Point(81, 17);
			this->ObjectBehaviour_txt->Name = L"ObjectBehaviour_txt";
			this->ObjectBehaviour_txt->Size = System::Drawing::Size(65, 20);
			this->ObjectBehaviour_txt->TabIndex = 56;
			// 
			// dummySingleObjectBehaviour_grp
			// 
			this->dummySingleObjectBehaviour_grp->Controls->Add(this->SingleObjBehaviour_txt);
			this->dummySingleObjectBehaviour_grp->Controls->Add(this->dummyObjectBehaviour_lbl);
			this->dummySingleObjectBehaviour_grp->Controls->Add(this->ObjectBehaviour_txt);
			this->dummySingleObjectBehaviour_grp->Controls->Add(this->oneObjectHist_btn);
			this->dummySingleObjectBehaviour_grp->Location = System::Drawing::Point(315, 493);
			this->dummySingleObjectBehaviour_grp->Name = L"dummySingleObjectBehaviour_grp";
			this->dummySingleObjectBehaviour_grp->Size = System::Drawing::Size(223, 177);
			this->dummySingleObjectBehaviour_grp->TabIndex = 59;
			this->dummySingleObjectBehaviour_grp->TabStop = false;
			this->dummySingleObjectBehaviour_grp->Text = L"Single Object Behaviour";
			// 
			// loadInitialMask_chk
			// 
			this->loadInitialMask_chk->AutoSize = true;
			this->loadInitialMask_chk->Location = System::Drawing::Point(703, 520);
			this->loadInitialMask_chk->Name = L"loadInitialMask_chk";
			this->loadInitialMask_chk->Size = System::Drawing::Size(78, 17);
			this->loadInitialMask_chk->TabIndex = 61;
			this->loadInitialMask_chk->Text = L"Load mask";
			this->loadInitialMask_chk->UseVisualStyleBackColor = true;
			this->loadInitialMask_chk->CheckedChanged += gcnew System::EventHandler(this, &Form1::loadInitialMask_chk_CheckedChanged);
			// 
			// Show_prediction_chk
			// 
			this->Show_prediction_chk->AutoSize = true;
			this->Show_prediction_chk->Enabled = false;
			this->Show_prediction_chk->Location = System::Drawing::Point(318, 140);
			this->Show_prediction_chk->Name = L"Show_prediction_chk";
			this->Show_prediction_chk->Size = System::Drawing::Size(108, 17);
			this->Show_prediction_chk->TabIndex = 62;
			this->Show_prediction_chk->Text = L"Show Predictions";
			this->Show_prediction_chk->UseVisualStyleBackColor = true;
			this->Show_prediction_chk->CheckedChanged += gcnew System::EventHandler(this, &Form1::Show_prediction_chk_CheckedChanged);
			// 
			// show_speed_height_chk
			// 
			this->show_speed_height_chk->AutoSize = true;
			this->show_speed_height_chk->Enabled = false;
			this->show_speed_height_chk->Location = System::Drawing::Point(318, 156);
			this->show_speed_height_chk->Name = L"show_speed_height_chk";
			this->show_speed_height_chk->Size = System::Drawing::Size(92, 17);
			this->show_speed_height_chk->TabIndex = 63;
			this->show_speed_height_chk->Text = L"Show velocity";
			this->show_speed_height_chk->UseVisualStyleBackColor = true;
			this->show_speed_height_chk->CheckedChanged += gcnew System::EventHandler(this, &Form1::show_speed_height_chk_CheckedChanged);
			// 
			// show_dummy_chk
			// 
			this->show_dummy_chk->AutoSize = true;
			this->show_dummy_chk->Enabled = false;
			this->show_dummy_chk->Location = System::Drawing::Point(318, 172);
			this->show_dummy_chk->Name = L"show_dummy_chk";
			this->show_dummy_chk->Size = System::Drawing::Size(97, 17);
			this->show_dummy_chk->TabIndex = 64;
			this->show_dummy_chk->Text = L"Show dummies";
			this->show_dummy_chk->UseVisualStyleBackColor = true;
			this->show_dummy_chk->CheckedChanged += gcnew System::EventHandler(this, &Form1::show_dummy_chk_CheckedChanged);
			// 
			// show_blob_chk
			// 
			this->show_blob_chk->AutoSize = true;
			this->show_blob_chk->Enabled = false;
			this->show_blob_chk->Location = System::Drawing::Point(421, 174);
			this->show_blob_chk->Name = L"show_blob_chk";
			this->show_blob_chk->Size = System::Drawing::Size(81, 17);
			this->show_blob_chk->TabIndex = 65;
			this->show_blob_chk->Text = L"Show blobs";
			this->show_blob_chk->UseVisualStyleBackColor = true;
			this->show_blob_chk->CheckedChanged += gcnew System::EventHandler(this, &Form1::show_blob_chk_CheckedChanged);
			// 
			// faint_chk
			// 
			this->faint_chk->AutoSize = true;
			this->faint_chk->Enabled = false;
			this->faint_chk->Location = System::Drawing::Point(432, 127);
			this->faint_chk->Name = L"faint_chk";
			this->faint_chk->Size = System::Drawing::Size(76, 17);
			this->faint_chk->TabIndex = 66;
			this->faint_chk->Text = L"Show faint";
			this->faint_chk->UseVisualStyleBackColor = true;
			this->faint_chk->CheckedChanged += gcnew System::EventHandler(this, &Form1::faint_chk_CheckedChanged);
			// 
			// button12
			// 
			this->button12->Location = System::Drawing::Point(6, 135);
			this->button12->Name = L"button12";
			this->button12->Size = System::Drawing::Size(93, 23);
			this->button12->TabIndex = 34;
			this->button12->Text = L"LoiterCAV";
			this->button12->UseVisualStyleBackColor = true;
			this->button12->Click += gcnew System::EventHandler(this, &Form1::button12_Click);
			// 
			// Form1
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(1174, 682);
			this->Controls->Add(this->faint_chk);
			this->Controls->Add(this->show_blob_chk);
			this->Controls->Add(this->show_dummy_chk);
			this->Controls->Add(this->show_speed_height_chk);
			this->Controls->Add(this->Show_prediction_chk);
			this->Controls->Add(this->loadInitialMask_chk);
			this->Controls->Add(this->dummySingleObjectBehaviour_grp);
			this->Controls->Add(this->debug_monitor_chk);
			this->Controls->Add(this->CCProperties_txt);
			this->Controls->Add(this->ObjProperties_txt);
			this->Controls->Add(this->timeOrFrames_grp);
			this->Controls->Add(this->videoTotalTime_lbl);
			this->Controls->Add(this->btn_halt);
			this->Controls->Add(this->whileRunningControls_grp);
			this->Controls->Add(this->mainAlgorithm_grp);
			this->Controls->Add(this->Output_grp);
			this->Controls->Add(this->processingtime_grp);
			this->Controls->Add(this->frameNumber_lbl);
			this->Controls->Add(this->param_grp);
			this->Controls->Add(this->ProcTime_grp);
			this->Controls->Add(this->Codebook_grp);
			this->Controls->Add(this->activatedOptions_grp);
			this->Controls->Add(this->selectSource_grp);
			this->MaximizeBox = false;
			this->Name = L"Form1";
			this->Text = L" ";
			this->Load += gcnew System::EventHandler(this, &Form1::Form1_Load);
			this->selectSource_grp->ResumeLayout(false);
			this->selectSource_grp->PerformLayout();
			this->selectVideoSource_grp->ResumeLayout(false);
			this->selectImagesSource_grp->ResumeLayout(false);
			this->selectImagesSource_grp->PerformLayout();
			this->activatedOptions_grp->ResumeLayout(false);
			this->activatedOptions_grp->PerformLayout();
			this->timeOrFrames_grp->ResumeLayout(false);
			this->timeOrFrames_grp->PerformLayout();
			this->Codebook_grp->ResumeLayout(false);
			this->Codebook_grp->PerformLayout();
			this->ProcTime_grp->ResumeLayout(false);
			this->ProcTime_grp->PerformLayout();
			this->param_grp->ResumeLayout(false);
			this->param_grp->PerformLayout();
			this->processingtime_grp->ResumeLayout(false);
			this->processingtime_grp->PerformLayout();
			this->Output_grp->ResumeLayout(false);
			this->Output_grp->PerformLayout();
			this->mainAlgorithm_grp->ResumeLayout(false);
			this->mainAlgorithm_grp->PerformLayout();
			this->whileRunningControls_grp->ResumeLayout(false);
			this->dummySingleObjectBehaviour_grp->ResumeLayout(false);
			this->dummySingleObjectBehaviour_grp->PerformLayout();
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion


	private: System::Void radioButton1_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
			 }
	private: System::Void radioButton2_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
			 }
	private: System::Void radioButton3_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
			 }
	private: System::Void textBox1_TextChanged(System::Object^  sender, System::EventArgs^  e) {
			 }
	private: System::Void textBox2_TextChanged(System::Object^  sender, System::EventArgs^  e) {
			 }
	private: System::Void radioButton21_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
			 }
	private: System::Void button4_Click(System::Object^  sender, System::EventArgs^  e) {
			 }
	private: System::Void button10_Click(System::Object^  sender, System::EventArgs^  e) {
			 }




	private: System::Void button2_Click(System::Object^  sender, System::EventArgs^  e) {
				 glbl_var.filename = "ball3.avi";
				 glbl_var.videoStream = cvCreateFileCapture("ball3.avi");	
				 load_video();
			 }



	private: System::Void play_btn_Click(System::Object^  sender, System::EventArgs^  e) {


			

				 Output_grp->Enabled = false;

				 if(glbl_var.saveFramesToHard && glbl_var.saveTimes)
				 {
					 char filename[100]; 
					 strcpy(filename,glbl_var.saveFramesTo);
					 strcat(filename,"CPUfile.csv");
					 glbl_var.CPUfile.open(filename);
					 glbl_var.CPUfile << "Resize time" <<"," << "Conversion time" << "," << "homomorphic time" << "," << "median time" << "," << "update time" << "," << "BGsubtraction time" << "," << "Post processing time" << "," << "Tracker time" << ","<< "Behaviour time" << "," << "display time" << "," << "total time" << "," << "avg num of CWpPXL" << "," << "Number of blobs" << "," << "real frame number" <<"," << "processed frame number" << "," << "current time" << "," << "abandoned luggage" << "," << "stolen luggage" << "," << "loiter" << "," << "fight" << "," << "faint" << "," << "meeting" << "," << "walking together" <<endl;
				 }


					 //initialized the returned image
					 if (glbl_var.BGframe == NULL)
					 {

						 //resize the current frame for processing if must
						 if (glbl_var.resize_processing_factor == true)
							 glbl_var.queryframe2currentframe();

						 cvSetMouseCallback("BG_window",mouseHandler, 0);
						 CvSize newSize = cvSize(glbl_var.BGmodel->get_w(), glbl_var.BGmodel->get_h());
						 glbl_var.BGframe =  cvCreateImage(newSize,IPL_DEPTH_8U,3);
						 glbl_var.BGmask = cvCreateImage(newSize,IPL_DEPTH_8U,1);



					 }

					 //buttons and groupboxes adjustment
					 pause_btn->Enabled = 1;
					 btn_halt->Enabled = 1;
					 play_btn->Enabled = 0;
					 savecurrentFrame_btn->Enabled = 1;
					 mainAlgorithm_grp->Enabled = 0;
					 LoadCB_btn->Enabled = 0;
					 Codebook_grp->Enabled = 1;
					 saveCB_btn->Enabled = 1;
					 glbl_var.video_paused = 0;

					 //load initial mask
					 if(glbl_var.initialMaskFrame)
					 {
						 glbl_var.initialMask = cvLoadImage("I:\\University work\\Thesis\\Code\\withInterface\\Version3withInterface\\initialMask.jpg",0);
						 cvThreshold(glbl_var.initialMask,glbl_var.initialMask,127,255,CV_THRESH_BINARY);
						 glbl_var.initialMask3Channels = cvCreateImage(cvGetSize(glbl_var.initialMask),glbl_var.initialMask->depth,3);
						 cvMerge(glbl_var.initialMask,glbl_var.initialMask,glbl_var.initialMask,0,glbl_var.initialMask3Channels);
						 glbl_var.blackFrame = cvCreateImage(cvGetSize(glbl_var.initialMask),glbl_var.initialMask->depth,1);
						 cvZero(glbl_var.blackFrame);
					 }

					 //distance related thresholds
					 glbl_var.BGmodel->CC_Obj_max_acceptable_distance_onetoone = 0.62; //, general: 0.62
					 glbl_var.BGmodel->CC_Obj_max_acceptable_distance_intersection = 0.62; //,general: 0.62
					 int unit_of_measurement = METER;//METER, MILLIMETER
					 double base = 10;
					 if (glbl_var.CameraCalibrated)
					 {
						 //mm
						 glbl_var.BGmodel->MOTION_TOLERANCE = 1*pow(base,unit_of_measurement);// 0.5 meters , 500 mm
						 glbl_var.BGmodel->AVERAGE_PERSON_HEIGHT = 1.6*pow(base,unit_of_measurement); // 1.6 meters , 1600 mm
						 glbl_var.BGmodel->FALLING_TOLERANCE = 4*pow(base,unit_of_measurement); //2004, behave: 3 meter, 3000 mm,
						 glbl_var.BehaviourDescriptor.OBJECT_MOVED_THRESHOLD = 1*pow(base,unit_of_measurement);// 1 meters , 1000 mm
						 glbl_var.BehaviourDescriptor.DIFFERENT_SPEEDS_THRESHOLD = 1*pow(base,unit_of_measurement); //0.5 meters/second. 500 mm/s
						 glbl_var.BehaviourDescriptor.ABANDONED_LUGGAGE_DISTANCE_THRESHOLD= 3*pow(base,unit_of_measurement); // 1 meter, 1000 mm
						 glbl_var.BehaviourDescriptor.MEETING_DISTANCE_THRESHOLD = 2*pow(base,unit_of_measurement); // usual : 1500 mm
						 //glbl_var.BehaviourDescriptor.FIGHTING_MOTION_THRESHOLD = 1*pow(base,unit_of_measurement); // 0.5 meter, 500 mm
						 glbl_var.BehaviourDescriptor.DRAGGING_DISTANCE_THRESHOLD = 0.75*pow(base,unit_of_measurement); //0.75 meter, 750 mm

						 //m
						 //glbl_var.BGmodel->MOTION_TOLERANCE = 0.500;// 0.5 meters , 500 mm
						 //glbl_var.BGmodel->AVERAGE_PERSON_HEIGHT = 1.600; // 1.6 meters , 1600 mm
						 //glbl_var.BGmodel->FALLING_TOLERANCE = 2.000; //2004: 3 meter, 3000 mm,
						 //glbl_var.BehaviourDescriptor.OBJECT_MOVED_THRESHOLD = 1.000;// 1 meters , 1000 mm
						 //glbl_var.BehaviourDescriptor.DIFFERENT_SPEEDS_THRESHOLD = 1.000; //0.5 meters/second. 500 mm/s
						 //glbl_var.BehaviourDescriptor.ABANDONED_LUGGAGE_DISTANCE_THRESHOLD= 1.000; // 1 meter, 1000 mm
						 //glbl_var.BehaviourDescriptor.MEETING_DISTANCE_THRESHOLD = 1.500; // 1.5 meter, 1500 mm 
						 //glbl_var.BehaviourDescriptor.FIGHTING_DISTANCE_THRESHOLD = 0.500; // 0.5 meter, 500 mm
						 //glbl_var.BehaviourDescriptor.DRAGGING_DISTANCE_THRESHOLD = 0.750; //0.75 meter, 750 mm

					 }
					 else
					 {
						 //pixel
						 glbl_var.BGmodel->MOTION_TOLERANCE = 15;// pixels
						 glbl_var.BGmodel-> AVERAGE_PERSON_HEIGHT = 50; // pixels
						 glbl_var.BGmodel->FALLING_TOLERANCE = -1; //meaningless in terms of pixels
						 glbl_var.BehaviourDescriptor.OBJECT_MOVED_THRESHOLD = 30;
						 glbl_var.BehaviourDescriptor.DIFFERENT_SPEEDS_THRESHOLD = 15; //15 pixels/second
						 glbl_var.BehaviourDescriptor.ABANDONED_LUGGAGE_DISTANCE_THRESHOLD = 30; // 
						 glbl_var.BehaviourDescriptor.MEETING_DISTANCE_THRESHOLD= 15; // 
						 glbl_var.BehaviourDescriptor.FIGHTING_DISTANCE_THRESHOLD= 15; // 
						 glbl_var.BehaviourDescriptor.DRAGGING_DISTANCE_THRESHOLD = 23; //
					 }



					 //disable processing dimensions changing
					 // rsz_prcs_txt->Enabled = false;

					 //determine sequence true frame rate

					 LARGE_INTEGER tickspersec;
					 QueryPerformanceFrequency(&tickspersec);

					 // Real-time vs. frame by frame
					 if (glbl_var.realtimeProcessing) //real-time
					 {

						 Thread^ frame_thread = gcnew Thread(gcnew ThreadStart(this, &Form1::ReadFrames));
						 Thread^ proc_thread = gcnew Thread(gcnew ThreadStart(this, &Form1::ProcessFrame));
						 frame_thread->Name = "FrameThread";
						 proc_thread->Name = "ProcessingThread";

						 //run the threads : one for reading frames, and when for processing as many as it can
						 frame_thread->Start();
						 proc_thread->Start();

						 /* frame_thread->Join();
						 frame_thread->Join();*/
					 }
					 else //frame by frame
					 {

						 while(!glbl_var.video_finished)
						 {
							 LARGE_INTEGER begin; QueryPerformanceCounter(&begin);

							 ReadFrames();
							 ProcessFrame();

							 LARGE_INTEGER end; QueryPerformanceCounter(&end);
							 LONGLONG dif = ((end.QuadPart - begin.QuadPart)*1000/tickspersec.QuadPart);
							 int tempWait =  1000/glbl_var.OriginalVideoFrameRate - dif;
							 if ( tempWait > 0 )
								 cvWaitKey(tempWait);
							 else
								 cvWaitKey(1); //to allow interaction interface
						 }
					 }

			 }
	private: System::Void pauseBtn_Click(System::Object^  sender, System::EventArgs^  e) {
				 glbl_var.video_paused = 1; //change play/pause status
				 pause_btn->Enabled = 0;
				 play_btn->Enabled = 1;
				 while (glbl_var.video_paused)
				 {

					 cvWaitKey(1);


					 //if the user requested a new pixel or codeword
					 if (glbl_var.codebook_lbls_changed)
						 update_CB_labels();
				 }

			 }

	private: System::Void lastacc_lbl_Click(System::Object^  sender, System::EventArgs^  e) {
			 }
	private: System::Void freq_lbl_Click(System::Object^  sender, System::EventArgs^  e) {
			 }
	private: System::Void Form1_Load(System::Object^  sender, System::EventArgs^  e) {

				 play_btn->Enabled = false;
				 pause_btn->Enabled = false;
				 map_next_btn->Enabled = false;
				 map_prev_btn->Enabled = false;
			 }
	private: System::Void pxlNum_lbl_Click(System::Object^  sender, System::EventArgs^  e) {
			 }



	private: System::Void video2_btn_Click(System::Object^  sender, System::EventArgs^  e) {

				 glbl_var.filename = "D:\\Datasets\\CAVIAR (PETS 2004)-\\Videos\\mpg\\Fight_OneManDown.mpg";
				 glbl_var.videoStream = cvCreateFileCapture("D:\\Datasets\\CAVIAR (PETS 2004)-\\Videos\\mpg\\Fight_OneManDown.mpg");
				 glbl_var.videoReadingMethod = readFromVideo;

				 /* glbl_var.filename = "D:\\Datasets\\PETS 2007\\s01\\S01_GENERAL_LOITERING_1\\3\\thirdView";
				 glbl_var.videoStream = cvCreateFileCapture("D:\\Datasets\\PETS 2007\\s01\\S01_GENERAL_LOITERING_1\\3\\thirdView");
				 glbl_var.ImageReadingExtension = ".jpg";
				 glbl_var.numberOfDigits = 6;*/
				 load_video();

			 }
	private: System::Void prevCB_lbl_Click(System::Object^  sender, System::EventArgs^  e) {
				 glbl_var.codebook_lbls_changed = true;
				 glbl_var.DisplayedCB--;
				 glbl_var.fetch_mh_params();

			 }
	private: System::Void nextCB_lbl_Click(System::Object^  sender, System::EventArgs^  e) {


				 glbl_var.codebook_lbls_changed = true;
				 glbl_var.DisplayedCB++;
				 glbl_var.fetch_mh_params();

			 }

	private: void update_CB_labels()
			 {
				 codebookNumber_lbl->Text = "CB #" + glbl_var.mh_prm.codebookNumber.ToString();
				 MNRL_lbl->Text = ((glbl_var.mh_prm.MNRL != -1) ?   "MNRL = " + glbl_var.mh_prm.MNRL.ToString() : "-");
				 firstacc_lbl->Text = ((glbl_var.mh_prm.firstacc != -1) ?  "1st acc = " + glbl_var.mh_prm.firstacc.ToString() : "-") ;
				 lastacc_lbl->Text = ((glbl_var.mh_prm.lastacc != -1) ?   "last acc = " + glbl_var.mh_prm.lastacc.ToString() : "-") ;
				 freq_lbl->Text = ((glbl_var.mh_prm.freq != -1) ?  "f = " + glbl_var.mh_prm.freq.ToString() : "-");
				 region_lbl->Text = ((glbl_var.mh_prm.region_num != 0) ?  "Region: " + glbl_var.mh_prm.region_num.ToString() : "-");
				 area_lbl->Text = ((glbl_var.mh_prm.region_area != -1) ?  "area: " + glbl_var.mh_prm.region_area.ToString() : "-");

				 char* temp = new char[3];
				 temp[0]= glbl_var.mh_prm.R;
				 temp[1]= glbl_var.mh_prm.G;
				 temp[2]= glbl_var.mh_prm.B;
				 I_lbl->Text = "I = " + glbl_var.BGmodel->calc_I_fromSqr(temp);
				 delete [] temp;



				 pxlNum_lbl->Text =  "pixel: " + glbl_var.mh_prm.pxlNum_x.ToString() + " , " + glbl_var.mh_prm.pxlNum_y.ToString();

				 switch(glbl_var.BGmodel->get_used_method())
				 {
				 case KimOriginal:
					 R_lbl->Text = "B = " + glbl_var.mh_prm.R;
					 G_lbl->Text = "G = " + glbl_var.mh_prm.G;
					 B_lbl->Text = "R = " + glbl_var.mh_prm.B;
					 R_CB_lbl->Text = "B_CB = " + cvFloor(glbl_var.mh_prm.R_CB);
					 G_CB_lbl->Text = "G_CB = " + cvFloor(glbl_var.mh_prm.G_CB);
					 B_CB_lbl->Text = "R_CB = " + cvFloor(glbl_var.mh_prm.B_CB);
					 Imin_lbl->Text =  ((glbl_var.mh_prm.Imin != -1) ? "Imin = " + cvFloor(glbl_var.mh_prm.Imin).ToString() : "-");
					 Imax_lbl->Text =  ((glbl_var.mh_prm.Imax != -1) ? "Imax = " + cvFloor(glbl_var.mh_prm.Imax).ToString() : "-");
					 Ilow_lbl->Text =  ((glbl_var.mh_prm.Imin != -1) ? 
						 "I low = " + cvFloor(glbl_var.BGmodel->get_Ilow(glbl_var.mh_prm.Imax)) : "-");
					 Ihigh_lbl->Text = ((glbl_var.mh_prm.Imin != -1) ? 
						 "I high = " + cvFloor(glbl_var.BGmodel->get_Ihigh(glbl_var.mh_prm.Imin,glbl_var.mh_prm.Imax)) : "-");

					 pxl_color_lbl->ForeColor = ColorTranslator::FromOle(RGB(glbl_var.mh_prm.R,glbl_var.mh_prm.G,glbl_var.mh_prm.B));
					 pxl_color_lbl->BackColor = ColorTranslator::FromOle(RGB(glbl_var.mh_prm.R,glbl_var.mh_prm.G,glbl_var.mh_prm.B));
					 CB_color_lbl->ForeColor = ColorTranslator::FromOle(RGB(glbl_var.mh_prm.R_CB,glbl_var.mh_prm.G_CB,glbl_var.mh_prm.B_CB));
					 CB_color_lbl->BackColor = ColorTranslator::FromOle(RGB(glbl_var.mh_prm.R_CB,glbl_var.mh_prm.G_CB,glbl_var.mh_prm.B_CB));
					 break;
				 case LabCylindrical:
				 case LabSpherical:
					 R_lbl->Text = "L = " + cvFloor(glbl_var.mh_prm.L);
					 G_lbl->Text = "a = " + cvFloor(glbl_var.mh_prm.a);
					 B_lbl->Text = "b = " + cvFloor(glbl_var.mh_prm.b);
					 R_CB_lbl->Text = "L_CB = " + cvFloor(glbl_var.mh_prm.L_CB);
					 G_CB_lbl->Text = "a_CB = " + cvFloor(glbl_var.mh_prm.a_CB);
					 B_CB_lbl->Text = "b_CB = " + cvFloor(glbl_var.mh_prm.b_CB);
					 Imin_lbl->Text = "-";
					 Imax_lbl->Text = "-";
					 Ilow_lbl->Text = "-";
					 Ihigh_lbl->Text ="-";
					 I_lbl->Text ="-";

					 CvScalar cvs = cvScalar(glbl_var.mh_prm.L_CB,glbl_var.mh_prm.a_CB,glbl_var.mh_prm.b_CB);
					 cvs = glbl_var.real2display_Lab(cvs);
					 CvScalar pxl;
					 pxl = glbl_var.convert_pxl_Lab2BGR(cvs);
					 CB_color_lbl->ForeColor = ColorTranslator::FromOle(RGB(pxl.val[2],pxl.val[1],pxl.val[0]));
					 CB_color_lbl->BackColor = ColorTranslator::FromOle(RGB(pxl.val[2],pxl.val[1],pxl.val[0]));

					 cvs = cvScalar(glbl_var.mh_prm.L,glbl_var.mh_prm.a,glbl_var.mh_prm.b);
					 cvs = glbl_var.real2display_Lab(cvs);
					 pxl = glbl_var.convert_pxl_Lab2BGR(cvs);
					 pxl_color_lbl->ForeColor = ColorTranslator::FromOle(RGB(pxl.val[2],pxl.val[1],pxl.val[0]));
					 pxl_color_lbl->BackColor = ColorTranslator::FromOle(RGB(pxl.val[2],pxl.val[1],pxl.val[0]));
					 break;
				 }


				 maxCB_lbl->Text = "Max = " + glbl_var.mh_prm.maxCB;
				 if (glbl_var.mh_prm.codebookNumber == 0)
				 {
					 prevCB_lbl->Enabled = 0;
					 if (glbl_var.mh_prm.maxCB > 1)
						 nextCB_lbl->Enabled = 1;
					 else
						 nextCB_lbl->Enabled = 0;
				 }
				 else if(glbl_var.mh_prm.codebookNumber == glbl_var.mh_prm.maxCB-1)
				 {
					 nextCB_lbl->Enabled = 0;
					 prevCB_lbl->Enabled = 1;
				 }
				 else
				 {
					 nextCB_lbl->Enabled = 1;
					 prevCB_lbl->Enabled = 1;
				 }


				 glbl_var.codebook_lbls_changed = false;

				 float delta;
				 float delta2;
				 int result = glbl_var.calc_pxl_status_mh(delta,glbl_var.BGmodel->get_used_method(),delta2);
				 switch (result)
				 {
				 case 1: //color at testing period
				 case 2:
					 pxlStatus_lbl->Text = "RGB: FG: Color ep2";
					 break;
				 case 4:  //color at training period
				 case 3:
					 pxlStatus_lbl->Text = "RGB: FG: Color ep1";
					 break;
				 case 5: //brightness
				 case 6:
					 pxlStatus_lbl->Text = "RGB: FG: Brightness";
					 break;
				 case 7: 
					 pxlStatus_lbl->Text = "FG: MNRL";
					 break;
				 case 11:
					 pxlStatus_lbl->Text = "Lab: FG: Color";
					 break;
				 case 12: 
					 pxlStatus_lbl->Text = "Lab Cyl: Object";
					 break;
				 case 13:
					 pxlStatus_lbl->Text = "Lab Cyl: Object Color";
					 break;
				 case 14:
					 pxlStatus_lbl->Text = "Lab Cyl: Shadow";
					 break;
				 case 0:
					 pxlStatus_lbl->Text = "BG";
					 break;
				 }
				 delta_lbl->Text = "delta= " + delta;



				 switch(glbl_var.BGmodel->get_used_method())
				 {
				 case KimOriginal:
					 if (glbl_var.BGmodel->get_t() > glbl_var.BGmodel->get_N())
						 epsilon_lbl->Text = "epsilon = " + glbl_var.BGmodel->get_epsilon2().ToString();
					 else
						 epsilon_lbl->Text = "epsilon = " + glbl_var.BGmodel->get_epsilon1().ToString();
					 break;
				 case LabSpherical: 
					 epsilon_lbl->Text = "Delta E = " + glbl_var.BGmodel->get_DeltaE().ToString();
					 break;
				 case LabCylindrical:
					 delta2_lbl->Text = "delta2 = " + delta2;
					 epsilon_lbl->Text = "Delta C = " + glbl_var.BGmodel->get_DeltaC().ToString();
					 epsilon2_lbl->Text = "Delta L = " + glbl_var.BGmodel->get_DeltaL().ToString();
					 break;
				 }



			 }


	private: System::Void video3_btn_Click(System::Object^  sender, System::EventArgs^  e) {
				  if (glbl_var.ImageReadingExtension) delete [] glbl_var.ImageReadingExtension;
				 glbl_var.ImageReadingExtension = new char[100];
				  if (glbl_var.filename) delete [] glbl_var.filename;
				 glbl_var.filename = new char[100];

				 strcpy(glbl_var.ImageReadingExtension,".jpg");
				 strcpy(glbl_var.filename,"D:\\Datasets\\BEHAVE-\\main\\Jpegs\\all\\");
				 glbl_var.numberOfDigits = 8;
				 glbl_var.startFromFrame = 46200;
				 glbl_var.videoReadingMethod = readFromImages;
				 load_video();
			 }
	private: System::Void Codebook_grp_Enter(System::Object^  sender, System::EventArgs^  e) {
			 }
	private: System::Void label9_Click(System::Object^  sender, System::EventArgs^  e) {
			 }
	private: System::Void label19_Click(System::Object^  sender, System::EventArgs^  e) {
			 }
	private: System::Void textBox6_TextChanged(System::Object^  sender, System::EventArgs^  e) {

			 }
	private: System::Void textBox7_TextChanged(System::Object^  sender, System::EventArgs^  e) {

			 }
	private: System::Void alpha_txt_KeyPress(System::Object^  sender, System::Windows::Forms::KeyPressEventArgs^  e) {
				 if( e->KeyChar == (char)Keys::Enter)
					 glbl_var.BGmodel->set_alpha(Single::Parse(alpha_txt->Text));
			 }
	private: System::Void beta_txt_KeyPress(System::Object^  sender, System::Windows::Forms::KeyPressEventArgs^  e) {
				 if( e->KeyChar == (char)Keys::Enter)
					 glbl_var.BGmodel->set_beta(Single::Parse(beta_txt->Text))	;	


			 }
	private: System::Void k_txt_KeyPress(System::Object^  sender, System::Windows::Forms::KeyPressEventArgs^  e) {
				 if( e->KeyChar == (char)Keys::Enter)
					 glbl_var.BGmodel->set_k(Single::Parse(k_txt->Text));

			 }

	private: void load_video()
			 {


				 glbl_var.reset();
				 glbl_var.video_paused=1; //initially, the video is not playing
				 pause_btn->Enabled = 0;
				 play_btn->Enabled = 1;
				 glbl_var.codebook_lbls_changed = false;


				 cvNamedWindow( "video_window", CV_WINDOW_AUTOSIZE );
				 cvSetMouseCallback("video_window",mouseHandler, 0);
				 //Dlgt d = gcnew Dlgt(&mouseHandler);

				 glbl_var.currentFramePos =  glbl_var.startFromFrame;
				 if (glbl_var.videoReadingMethod == readFromVideo)
					 glbl_var.queryFrame = cvQueryFrame( glbl_var.videoStream );
				 else //read from images
				 {
					 /* char* pathTest = new char[150];
					 readFrame(glbl_var.filename, glbl_var.currentFramePos, glbl_var.numberOfDigits, glbl_var.ImageReadingExtension, pathTest);
					 glbl_var.queryFrame =  cvLoadImage(pathTest);
					 delete[] pathTest;*/
					 readFrame(glbl_var.filename, glbl_var.currentFramePos, glbl_var.numberOfDigits, glbl_var.ImageReadingExtension, glbl_var.queryFrame);
					 //glbl_var.queryFrame = readFrame(glbl_var.filename, glbl_var.currentFramePos, glbl_var.numberOfDigits, glbl_var.ImageReadingExtension);
				 }

				 //REMOVE
				 IplImage* imgcrap = glbl_var.queryFrame;

				 glbl_var.currentFrame = cvCreateImage(cvSize(glbl_var.queryFrame->width,glbl_var.queryFrame->height),glbl_var.queryFrame->depth,glbl_var.queryFrame->nChannels);
				 cvCopy(glbl_var.queryFrame,glbl_var.currentFrame);

				 int height = cvGetSize(glbl_var.currentFrame).height;
				 int width = cvGetSize(glbl_var.currentFrame).width;


				 if ( glbl_var.BGmodel) 
					 delete glbl_var.BGmodel;
				 glbl_var.BGmodel = new BG_codebook_model(height,width);

				 k_txt->Text = glbl_var.BGmodel->get_k().ToString();
				 k2_txt->Text = glbl_var.BGmodel->get_k2().ToString();
				 alpha_txt->Text = glbl_var.BGmodel->get_alpha().ToString();
				 beta_txt->Text = glbl_var.BGmodel->get_beta().ToString();
				 N_txt->Text = glbl_var.BGmodel->get_N().ToString();
				 TM_txt->Text = glbl_var.BGmodel->get_TM().ToString();
				 Lmax_txt->Text = glbl_var.BGmodel->get_Lmax().ToString();
				 DeltaE_txt->Text = glbl_var.BGmodel->get_DeltaE().ToString();
				 DeltaC_txt->Text = glbl_var.BGmodel->get_DeltaC().ToString();
				 DeltaS_txt->Text = glbl_var.BGmodel->get_DeltaS().ToString();
				 DeltaL_txt->Text = glbl_var.BGmodel->get_DeltaL().ToString();
				 fmin_txt->Text = glbl_var.BGmodel->get_fmin().ToString();
				 wantedSpeed_txt->Text = glbl_var.speedLimit.ToString();
				 minAreaThreshold_txt->Text = glbl_var.BGmodel->get_minArea().ToString();
				 maxAreaThreshold_txt->Text = glbl_var.BGmodel->get_maxArea().ToString();
				 //txt_trackerBW->Text = glbl_var.BGmodel->get_kernel_h().ToString();
				 inTermsOfTime_radio->Checked = true;


				 //glbl_var.total_num_of_frames = (int) cvGetCaptureProperty(glbl_var.videoStream,CV_CAP_PROP_FRAME_COUNT);


				 // glbl_var.currentFramePos = (int) cvGetCaptureProperty(glbl_var.videoStream,CV_CAP_PROP_POS_FRAMES)/(int) cvGetCaptureProperty(glbl_var.videoStream,CV_CAP_PROP_FRAME_COUNT);
				 if( glbl_var.BGmodel->get_t() != 0 ) 
					 int tmp_slide = cvCreateTrackbar("Position","video_window",&glbl_var.currentFramePos,100,onTrackbarSlide);

				 //set the size of the median array
				 int dum1 =  cvGetCaptureProperty(glbl_var.videoStream,CV_CAP_PROP_FRAME_COUNT);
				 int dum2 =  cvGetCaptureProperty(glbl_var.videoStream,CV_CAP_PROP_POS_FRAMES);
				 int temp  = cvGetCaptureProperty(glbl_var.videoStream,CV_CAP_PROP_FRAME_COUNT)/cvGetCaptureProperty(glbl_var.videoStream,CV_CAP_PROP_POS_FRAMES);
				 //glbl_var.BGmodel->set_spc_medians_size(temp*2);
				 //glbl_var.BGmodel->set_spc_medians_size(3000);

				 //convert the size to the parameters in the textboxes 
				 //CvSize newSize = cvSize(Convert::ToInt32(desiredHeight_txt->Text), Convert::ToInt32(desiredWidth_txt->Text)); //paper's specification
				 //cvResize(initFrame,frame);



				 //specify framerate
				 if (glbl_var.videoReadingMethod == readFromVideo)
					 glbl_var.OriginalVideoFrameRate = cvGetCaptureProperty(glbl_var.videoStream,CV_CAP_PROP_FPS); //real fps
				 else
					 glbl_var.OriginalVideoFrameRate = Single::Parse(readfromimagesfps_txt->Text); //real fps

				 desiredfps_txt->Text = L"1";



				 /* glbl_var.concatimage = cvCreateImage(cvSize(glbl_var.currentFrame->width*2,glbl_var.currentFrame->height),glbl_var.currentFrame->depth,glbl_var.currentFrame->nChannels);
				 cvSetImageROI(glbl_var.concatimage,cvRect(0,0,glbl_var.BGmodel->get_w(),glbl_var.BGmodel->get_h()));
				 cvCopy(glbl_var.currentFrame,glbl_var.concatimage);
				 cvResetImageROI(glbl_var.concatimage);
				 cvShowImage( "video_window", glbl_var.concatimage );
				 cvReleaseImage(&glbl_var.concatimage);*/

				 //activate method stage
				 mainAlgorithm_grp->Enabled = true;
				 selectImagesSource_grp->Enabled = false;

				 //Construct Color tables
				 //glbl_var.buildBGR2Lab_table();
				 //glbl_var.buildLab2BGR_table();

				 //make the saveFramesDirectory
				 strcpy(glbl_var.saveFramesTo,(char*)(void*)Marshal::StringToHGlobalAnsi(saveFrames_txt->Text));
				 srand(time(0));
				 char buffer[100];
				 strcat(glbl_var.saveFramesTo,itoa(rand()%10000000,buffer,10));
				 strcat(glbl_var.saveFramesTo,"\\");
				 mkdir(glbl_var.saveFramesTo);

			 }
	private: System::Void groupBox4_Enter(System::Object^  sender, System::EventArgs^  e) {
			 }
	private: System::Void k2_txt_KeyPress(System::Object^  sender, System::Windows::Forms::KeyPressEventArgs^  e) {
				 if( e->KeyChar == (char)Keys::Enter)
					 glbl_var.BGmodel->set_k2(Single::Parse(k2_txt->Text));
			 }
	private: System::Void alpha_txt_TextChanged(System::Object^  sender, System::EventArgs^  e) {
			 }
	private: System::Void button8_Click(System::Object^  sender, System::EventArgs^  e) {
				 cvSetTrackbarPos("Position","video_window",100);
			 }
	private: System::Void N_txt_KeyPress(System::Object^  sender, System::Windows::Forms::KeyPressEventArgs^  e) {
				 if( e->KeyChar == (char)Keys::Enter)
					 glbl_var.BGmodel->set_N(Single::Parse(N_txt->Text));
			 }
	private: System::Void TM_txt_KeyPress(System::Object^  sender, System::Windows::Forms::KeyPressEventArgs^  e) {
				 if( e->KeyChar == (char)Keys::Enter)
					 glbl_var.BGmodel->set_TM(Single::Parse(TM_txt->Text));
			 }
	private: System::Void video4_btn_Click(System::Object^  sender, System::EventArgs^  e) {
				 glbl_var.filename = "D:\\Datasets\\CAVIAR (PETS 2004)-\\Videos\\mpg\\Meet_WalkTogether1.mpg";
				 glbl_var.videoStream = cvCreateFileCapture("D:\\Datasets\\CAVIAR (PETS 2004)-\\Videos\\mpg\\Meet_WalkTogether1.mpg");
				 glbl_var.videoReadingMethod = readFromVideo;
				 load_video();
			 }
	private: System::Void ImgVid_grp_Enter(System::Object^  sender, System::EventArgs^  e) {
			 }
	private: System::Void swapRB_chk_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
			 }
	private: System::Void k2_txt_TextChanged(System::Object^  sender, System::EventArgs^  e) {
			 }
	private: System::Void Ihigh_lbl_Click(System::Object^  sender, System::EventArgs^  e) {
			 }
	private: System::Void video5_btn_Click(System::Object^  sender, System::EventArgs^  e) {
				 glbl_var.filename = "D:\\Datasets\\CAVIAR (PETS 2004)-\\Videos\\mpg\\Rest_FallOnFloor.mpg";
				 glbl_var.videoStream = cvCreateFileCapture("D:\\Datasets\\CAVIAR (PETS 2004)-\\Videos\\mpg\\Rest_FallOnFloor.mpg");
				 glbl_var.videoReadingMethod = readFromVideo;
				 load_video();
			 }
	private: System::Void showmap_btn_Click(System::Object^  sender, System::EventArgs^  e) {


				 if (map_next_btn->Enabled == false && map_prev_btn->Enabled == false)
				 {
					 cvNamedWindow( "map_window", CV_WINDOW_AUTOSIZE ); //to show the codebooks 
					 cvNamedWindow( "Detailed_BG_window", CV_WINDOW_AUTOSIZE ); //to show why a pixel is a forgeround
					 cvSetMouseCallback("map_window",mouseHandler, 0);
					 cvSetMouseCallback("Detailed_BG_window",mouseHandler, 0);
					 map_next_btn->Enabled = true;
				 }





				 CvScalar x; 
				 float I;
				 codebook* CB_ptr = glbl_var.BGmodel->getCodebook(0,0,0);
				 //CB_ptr += glbl_var.DisplayedMapCB;
				 //char* pixel_ptr;
				 char* pixel_ptr = glbl_var.currentFrame->imageData;

				 CvSize newSize = cvSize(glbl_var.BGmodel->get_w(), glbl_var.BGmodel->get_h());
				 glbl_var.Map_returned_image= cvCreateImage(newSize,IPL_DEPTH_8U,3);
				 glbl_var.Detailed_BG_returned_image= cvCreateImage(newSize,IPL_DEPTH_8U,3);

				 //for (int i=0; i<frame_size; i++)
				 int j_map=0;
				 for (int cnt_y=0; cnt_y<glbl_var.BGmodel->get_h(); cnt_y++)
					 for (int cnt_x=0; cnt_x<glbl_var.BGmodel->get_w(); cnt_x++)
					 {




						 //pixel_ptr = glbl_var.currentFrame->imageData + glbl_var.BGmodel->get_widthStep()*cnt_y + cnt_x*glbl_var.BGmodel->get_nCh();

						 switch(glbl_var.BGmodel->get_used_method())
						 {
						 case KimOriginal:
							 x = cvScalar((double)(unsigned char)pixel_ptr[0],(double)(unsigned char)pixel_ptr[1],(double)(unsigned char)pixel_ptr[2],0); //assigning the RGB values from the frame
							 break;
						 case LabCylindrical:
						 case LabSpherical:
							 x = cvScalar((double)(unsigned char)pixel_ptr[0],(double)(unsigned char)pixel_ptr[1],(double)(unsigned char)pixel_ptr[2],0); //assigning the RGB values from the frame
							 //x = glbl_var.convert_pxl_BGR2Lab(x);
							 x = glbl_var.display2real_Lab(x);
							 break;
						 }

						 if (glbl_var.BGmodel->get_used_method() == KimOriginal) I = glbl_var.BGmodel->calc_I_fromSqr(pixel_ptr);

						 CB_ptr = glbl_var.BGmodel->get_needed_CB(cnt_x + cnt_y*glbl_var.BGmodel->get_w()
							 ,glbl_var.DisplayedMapCB); //passing a pointer which value will be the needed CB
						 if (!CB_ptr) //skip all operations in case the CW doesn't exist
						 {
							 j_map+=3;
							 continue;
						 }

						 switch(glbl_var.BGmodel->get_used_method())
						 {
						 case KimOriginal:
							 glbl_var.Map_returned_image->imageData[j_map] = CB_ptr->getRGB().val[0];
							 glbl_var.Map_returned_image->imageData[j_map+1] = CB_ptr->getRGB().val[1];
							 glbl_var.Map_returned_image->imageData[j_map+2] = CB_ptr->getRGB().val[2];
							 break;
						 case LabCylindrical:
						 case LabSpherical:
							 CvScalar cvs = glbl_var.real2display_Lab(CB_ptr->getLab());
							 CvScalar pxl;
							 pxl = glbl_var.convert_pxl_Lab2BGR(cvs);
							 glbl_var.Map_returned_image->imageData[j_map] = pxl.val[0];
							 glbl_var.Map_returned_image->imageData[j_map+1] = pxl.val[1];
							 glbl_var.Map_returned_image->imageData[j_map+2] = pxl.val[2];
							 break;
						 }


						 int result=-1; //invalid default value
						 if ( glbl_var.BGmodel->getNum_of_active_models_per_pixel()[(int)(j_map/3)] > glbl_var.DisplayedMapCB)
							 //the CW exists
						 {

							 float d_dummy;
							 float d_dummy2;
							 result = glbl_var.calc_pxl_status_pointer(d_dummy,glbl_var.BGmodel->get_used_method(),CB_ptr,x,d_dummy2);
							 switch (result)
							 {
							 case KimOriginal_FG_ColorBrightness_Training:
							 case KimOriginal_FG_ColorBrightness_Testing:
							 case LabCylindrical_FG_ChromaLuminance:
								 glbl_var.Detailed_BG_returned_image->imageData[j_map] = 128;//purple
								 glbl_var.Detailed_BG_returned_image->imageData[j_map+1] = 0;
								 glbl_var.Detailed_BG_returned_image->imageData[j_map+2] = 128;
								 break;
							 case KimOriginal_FG_Color_Training:
							 case LabSpherical_FG_Color:
							 case 13:
								 glbl_var.Detailed_BG_returned_image->imageData[j_map] = 0;//orange
								 glbl_var.Detailed_BG_returned_image->imageData[j_map+1] = 112;
								 glbl_var.Detailed_BG_returned_image->imageData[j_map+2] = 255;
								 break;
							 case KimOriginal_FG_Color_Testing:
								 glbl_var.Detailed_BG_returned_image->imageData[j_map] = 0;//red
								 glbl_var.Detailed_BG_returned_image->imageData[j_map+1] = 0;
								 glbl_var.Detailed_BG_returned_image->imageData[j_map+2] = 255;
								 break;
							 case KimOriginal_FG_BrightnessLow:
								 glbl_var.Detailed_BG_returned_image->imageData[j_map] = 0;//light green
								 glbl_var.Detailed_BG_returned_image->imageData[j_map+1] = 128;
								 glbl_var.Detailed_BG_returned_image->imageData[j_map+2] = 0;
								 break;
							 case KimOriginal_FG_BrightnessHigh:
							 case LabCylindrical_FG_Shadow:
								 glbl_var.Detailed_BG_returned_image->imageData[j_map] = 0;//green
								 glbl_var.Detailed_BG_returned_image->imageData[j_map+1] = 255;
								 glbl_var.Detailed_BG_returned_image->imageData[j_map+2] = 0;
								 break;
							 case AllMethods_FG_MNRLorFreq:
								 glbl_var.Detailed_BG_returned_image->imageData[j_map] = 255;//blue
								 glbl_var.Detailed_BG_returned_image->imageData[j_map+1] = 0;
								 glbl_var.Detailed_BG_returned_image->imageData[j_map+2] = 0;
								 break;
							 case AllMethods_BG:
								 //case LabCylindrical_FG_Shadow: //if it's part of the BG, restore its color (unless method = LabCylindrical and it's a shadow(green), then display it even though it's a BG)
								 CvScalar tmpx = cvScalar((double)(unsigned char)pixel_ptr[0],(double)(unsigned char)pixel_ptr[1],(double)(unsigned char)pixel_ptr[2],0); //assigning the RGB values from the frame
								 tmpx = glbl_var.convert_pxl_Lab2BGR(tmpx);
								 //tmpx = glbl_var.display2real_Lab(tmpx);
								 glbl_var.Detailed_BG_returned_image->imageData[j_map] = tmpx.val[0];
								 glbl_var.Detailed_BG_returned_image->imageData[j_map+1] = tmpx.val[1];
								 glbl_var.Detailed_BG_returned_image->imageData[j_map+2] = tmpx.val[2];
								 break;
							 }

						 }
						 else //CW doesn't exist
						 {
							 glbl_var.Map_returned_image->imageData[j_map] = 255; //white
							 glbl_var.Map_returned_image->imageData[j_map+1] = 255;
							 glbl_var.Map_returned_image->imageData[j_map+2] = 255;

						 }

						 //CB_ptr += glbl_var.BGmodel->get_Lmax(); //move to next pixel
						 j_map+=3;
						 pixel_ptr+=3;
					 }

					 if (glbl_var.resize_factor!= 1)
					 {
						 cvResize(glbl_var.Map_returned_image,glbl_var.displayedFrame,CV_INTER_AREA );
						 cvShowImage("map_window",glbl_var.displayedFrame);
						 cvResize(glbl_var.Detailed_BG_returned_image,glbl_var.displayedFrame,CV_INTER_AREA );
						 cvShowImage("Detailed_BG_window",glbl_var.displayedFrame);
					 }
					 else
					 {
						 cvShowImage("map_window",glbl_var.Map_returned_image);
						 cvShowImage("Detailed_BG_window",glbl_var.Detailed_BG_returned_image);
					 }
			 }
	private: System::Void map_next_btn_Click(System::Object^  sender, System::EventArgs^  e) {

				 glbl_var.DisplayedMapCB++;
				 if (glbl_var.DisplayedMapCB == 0)
				 {
					 map_prev_btn->Enabled = 0;
					 map_next_btn->Enabled = 1;
				 }
				 else if (glbl_var.DisplayedMapCB ==glbl_var.BGmodel->get_Lmax()-1)
				 {
					 map_next_btn->Enabled = 0;
					 map_prev_btn->Enabled = 1;
				 }
				 else
				 {
					 map_next_btn->Enabled = 1;
					 map_prev_btn->Enabled = 1;
				 }
				 showmap_btn->Text = "Show Map CB " + glbl_var.DisplayedMapCB;
			 }
	private: System::Void map_prev_btn_Click(System::Object^  sender, System::EventArgs^  e) {
				 glbl_var.DisplayedMapCB--;
				 if (glbl_var.DisplayedMapCB == 0)
				 {
					 map_prev_btn->Enabled = 0;
					 map_next_btn->Enabled = 1;
				 }
				 else if (glbl_var.DisplayedMapCB == glbl_var.BGmodel->get_Lmax()-1)
				 {
					 map_next_btn->Enabled = 0;
					 map_prev_btn->Enabled = 1;
				 }
				 else
				 {
					 map_next_btn->Enabled = 1;
					 map_prev_btn->Enabled = 1;
				 }
				 showmap_btn->Text = "Show Map CB " + glbl_var.DisplayedMapCB;
			 }
	private: System::Void groupBox6_Enter(System::Object^  sender, System::EventArgs^  e) {
			 }
	private: System::Void label9_Click_1(System::Object^  sender, System::EventArgs^  e) {
			 }
	private: System::Void textBox5_TextChanged(System::Object^  sender, System::EventArgs^  e) {
			 }
	private: System::Void Lmax_txt_KeyPress(System::Object^  sender, System::Windows::Forms::KeyPressEventArgs^  e) {
				 if( e->KeyChar == (char)Keys::Enter)
					 glbl_var.BGmodel->set_Lmax(Single::Parse(Lmax_txt->Text));
			 }
	private: System::Void resize_txt_KeyPress(System::Object^  sender, System::Windows::Forms::KeyPressEventArgs^  e) {



			 }
	private: System::Void DeltaE_txt_KeyPress(System::Object^  sender, System::Windows::Forms::KeyPressEventArgs^  e) {
				 if( e->KeyChar == (char)Keys::Enter)
					 glbl_var.BGmodel->set_DeltaE((Single::Parse(DeltaE_txt->Text)));
			 }
	private: System::Void DeltaE_txt_TextChanged(System::Object^  sender, System::EventArgs^  e) {
			 }
	private: System::Void LoadCB_btn_Click(System::Object^  sender, System::EventArgs^  e) {
				 ifstream inCB("CB.cb",ios::in);


				 int int_param;//both used to retrieve data temporarily
				 float float_param;
				 double double_param;
				 double double_param2;
				 double double_param3;
				 bool bool_param;

				 inCB >> int_param; glbl_var.BGmodel->set_used_method(int_param);
				 //inCB >> bool_param; glbl_var.BGmodel->set_fminState(bool_param);
				 inCB >> int_param; glbl_var.BGmodel->set_Lmax(int_param);
				 //inCB >> int_param; glbl_var.BGmodel->set_TM(int_param);
				 //inCB >> int_param; glbl_var.BGmodel->set_N(int_param);
				 inCB >> int_param; glbl_var.BGmodel->set_w(int_param); 
				 inCB >> int_param; glbl_var.BGmodel->set_h(int_param);
				 //if (bool_param) { inCB >> int_param; glbl_var.BGmodel->set_fmin(int_param); }
				 int m = glbl_var.BGmodel->get_used_method();
				 /*if ( m == 0) { inCB >> float_param; glbl_var.BGmodel->set_alpha(float_param); }
				 if ( m == 0) { inCB >> float_param; glbl_var.BGmodel->set_beta(float_param); }
				 if ( m == 0) { inCB >> float_param; glbl_var.BGmodel->set_epsilon1(float_param); }
				 if ( m == 0) { inCB >> float_param; glbl_var.BGmodel->set_epsilon2(float_param); }
				 if ( m == 1) { inCB >> float_param; glbl_var.BGmodel->set_DeltaE(float_param); }
				 if ( m == 2) { inCB >> float_param; glbl_var.BGmodel->set_DeltaC(float_param); }
				 if ( m == 2) { inCB >> float_param; glbl_var.BGmodel->set_DeltaL(float_param); }
				 if ( m == 2) { inCB >> float_param; glbl_var.BGmodel->set_DeltaS(float_param); }*/


				 //codebook* x;
				 for (int cnt_y=0; cnt_y<glbl_var.BGmodel->get_h(); cnt_y++)
					 for (int cnt_x=0; cnt_x<glbl_var.BGmodel->get_w(); cnt_x++)
					 {

						 inCB >>int_param; glbl_var.BGmodel->getNum_of_active_models_per_pixel()[cnt_y*glbl_var.BGmodel->get_w() + cnt_x] = int_param;
						 int for_loop_bound = int_param;
						 codebook* x = glbl_var.BGmodel->force_getCodebook(cnt_x,cnt_y,0);
						 for (int j=0; j < for_loop_bound; j++)
						 {

							 codebook* x = glbl_var.BGmodel->force_getCodebook(cnt_x,cnt_y,j);
							 inCB >> int_param; x->setCW_freq(int_param);
							 inCB >> int_param; x->setMNRL(int_param);
							 inCB >> int_param; x->setfirst_access(int_param);
							 inCB >> int_param; x->setlast_access(int_param);
							 if ( m == 0) inCB >> float_param; x->setImax(float_param);
							 if ( m == 0) inCB >> float_param; x->setImin(float_param);
							 if ( m == 0) inCB >> double_param >> double_param2 >> double_param3; x->setRGB(cvScalar(double_param,double_param2,double_param3));
							 if ( m == 1 || m == 2 ) inCB >> double_param >> double_param2 >> double_param3; x->setLab(cvScalar(double_param,double_param2,double_param3));

							 //x = x->get_next_CW();
						 }
					 }
					 inCB.close();

					 //allow only for once;
					 LoadCB_btn->Enabled = false;


			 }
	private: System::Void saveCB_btn_Click(System::Object^  sender, System::EventArgs^  e) {



				 glbl_var.BGmodel->update_MNRLs(); //because MNRLs that haven't been accessed for along time haven't been updated.

				 ofstream outCB("CB.cb",ios::out);

				 // codebook* x = glbl_var.BGmodel->getCodebook(0,0,0);
				 int m = glbl_var.BGmodel->get_used_method();
				 //bool n = glbl_var.BGmodel->get_fminState();
				 outCB << m << ' ';
				 //outCB<< n <<' ';
				 outCB << glbl_var.BGmodel->get_Lmax() << ' ';
				 //outCB << glbl_var.BGmodel->get_TM() << ' ' << glbl_var.BGmodel->get_N() << ' ';
				 outCB <<  glbl_var.BGmodel->get_w() << ' ' << glbl_var.BGmodel->get_h() << ' ';
				 //if (n) outCB << glbl_var.BGmodel->get_fmin() <<endl;
				 /*	 if ( m == 0)
				 {
				 outCB << glbl_var.BGmodel->get_alpha() << ' ' << glbl_var.BGmodel->get_beta() << ' '; 
				 outCB << glbl_var.BGmodel->get_epsilon1() << ' ';
				 outCB << glbl_var.BGmodel->get_epsilon2() << endl;
				 }
				 else if (m == 1)
				 {
				 outCB << glbl_var.BGmodel->get_DeltaE() << endl;
				 }
				 else if (m == 2)
				 {
				 outCB << glbl_var.BGmodel->get_DeltaC() << endl;
				 outCB << glbl_var.BGmodel->get_DeltaL() << endl;
				 outCB << glbl_var.BGmodel->get_DeltaS() << endl;
				 }*/
				 for (int cnt_y=0; cnt_y<glbl_var.BGmodel->get_h(); cnt_y++)
					 for (int cnt_x=0; cnt_x<glbl_var.BGmodel->get_w(); cnt_x++)
					 {

						 codebook* x = glbl_var.BGmodel->getCodebook(cnt_x,cnt_y,0);
						 outCB << glbl_var.BGmodel->getNum_of_active_models_per_pixel()[cnt_y*glbl_var.BGmodel->get_w() + cnt_x] <<endl;
						 for (int j=0; j < glbl_var.BGmodel->getNum_of_active_models_per_pixel()[cnt_y*glbl_var.BGmodel->get_w() + cnt_x]; j++)
						 {

							 //update MNRL for 
							 int tempLastaccess = x->getlast_access();
							 if ((glbl_var.BGmodel->get_t() - tempLastaccess) > x->getMNRL()) 
								 x->setMNRL(glbl_var.BGmodel->get_t() - tempLastaccess);


							 outCB << x->getCW_freq() << ' ' << x->getMNRL() << ' ' << x->getfirst_access() << ' ' << x->getlast_access() << ' ';
							 if ( m == 0) outCB << x->getImax() << ' ' << x->getImin() << ' ' ;
							 if ( m == 0) outCB << x->getRGB().val[0] << ' ' << x->getRGB().val[1] << ' ' << x->getRGB().val[2] << endl;
							 if ( m == 1 || m == 2) outCB << x->getLab().val[0] << ' ' << x->getLab().val[1] << ' ' << x->getLab().val[2] << endl;
							 x = x->get_next_CW();
						 }
					 }

					 outCB.close();
			 }



	private: System::Void savecurrentFrame_btn_Click(System::Object^  sender, System::EventArgs^  e) {
				 cvSaveImage("currentFrame.jpg",glbl_var.currentFrame);
				 cvSaveImage("BGFrame.jpg",glbl_var.BGframe);
				 cvSaveImage("Detailed_BG_returned_image.jpg",glbl_var.Detailed_BG_returned_image);
				 cvSaveImage("Map_returned_image.jpg",glbl_var.Map_returned_image);
			 }
	private: System::Void saveBGframe_btn_Click(System::Object^  sender, System::EventArgs^  e) {
				 cvSaveImage("BGFrame.jpg",glbl_var.BGframe);
			 }
	private: System::Void button7_Click(System::Object^  sender, System::EventArgs^  e) {
			 }
	private: System::Void setMethod_btn_Click(System::Object^  sender, System::EventArgs^  e) {
				 if (OriginalCB_radio->Checked)
					 glbl_var.BGmodel->set_used_method(KimOriginal);
				 else if (LabSpace_radio->Checked)
					 glbl_var.BGmodel->set_used_method(LabSpherical);
				 else if (LabCylinder_radio->Checked)
					 glbl_var.BGmodel->set_used_method(LabCylindrical);

				 //allow loading codebook
				 setMethod_btn->Enabled = false;
				 timeOrFrames_grp->Enabled = true;
				 LoadCB_btn->Enabled = true;
				 activatedOptions_grp->Enabled = true;
				 param_grp->Enabled = true;
				 whileRunningControls_grp->Enabled = true;
				 processingtime_grp->Enabled = true;
			 }
	private: System::Void btn_setParam_Click(System::Object^  sender, System::EventArgs^  e) {


				 //glbl_var.resize_processing_factor =  (Single::Parse(rsz_prcs_txt->Text));
				 //reset dimentions of BGmodel if must
				 if (glbl_var.resize_processing_factor == true)
				 {
					 int w = glbl_var.BGmodel->get_w();
					 int h = glbl_var.BGmodel->get_h();
					 delete glbl_var.BGmodel;
					 //glbl_var.BGmodel = new BG_codebook_model(cvFloor(h*glbl_var.resize_processing_factor),cvFloor(w*glbl_var.resize_processing_factor));
					 glbl_var.BGmodel = new BG_codebook_model(240,
						 320);
					 glbl_var.BGmodel->set_widthStep(glbl_var.BGmodel->get_w()*glbl_var.BGmodel->get_nCh());

					 if (OriginalCB_radio->Checked)
						 glbl_var.BGmodel->set_used_method(KimOriginal);
					 else if (LabSpace_radio->Checked)
						 glbl_var.BGmodel->set_used_method(LabSpherical);
					 else if (LabCylinder_radio->Checked)
						 glbl_var.BGmodel->set_used_method(LabCylindrical);
				 }

				 glbl_var.BGmodel->set_DeltaE((Single::Parse(DeltaE_txt->Text)));
				 glbl_var.BGmodel->set_DeltaL((Single::Parse(DeltaL_txt->Text)));
				 glbl_var.BGmodel->set_DeltaC((Single::Parse(DeltaC_txt->Text)));
				 glbl_var.BGmodel->set_DeltaS(Single::Parse(DeltaS_txt->Text));
				 glbl_var.BGmodel->set_fmin(Single::Parse(fmin_txt->Text));
				 glbl_var.BGmodel->set_Lmax(Single::Parse(Lmax_txt->Text));
				 glbl_var.BGmodel->set_TM(Single::Parse(TM_txt->Text));
				 glbl_var.BGmodel->set_k2(Single::Parse(k2_txt->Text));
				 glbl_var.BGmodel->set_N(Single::Parse(N_txt->Text));
				 glbl_var.BGmodel->set_k(Single::Parse(k_txt->Text));
				 glbl_var.BGmodel->set_beta(Single::Parse(beta_txt->Text))	;
				 glbl_var.BGmodel->set_alpha(Single::Parse(alpha_txt->Text));
				 glbl_var.BGmodel->set_minArea((int) Single::Parse(minAreaThreshold_txt->Text));
				 glbl_var.BGmodel->set_maxArea((int) Single::Parse(maxAreaThreshold_txt->Text));
				 glbl_var.speedLimit = Single::Parse(wantedSpeed_txt->Text);
				 glbl_var.resize_factor =  (Single::Parse(resize_txt->Text));
				 //glbl_var.BGmodel->set_kernel_h(Single::Parse(txt_trackerBW->Text));


				 if ( disableUpdate_chk->Checked )
					 glbl_var.BGmodel->set_updatedDisabledDuringSubtraction(1);
				 else
					 glbl_var.BGmodel->set_updatedDisabledDuringSubtraction(0);

				 if (fmin_chk->Checked)
					 glbl_var.BGmodel->set_fminState(1);
				 else
					 glbl_var.BGmodel->set_fminState(0);

				 //disable the group (can only clicked once per video)
				 param_grp->Enabled = false;
				 Output_grp->Enabled = true;
				 activatedOptions_grp->Enabled = false;
				 timeOrFrames_grp->Enabled = false;

			 }
	private: System::Void button9_Click(System::Object^  sender, System::EventArgs^  e) {
			 }


	private: System::Void OriginalCB_radio_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
				 DeltaE_txt->Enabled = false;
				 DeltaC_txt->Enabled = false;
				 DeltaL_txt->Enabled = false;
				 DeltaS_txt->Enabled = false;
				 Lmax_txt->Enabled = true;
				 TM_txt->Enabled = true;
				 k2_txt->Enabled = true;
				 N_txt->Enabled = true;
				 k_txt->Enabled = true;
				 beta_txt->Enabled = true;
				 alpha_txt->Enabled = true;

				 homomorphic_chk->Enabled = false;
			 }
	private: System::Void LabSpace_radio_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
				 DeltaE_txt->Enabled = true;
				 DeltaC_txt->Enabled = false;
				 DeltaL_txt->Enabled = false;
				 DeltaS_txt->Enabled = false;
				 Lmax_txt->Enabled = true;
				 TM_txt->Enabled = true;
				 k2_txt->Enabled = false;
				 N_txt->Enabled = true;
				 k_txt->Enabled = false;
				 beta_txt->Enabled = false;
				 alpha_txt->Enabled = false;

				 homomorphic_chk->Enabled = false;
			 }
	private: System::Void LabCylinder_radio_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
				 DeltaE_txt->Enabled = false;
				 DeltaC_txt->Enabled = true;
				 DeltaL_txt->Enabled = true;
				 DeltaS_txt->Enabled = true;
				 Lmax_txt->Enabled = true;
				 TM_txt->Enabled = true;
				 k2_txt->Enabled = false;
				 N_txt->Enabled = true;
				 k_txt->Enabled = false;
				 beta_txt->Enabled = false;
				 alpha_txt->Enabled = false;

				 homomorphic_chk->Enabled = true;
			 }
	private: System::Void DeltaC_txt_TextChanged(System::Object^  sender, System::EventArgs^  e) {
			 }
	private: System::Void DeltaC_txt_KeyPress(System::Object^  sender, System::Windows::Forms::KeyPressEventArgs^  e) {
				 if( e->KeyChar == (char)Keys::Enter)
					 glbl_var.BGmodel->set_DeltaC((Single::Parse(DeltaC_txt->Text)));
			 }
	private: System::Void DeltaL_txt_KeyPress(System::Object^  sender, System::Windows::Forms::KeyPressEventArgs^  e) {
				 if( e->KeyChar == (char)Keys::Enter)
					 glbl_var.BGmodel->set_DeltaL((Single::Parse(DeltaL_txt->Text)));
			 }
	private: System::Void delta_lbl_Click(System::Object^  sender, System::EventArgs^  e) {
			 }
	private: System::Void wantedSpeed_txt_TextChanged(System::Object^  sender, System::EventArgs^  e) {
			 }
	private: System::Void wantedSpeed_txt_KeyPress(System::Object^  sender, System::Windows::Forms::KeyPressEventArgs^  e) {
				 /*if( e->KeyChar == (char)Keys::Enter)
				 glbl_var.speedLimit = Single::Parse(wantedSpeed_txt->Text);*/
			 }
	private: System::Void disableUpdate_chk_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
				 if ( disableUpdate_chk->Checked )
					 glbl_var.BGmodel->set_updatedDisabledDuringSubtraction(1);
				 else
					 glbl_var.BGmodel->set_updatedDisabledDuringSubtraction(0);
			 }
	private: System::Void DeltaS_txt_KeyPress(System::Object^  sender, System::Windows::Forms::KeyPressEventArgs^  e) {
				 if( e->KeyChar == (char)Keys::Enter)
					 glbl_var.BGmodel->set_DeltaS(Single::Parse(DeltaS_txt->Text));
			 }
	private: System::Void fmin_rad_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {

			 }
	private: System::Void fmin_chk_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
				 if (fmin_chk->Checked)
			  {
				  fmin_txt->Enabled = true;
				  glbl_var.BGmodel->set_fminState(true);
			  }
				 else
				 {
					 fmin_txt->Enabled = false;
					 glbl_var.BGmodel->set_fminState(false);
			  }
			 }
	private: System::Void fmin_txt_KeyPress(System::Object^  sender, System::Windows::Forms::KeyPressEventArgs^  e) {
				 if( e->KeyChar == (char)Keys::Enter)
					 glbl_var.BGmodel->set_fmin(Single::Parse(fmin_txt->Text));
			 }
	private: System::Void minArea_chk_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
				 if (minArea_chk->Checked)
				 {
					 minAreaThreshold_txt->Enabled = true;
					 maxAreaThreshold_txt->Enabled = true;
					 glbl_var.NFPP2_checked = 1;
				 }
				 else
				 {
					 minAreaThreshold_txt->Enabled = false;
					 maxAreaThreshold_txt->Enabled = false;
					 glbl_var.NFPP2_checked = 0;
				 }
			 }
	private: System::Void NFPP_chk_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
				 if (NFPP_chk->Checked)
				 {
					 minArea_chk->Enabled = true;
					 useMorphology_chk->Enabled = true;
					 glbl_var.NFPP_checked = 1;
				 }
				 else
				 {
					 minArea_chk->Enabled = false;
					 useMorphology_chk->Enabled = false;
					 glbl_var.NFPP_checked = 0;
				 }
			 }
	private: System::Void minAreaThreshold_txt_KeyPress(System::Object^  sender, System::Windows::Forms::KeyPressEventArgs^  e) {

			 }
	private: System::Void minAreaThreshold_txt_TextChanged(System::Object^  sender, System::EventArgs^  e) {
			 }
	private: System::Void rsz_prcs_txt_KeyPress(System::Object^  sender, System::Windows::Forms::KeyPressEventArgs^  e) {

			 }
	private: System::Void B_CB_lbl_Click(System::Object^  sender, System::EventArgs^  e) {
			 }
	private: System::Void I_lbl_Click(System::Object^  sender, System::EventArgs^  e) {
			 }
	private: System::Void CB_color_lbl_Click(System::Object^  sender, System::EventArgs^  e) {
			 }
	private: System::Void rsz_prcs_txt_TextChanged(System::Object^  sender, System::EventArgs^  e) {
			 }




	private: System::Void LoadCB_btn_Click_1(System::Object^  sender, System::EventArgs^  e) {
				 ifstream inCB("CB.cb",ios::in);


				 int int_param;//both used to retrieve data temporarily
				 float float_param;
				 double double_param;
				 double double_param2;
				 double double_param3;
				 bool bool_param;

				 inCB >> int_param; glbl_var.BGmodel->set_used_method(int_param);
				 inCB >> bool_param; glbl_var.BGmodel->set_fminState(bool_param);
				 inCB >> int_param; glbl_var.BGmodel->set_Lmax(int_param);
				 inCB >> int_param; glbl_var.BGmodel->set_TM(int_param);
				 inCB >> int_param; glbl_var.BGmodel->set_N(int_param);
				 inCB >> int_param; glbl_var.BGmodel->set_w(int_param); 
				 inCB >> int_param; glbl_var.BGmodel->set_h(int_param);
				 if (bool_param) { inCB >> int_param; glbl_var.BGmodel->set_fmin(int_param); }
				 int m = glbl_var.BGmodel->get_used_method();
				 if ( m == 0) { inCB >> float_param; glbl_var.BGmodel->set_alpha(float_param); }
				 if ( m == 0) { inCB >> float_param; glbl_var.BGmodel->set_beta(float_param); }
				 if ( m == 0) { inCB >> float_param; glbl_var.BGmodel->set_epsilon1(float_param); }
				 if ( m == 0) { inCB >> float_param; glbl_var.BGmodel->set_epsilon2(float_param); }
				 if ( m == 1) { inCB >> float_param; glbl_var.BGmodel->set_DeltaE(float_param); }
				 if ( m == 2) { inCB >> float_param; glbl_var.BGmodel->set_DeltaC(float_param); }
				 if ( m == 2) { inCB >> float_param; glbl_var.BGmodel->set_DeltaL(float_param); }
				 if ( m == 2) { inCB >> float_param; glbl_var.BGmodel->set_DeltaS(float_param); }


				 codebook* x;
				 for (int cnt_y=0; cnt_y<glbl_var.BGmodel->get_h(); cnt_y++)
					 for (int cnt_x=0; cnt_x<glbl_var.BGmodel->get_w(); cnt_x++)
					 {
						 x = glbl_var.BGmodel->force_getCodebook(cnt_x,cnt_y,0);
						 inCB >>int_param; glbl_var.BGmodel->getNum_of_active_models_per_pixel()[cnt_y*glbl_var.BGmodel->get_w() + cnt_x] = int_param;
						 int for_loop_bound = int_param;
						 for (int j=0; j < glbl_var.BGmodel->get_Lmax(); j++)
						 {
							 inCB >> int_param; x->setCW_freq(int_param);
							 inCB >> int_param; x->setMNRL(int_param);
							 inCB >> int_param; x->setfirst_access(int_param);
							 inCB >> int_param; x->setlast_access(int_param);
							 if ( m == 0) inCB >> float_param; x->setImax(float_param);
							 if ( m == 0) inCB >> float_param; x->setImin(float_param);
							 if ( m == 0) inCB >> double_param >> double_param2 >> double_param3; x->setRGB(cvScalar(double_param,double_param2,double_param3));
							 if ( m == 1 || m == 2 ) inCB >> double_param >> double_param2 >> double_param3; x->setLab(cvScalar(double_param,double_param2,double_param3));

							 x++;
						 }
					 }
					 inCB.close();

			 }
	private: System::Void savecurrentFrame_btn_Click_1(System::Object^  sender, System::EventArgs^  e) {
			 }
	private: System::Void Img_rdb_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
				 if (Img_rdb->Checked) //enable Images buttons
				 {
					 glbl_var.videoReadingMethod = readFromImages;
					 selectImagesSource_grp->Enabled = true;
				 }
				 else
				 {
					 selectImagesSource_grp->Enabled = false;
				 }
			 }
	private: System::Void vid_rdb_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
				 if (vid_rdb->Checked) //enable video buttons
				 {
					 glbl_var.videoReadingMethod = readFromVideo;
					 selectVideoSource_grp->Enabled = true;
				 }
				 else
				 {
					 selectVideoSource_grp->Enabled = false;
				 }
			 }
	private: System::Void label13_Click(System::Object^  sender, System::EventArgs^  e) {
			 }
	private: System::Void trackerGrpBox_Enter(System::Object^  sender, System::EventArgs^  e) {
			 }
	private: System::Void TrackerDisplaylbl_Enter(System::Object^  sender, System::EventArgs^  e) {
			 }
	private: System::Void TrackerDisplaylbl_KeyPress(System::Object^  sender, System::Windows::Forms::KeyPressEventArgs^  e) {

			 }
	private: System::Void TrackerDisplaylbl_TextChanged(System::Object^  sender, System::EventArgs^  e) {
			 }
	private: System::Void video6_btn_Click(System::Object^  sender, System::EventArgs^  e) {
				 glbl_var.filename = "D:\\Datasets\\CAVIAR (PETS 2004)-\\Videos\\mpg\\LeftBag.mpg";
				 glbl_var.videoStream = cvCreateFileCapture("D:\\Datasets\\CAVIAR (PETS 2004)-\\Videos\\mpg\\LeftBag.mpg");
				 glbl_var.videoReadingMethod = readFromVideo;
				 load_video();
			 }
	private: System::Void video7_btn_Click(System::Object^  sender, System::EventArgs^  e) {
				 glbl_var.filename = "D:\\Datasets\\CAVIAR (PETS 2004)-\\Videos\\mpg\\LeftBag_PickedUp.mpg";
				 glbl_var.videoStream = cvCreateFileCapture("D:\\Datasets\\CAVIAR (PETS 2004)-\\Videos\\mpg\\LeftBag_PickedUp.mpg");
				 glbl_var.videoReadingMethod = readFromVideo;
				 load_video();
			 }
	private: System::Void video8_btn_Click(System::Object^  sender, System::EventArgs^  e) {

				 if (glbl_var.ImageReadingExtension) delete [] glbl_var.ImageReadingExtension;
				 glbl_var.ImageReadingExtension = new char[100];
				 if (glbl_var.filename) delete [] glbl_var.filename;
				 glbl_var.filename = new char[100];

				 strcpy(glbl_var.ImageReadingExtension,".jpg");
				 strcpy(glbl_var.filename,"D:\\Datasets\\BEHAVE-\\main\\Jpegs\\all\\");
				 glbl_var.numberOfDigits = 8;
				 glbl_var.startFromFrame = 50375;
				 glbl_var.videoReadingMethod = readFromImages;
				 load_video();
			 }
	private: System::Void video9_btn_Click(System::Object^  sender, System::EventArgs^  e) {

				 if (glbl_var.ImageReadingExtension) delete [] glbl_var.ImageReadingExtension;
				 glbl_var.ImageReadingExtension = new char[100];
				 if (glbl_var.filename) delete [] glbl_var.filename;
				 glbl_var.filename = new char[100];

				 strcpy(glbl_var.ImageReadingExtension,".jpg");
				 strcpy(glbl_var.filename,"D:\\Datasets\\BEHAVE-\\main\\Jpegs\\all\\");
				 glbl_var.numberOfDigits = 8;
				 glbl_var.startFromFrame = 35466;
				 glbl_var.videoReadingMethod = readFromImages;
				 load_video();
			 }
	private: System::Void video10_btn_Click(System::Object^  sender, System::EventArgs^  e) {

				 if (glbl_var.ImageReadingExtension) delete [] glbl_var.ImageReadingExtension;
				 glbl_var.ImageReadingExtension = new char[100];
				  if (glbl_var.filename) delete [] glbl_var.filename;
				 glbl_var.filename = new char[100];

				 strcpy(glbl_var.ImageReadingExtension,".jpg");
				 strcpy(glbl_var.filename,"D:\\Datasets\\BEHAVE-\\main\\Jpegs\\all\\");
				 glbl_var.numberOfDigits = 8;
				 glbl_var.startFromFrame = 51260;
				 glbl_var.videoReadingMethod = readFromImages;
				 load_video();
			 }
	private: System::Void video11_btn_Click(System::Object^  sender, System::EventArgs^  e) {
				 glbl_var.filename = "D:\\Datasets\\CAVIAR (PETS 2004)-\\Videos\\mpg\\Meet_WalkSplit.mpg";
				 glbl_var.videoStream = cvCreateFileCapture("D:\\Datasets\\CAVIAR (PETS 2004)-\\Videos\\mpg\\Meet_WalkSplit.mpg");
				 glbl_var.videoReadingMethod = readFromVideo;
				 load_video();
			 }
	private: System::Void video12_btn_Click(System::Object^  sender, System::EventArgs^  e) {

				 if (glbl_var.ImageReadingExtension) delete [] glbl_var.ImageReadingExtension;
				 glbl_var.ImageReadingExtension = new char[100];
				  if (glbl_var.filename) delete [] glbl_var.filename;
				 glbl_var.filename = new char[100];

				 strcpy(glbl_var.ImageReadingExtension,".jpg");
				 strcpy(glbl_var.filename,"D:\\Datasets\\BEHAVE-\\main\\Jpegs\\all\\");
				 glbl_var.numberOfDigits = 8;
				 glbl_var.startFromFrame = 50359;
				 glbl_var.videoReadingMethod = readFromImages;
				 load_video();
			 }
	private: System::Void video13_btn_Click(System::Object^  sender, System::EventArgs^  e) {
				 glbl_var.filename = "D:\\Datasets\\CAVIAR (PETS 2004)-\\Videos\\mpg\\Rest_WiggleOnFloor.mpg";
				 glbl_var.videoStream = cvCreateFileCapture("D:\\Datasets\\CAVIAR (PETS 2004)-\\Videos\\mpg\\Rest_WiggleOnFloor.mpg");
				 glbl_var.videoReadingMethod = readFromVideo;
				 load_video();
			 }
	private: System::Void realtime_radio_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
				 if (realtime_radio->Checked)
				 {
					 glbl_var.realtimeProcessing =1;
					 //convert from seconds to frames if necessary
					 if (glbl_var.timeOrFrames == IN_TERMS_OF_FRAMES)
					 {
						 glbl_var.BGmodel->set_N(Single::Parse(N_txt->Text));
						 glbl_var.BGmodel->set_TM(Single::Parse(TM_txt->Text));
						 glbl_var.BGmodel->set_fmin(Single::Parse(fmin_txt->Text));
					 }
				 }
			 }
	private: System::Void frmbyfrm_radio_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
				 if (frmbyfrm_radio->Checked)
				 {
					 glbl_var.realtimeProcessing =0;
					 //convert from seconds to frames if necessary
					 if (glbl_var.timeOrFrames == IN_TERMS_OF_SECS)
					 {
						 glbl_var.BGmodel->set_N(glbl_var.sec_to_frame(glbl_var.BGmodel->get_N(),true));
						 glbl_var.BGmodel->set_TM(glbl_var.sec_to_frame(glbl_var.BGmodel->get_TM(),true));
						 glbl_var.BGmodel->set_fmin(glbl_var.sec_to_frame(glbl_var.BGmodel->get_fmin(),true));
					 }
					 else
					 {
						 glbl_var.BGmodel->set_N(Single::Parse(N_txt->Text));
						 glbl_var.BGmodel->set_TM(Single::Parse(TM_txt->Text));
						 glbl_var.BGmodel->set_fmin(Single::Parse(fmin_txt->Text));
					 }
				 }
			 }
	private: System::Void selectVideo_btn_Click(System::Object^  sender, System::EventArgs^  e) {
				 //openVideo_dlg->Filter = "AVI|*.avi";
				 openVideo_dlg->Title = "Select a video file to process";

				 if (openVideo_dlg->ShowDialog() == Windows::Forms::DialogResult::OK)
				 {
					 // Assign the cursor in the Stream to
					 // the Form's Cursor property.
					 const char* strtmp = (char*)(void*)Marshal::StringToHGlobalAnsi(openVideo_dlg->FileName);

					 if (glbl_var.filename) delete [] glbl_var.filename;
					 glbl_var.filename = new char[100];

					 strcpy(glbl_var.filename,strtmp);
					 glbl_var.videoStream = cvCreateFileCapture(glbl_var.filename);
					 load_video();
					 Marshal::FreeHGlobal((System::IntPtr)(void*)strtmp);
				 }

			 }
	private: System::Void radioButton8_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
			 }
	private: System::Void param_grp_Enter(System::Object^  sender, System::EventArgs^  e) {
			 }
	private: System::Void wantedSpeed_txt_TextChanged_1(System::Object^  sender, System::EventArgs^  e) {
			 }
	private: System::Void textBox2_TextChanged_1(System::Object^  sender, System::EventArgs^  e) {
			 }
	private: System::Void vid_rdb_CheckedChanged_1(System::Object^  sender, System::EventArgs^  e) {

			 }

	private: System::Void LoadFromimages_btn_Click(System::Object^  sender, System::EventArgs^  e) {

				 //assign the Images path, extension, and number of digits

				 const char* strtmp = (char*)(void*)Marshal::StringToHGlobalAnsi(LoadImagesExtension_txt->Text);
				 if (glbl_var.ImageReadingExtension) delete [] glbl_var.ImageReadingExtension;
				 glbl_var.ImageReadingExtension = new char[100];
				 strcpy(glbl_var.ImageReadingExtension,strtmp);
				 Marshal::FreeHGlobal((System::IntPtr)(void*)strtmp);

				 strtmp = (char*)(void*)Marshal::StringToHGlobalAnsi(LoadImagesPath_txt->Text);
				 if (glbl_var.filename) delete [] glbl_var.filename;
				 glbl_var.filename = new char[100];
				 strcpy(glbl_var.filename,strtmp);
				 Marshal::FreeHGlobal((System::IntPtr)(void*)strtmp);

				 glbl_var.numberOfDigits = (int) Single::Parse(LoadImagesNumberOfDigits_txt->Text);
				 glbl_var.startFromFrame = (int) Single::Parse(startfrom_txt->Text);

				 load_video();

			 }


	private: System::Void saveCB_btn_Click_1(System::Object^  sender, System::EventArgs^  e) {
			 }
	private: System::Void trackerEnables_chk_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
				 if (trackerEnables_chk->Enabled)
				 {
					 trackaftertrainingonly_chk->Enabled = true;
					 glbl_var.trackerEnabled = true;
				 }
				 else
				 {
					 trackaftertrainingonly_chk->Enabled = false;
					 glbl_var.trackerEnabled = false;
				 }
			 }
	private: System::Void trackaftertrainingonly_chk_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
				 if (trackaftertrainingonly_chk->Enabled)
					 glbl_var.trackerEnabledOnlyAfterTraining = true;
				 else
					 glbl_var.trackerEnabledOnlyAfterTraining = false;
			 }
	private: System::Void btn_halt_Click(System::Object^  sender, System::EventArgs^  e) {
				 glbl_var.video_finished = true;
				 resetControls();
			 }


	private: System::Void showTrackerLabelInfo_btn_Click(System::Object^  sender, System::EventArgs^  e) {

			 }
	private: System::Void inTermsOfFrames_radio_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
				 glbl_var.timeOrFrames = IN_TERMS_OF_FRAMES;
			 }
	private: System::Void inTermsOfTime_radio_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
				 glbl_var.timeOrFrames = IN_TERMS_OF_SECS;
			 }
	private: System::Void saveFrames_rdo_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
				 if(saveFrames_rdo->Checked)
				 {
					 glbl_var.saveFramesToHard = true;
					 /*					 strcpy(glbl_var.saveFramesTo,(char*)(void*)Marshal::StringToHGlobalAnsi(saveFrames_txt->Text));
					 srand(time(0));
					 char buffer[100];
					 strcat(glbl_var.saveFramesTo,itoa(rand()%1000000,buffer,10));
					 strcat(glbl_var.saveFramesTo,"\\");
					 mkdir(glbl_var.saveFramesTo);
					 */			 }
				 else
					 glbl_var.saveFramesToHard = false;
			 }
	private: System::Void tableColorConversion_chk_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {

			 }
	private: System::Void homomorphic_chk_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
				 if (homomorphic_chk->Checked)
					 glbl_var.useHomomorphic = true;
				 else
					 glbl_var.useHomomorphic = false;

			 }
	private: System::Void smooth_chk_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
				 if (smooth_chk->Checked)
					 glbl_var.GaussianSmooth = true;
				 else
					 glbl_var.GaussianSmooth = false;
			 }
	private: System::Void useMorphology_chk_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
				 if (useMorphology_chk->Checked)
					 glbl_var.useMorphology = true;
				 else
					 glbl_var.useMorphology = false;
			 }
	private: System::Void ObjProperties_lbl_MouseClick(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e) {

				 char input[10000];
				 strcpy(input,"Objects\r\n\r\n");
				 list<TrackerObject*>::iterator ptrObj;
				 list<TrackerObject*>::iterator ptrObj2;
				 list<TrackerObject*>::iterator ptrObj3;
				 char buffer[100];

				 for(ptrObj= glbl_var.BGmodel->trackerBlobs.begin(); ptrObj != glbl_var.BGmodel->trackerBlobs.end(); ptrObj++)
				 {
					 char object[1000];
					 strcpy(object,"Object ");
					 strcat(object, itoa((*ptrObj)->blob->ID,buffer,10));
					 if (!(*ptrObj)->isVisible)
						 strcat(object, "(Not Visible)");


					 //position
					 strcat(object, "\r\nx = " );
					 strcat(object, itoa((*ptrObj)->blob->x,buffer,10) );
					 strcat(object, ", y = " );
					 strcat(object, itoa((*ptrObj)->blob->y,buffer,10) );
					 strcat(object, ", w = " );
					 strcat(object, itoa((*ptrObj)->blob->w,buffer,10) );
					 strcat(object, ", h = " );
					 strcat(object, itoa((*ptrObj)->blob->h,buffer,10) );

					 //prediction
					 CvBlob tempBlob = (*ptrObj)->predictedPosition();
					 strcat(object, "\r\nx predicted = " );
					 strcat(object, itoa(tempBlob.x,buffer,10) );
					 strcat(object, ", y predicted = " );
					 strcat(object, itoa(tempBlob.y,buffer,10) );

					 //time
					 strcat(object, "\r\npresent since " );
					 strcat(object, itoa((*ptrObj)->time,buffer,10) );
					 if((*ptrObj)->absence != -1)
					 {
						 strcat(object, ", has been absent since " );
						 strcat(object, itoa((*ptrObj)->absence,buffer,10) );
					 }

					 //moment
					 sprintf(buffer,"%.2f",(*ptrObj)->moment);
					 strcat(object, "\r\nmoment = " );
					 strcat(object, buffer);
					 strcat(object, ", type = " );
					 //strcat(object, itoa((*ptrObj)->type,buffer,10) );
					 strcat(object,((*ptrObj)->type) ? "P": "B");

					 //CC
					 strcat(object, "\r\n" );
					 if ((*ptrObj)->matched)
					 {
						 strcat(object, "matched to CC# "); 
						 strcat(object, itoa((*ptrObj)->CC->blob->ID,buffer,10) );
						 strcat(object, ", color distance from match = " );
						 sprintf(buffer,"%.2f",(*ptrObj)->distanceFromCC);
						 strcat(object, buffer);
					 }
					 else
					 {
						 strcat(object,"Not matched to any CC");
					 }

					 //occlusion
					 strcat(object, "\r\n" );
					 if (!(*ptrObj)->occlusion_list.empty())
					 {
						 strcat(object, "Occlusion Group = " );
						 strcat(object, itoa((*ptrObj)->occlusion,buffer,10) );
						 if (!(*ptrObj)->occlusion_child)
						 {
							 strcat(object, "includes objects " );
							 for(ptrObj2= (*ptrObj)->occlusion_list.begin(); ptrObj2 != (*ptrObj)->occlusion_list.end(); ptrObj2++) //objects childran
							 {
								 strcat(object,itoa((*ptrObj2)->blob->ID,buffer,10));
								 strcat(object, "," );
							 }
						 }
					 }
					 else
					 {
						 strcat(object, "Not participating in occlusion" );
					 }

					 // if(glbl_var.CameraCalibrated)
					 //{
					 //Base coordinates
					 strcat(object, "\r\nLast reliable possibilities: " );
					 list<historyPoint*>::iterator itrHP;
					 list<motionPossibility*>::iterator itrPossibility;
					 int dummy_counter = 0;
					 for(itrHP = (*ptrObj)->lastHistoryMeasurement->begin(); itrHP != (*ptrObj)->lastHistoryMeasurement->end(); itrHP++)
					 {
						 strcat(object, "\r\nBase Coordinates = " );
						 sprintf(buffer,"%.2f",(*itrHP)->xBase);
						 strcat(object, buffer);
						 strcat(object, " , " );
						 sprintf(buffer,"%.2f",(*itrHP)->yBase);
						 strcat(object, buffer);

						 //head to base distance
						 strcat(object, "\r\nHead_to_feet = " );
						 float head_feet_distance_2 =  ( (*itrHP)->xTop - (*itrHP)->xBase )*( (*itrHP)->xTop - (*itrHP)->xBase ) + ( (*itrHP)->yTop - (*itrHP)->yBase )*( (*itrHP)->yTop - (*itrHP)->yBase );
						 sprintf(buffer,"%.2f",cvSqrt( head_feet_distance_2));
						 strcat(object,buffer);


						 for(itrPossibility = (*itrHP)->motionPossibilities.begin(); itrPossibility != (*itrHP)->motionPossibilities.end(); itrPossibility++)
						 {

							 strcat(object, "\r\nPossibility " );
							 strcat(object, itoa(dummy_counter++,buffer,10));



							 //Base velocity
							 strcat(object, "\r\nBase velocity = " );
							 sprintf(buffer,"%.2f",(*itrPossibility)->vxBase);
							 strcat(object, buffer);
							 strcat(object, " , " );
							 sprintf(buffer,"%.2f",(*itrPossibility)->vyBase);
							 strcat(object, buffer);

							 //Speed
							 strcat(object, "\r\nBase speed = " );
							 sprintf(buffer,"%.2f",(*itrPossibility)->speed);
							 strcat(object, buffer);


							 //Direction
							 strcat(object, "\r\nBase Direction = " );
							 sprintf(buffer,"%.2f",(*itrPossibility)->xDirection);
							 strcat(object, buffer);
							 strcat(object, " , " );
							 sprintf(buffer,"%.2f",(*itrPossibility)->yDirection);
							 strcat(object, buffer);
							 // }


						 }

					 }






					 ////moving object ?
					 //strcat(object, "\r\n" );
					 //strcat(object, ((*ptrObj)->movingObject ? "is a moving object" : " is not a moving object"));

					 //object classification
					 strcat(object, "\r\n" );
					 strcat(object,( (*ptrObj)->object_classification == UNKNOWN ? "U" :((*ptrObj)->object_classification == PERSON ? "P" : ((*ptrObj)->object_classification == STILL_PERSON ? "SP" : "O" )) ));

					 //owner
					 if((*ptrObj)->object_classification == OBJECT)
					 {
						 strcat(object, "\r\nowner is Object ID " );
						 strcat(object, ( (*ptrObj)->belongs_to ? itoa((*ptrObj)->belongs_to->blob->ID,buffer, 10) : "NULL" ));
					 }


					 //concatenate
					 strcat(object, "\r\n\r\n\r\n" );
					 strcat(input ,object);

				 }

				 //write to the textbox
				 ObjProperties_txt->Text = gcnew String(input);
			 }


	private: System::Void CCProperties_txt_MouseClick(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e) {
				 char input[10000];
				 strcpy(input,"CCs\r\n\r\n");
				 list<ConnectedComponent*>::iterator ptrCC;
				 list<TrackerObject*>::iterator ptrObj2;
				 char buffer[100];

				 for(ptrCC= glbl_var.BGmodel->BGSubtractionBlobs.begin(); ptrCC != glbl_var.BGmodel->BGSubtractionBlobs.end(); ptrCC++)
				 {
					 char object[1000];
					 strcpy(object,"CC ");
					 strcat(object, itoa((*ptrCC)->blob->ID,buffer,10));



					 //position
					 strcat(object, "\r\nx = " );
					 strcat(object, itoa((*ptrCC)->blob->x,buffer,10) );
					 strcat(object, ", y = " );
					 strcat(object, itoa((*ptrCC)->blob->y,buffer,10) );
					 strcat(object, ", w = " );
					 strcat(object, itoa((*ptrCC)->blob->w,buffer,10) );
					 strcat(object, ", h = " );
					 strcat(object, itoa((*ptrCC)->blob->h,buffer,10) );


					 //Object
					 for (ptrObj2 = (*ptrCC)->objects.begin(); ptrObj2 != (*ptrCC)->objects.end(); ptrObj2++)
					 {
						 strcat(object, "\r\nobj ID = " );
						 strcat(object, itoa((*ptrObj2)->blob->ID,buffer,10)  );
					 }


					 //distance
					 strcat(object, "\r\nColor distance from the match = " );
					 sprintf(buffer,"%.2f",(*ptrCC)->distanceFromCC);
					 strcat(object, buffer);

					 if(glbl_var.CameraCalibrated)
					 {
						 //Base coordinates
						 strcat(object, "\r\nBase Coordinates = " );
						 sprintf(buffer,"%.2f",(*ptrCC)->xBase);
						 strcat(object, buffer);
						 strcat(object, " , " );
						 sprintf(buffer,"%.2f",(*ptrCC)->yBase);
						 strcat(object, buffer);
					 }

					 //ghost
					 strcat(object,"\r\n");
					 if ((*ptrCC)->ghost) strcat(object, "ghost\n");
					 strcat(object,"color distance from surrounding is ");
					 sprintf(buffer,"%.2f",(*ptrCC)->contour_distance);
					 strcat(object, buffer);

					 //concatenate
					 strcat(object, "\r\n\r\n\r\n" );
					 strcat(input ,object);


				 }


				 CCProperties_txt->Text = gcnew String(input);
			 }
	private: System::Void traintest_chk_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
				 if (traintest_chk->Checked)
					 glbl_var.seperateTrainTest = true;
				 else
					 glbl_var.seperateTrainTest = false;
			 }
	private: System::Void ResizeProcessedbydummy_lbl_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {

				 if (ResizeProcessedbydummy_lbl->Checked)
					 glbl_var.resize_processing_factor = true;
				 else
					 glbl_var.resize_processing_factor = false;
			 }

	private: System::Void ObjProperties_txt_TextChanged(System::Object^  sender, System::EventArgs^  e) {
			 }

	private: System::Void debug_monitor_chk_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
				 if (debug_monitor_chk->Checked)
				 {
					 show_speed_height_chk->Enabled = true;
					 Show_prediction_chk->Enabled = true;
					 show_dummy_chk->Enabled = true;
					 show_blob_chk->Enabled = true;
					 faint_chk->Enabled = true;
					 glbl_var.debugScreen = true;
				 }
				 else
				 {
					 show_speed_height_chk->Checked = false;
					 Show_prediction_chk->Checked = false;
					 show_dummy_chk->Checked = false;
					 show_speed_height_chk->Enabled = false;
					 Show_prediction_chk->Enabled = false;
					 show_dummy_chk->Enabled = false;
					 show_blob_chk->Enabled = false;
					 faint_chk->Enabled = false;
					 glbl_var.debugScreen = false;
					 glbl_var.debugPrediction = false;
					 glbl_var.debugSpeed = false;
					 glbl_var.debugDummy = false;
					 glbl_var.debugBlob = false;
					 glbl_var.debugFaint = false;
				 }
			 }
	private: System::Void camera_chk_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
				 if(camera_chk->Checked)
				 {
					 glbl_var.CameraCalibrated = true;
					 glbl_var.cam = new Etiseo::CameraModel;
					 Etiseo::UtilXml::Init();
					 ifstream is;
					 is.open("camera.xml");
					 glbl_var.cam->fromXml(is);
					 is.close();
					 glbl_var.camCalXRatio = glbl_var.cam->width()/((float)glbl_var.BGmodel->get_w());
					 glbl_var.camCalYRatio = glbl_var.cam->height()/((float)glbl_var.BGmodel->get_h());



				 }
				 else
				 {
					 glbl_var.CameraCalibrated = false;
					 if (glbl_var.cam) 
					 {
						 delete glbl_var.cam;
						 glbl_var.cam = NULL;
					 }

				 }
			 }
	private: System::Void desiredfps_txt_TextChanged(System::Object^  sender, System::EventArgs^  e) {

			 }
	private: System::Void desiredfps_txt_Enter(System::Object^  sender, System::EventArgs^  e) {

			 }
	private: System::Void desiredfps_txt_KeyPress(System::Object^  sender, System::Windows::Forms::KeyPressEventArgs^  e) {
				 if( e->KeyChar == (char)Keys::Enter)
				 {
					 if (frmbyfrm_radio->Checked)
					 {
						 glbl_var.numberOfFramesToSkip = Single::Parse(desiredfps_txt->Text) - 1;

						 glbl_var.BGmodel->set_N((float)glbl_var.BGmodel->get_N()/(glbl_var.numberOfFramesToSkip+1));
						 glbl_var.BGmodel->set_TM((float)glbl_var.BGmodel->get_TM()/(glbl_var.numberOfFramesToSkip+1));
						 glbl_var.BGmodel->set_fmin((float)glbl_var.BGmodel->get_fmin()/(glbl_var.numberOfFramesToSkip+1));



					 }
					 desiredfps_txt->Enabled = false;
				 }
			 }
	private: System::Void BehaviourRecognition_chk_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
				 if (BehaviourRecognition_chk->Checked)
				 {
					 glbl_var.behaviourEnabled = true;
				 }
				 else
				 {
					 glbl_var.behaviourEnabled = false;
				 }
			 }
	private: System::Void TrackerTime_lbl_Click(System::Object^  sender, System::EventArgs^  e) {
			 }
	private: System::Void oneObjectHist_btn_Click(System::Object^  sender, System::EventArgs^  e) {

				 int objectID = Single::Parse(ObjectBehaviour_txt->Text);

				 char info[100000];
				 char buffer[10];
				 strcpy(info,"Object");
				 strcat(info,itoa(objectID,buffer,10));
				 strcat(info,"\r\n\r\n");

				 list<TrackerObject*>::iterator ObjPtr;
				 list<list<historyPoint*>*>::iterator ptrHistoryLine;
				 list<historyPoint*>::iterator ptrHistoryPoint;
				 list<motionPossibility*>::iterator ptrPossibility;






				 for(ObjPtr = glbl_var.BGmodel->trackerBlobs.begin() ;ObjPtr != glbl_var.BGmodel->trackerBlobs.end() ; ObjPtr++) 
				 {


					 //find the designated object
					 if((*ObjPtr)->blob->ID != objectID) continue;

					 list<historyPoint*>* lastLine = (*ObjPtr)->lastHistoryMeasurement;

					 //print last measurement
					 strcat(info,"Last History:\r\n");
					 if(!lastLine)
					 {
						 strcat(info,"empty history line\r\n");
					 }
					 else
					 {
						 //t and frameNumber
						 strcat(info,"time = ");
						 sprintf(buffer, "%.2f", (*(lastLine->begin()))->time);
						 strcat(info,buffer);
						 strcat(info," ,frameNumber = ");
						 strcat(info,itoa((*(lastLine->begin()))->frameNumber,buffer,10));
						 strcat(info,"\r\n");
					 }



					 for(ptrHistoryPoint = lastLine->begin() ; ptrHistoryPoint != lastLine->end() ;ptrHistoryPoint++ )
					 {

						 //position
						 strcat(info,"-position = ");
						 sprintf(buffer, "%.2f", (*ptrHistoryPoint)->xBase);
						 strcat(info,buffer);
						 strcat(info," , ");
						 sprintf(buffer, "%.2f",(*ptrHistoryPoint)->yBase);
						 strcat(info,buffer);
						 strcat(info,"\r\n");


						 for(ptrPossibility = (*ptrHistoryPoint)->motionPossibilities.begin() ; ptrPossibility != (*ptrHistoryPoint)->motionPossibilities.end() ;ptrPossibility++ )
						 {
							 //possibility
							 strcat(info,"--possibility:");
							 sprintf(buffer, "%.2f", (*ptrPossibility)->vxBase);
							 strcat(info,buffer);
							 strcat(info," , ");
							 sprintf(buffer, "%.2f", (*ptrPossibility)->vyBase);
							 strcat(info,buffer);
							 strcat(info,"\r\n");


						 }
					 }

					 strcat(info,"-----------\r\n");
					 strcat(info,"\r\n");

					 //print history
					 strcat(info,"History:\r\n");
					 for(ptrHistoryLine = (*ObjPtr)->history.begin() ; ptrHistoryLine != (*ObjPtr)->history.end() ;ptrHistoryLine++ )
					 {


						 if((*ptrHistoryLine)->empty())
						 {
							 strcat(info,"empty history line\r\n");
						 }
						 else
						 {
							 //t and frameNumber
							 strcat(info,"time = ");
							 sprintf(buffer, "%.2f", (*(*ptrHistoryLine)->begin())->time);
							 strcat(info,buffer);
							 strcat(info," ,frameNumber = ");
							 strcat(info,itoa((*(*ptrHistoryLine)->begin())->frameNumber,buffer,10));
							 strcat(info,"\r\n");
						 }



						 for(ptrHistoryPoint = (*ptrHistoryLine)->begin() ; ptrHistoryPoint != (*ptrHistoryLine)->end() ;ptrHistoryPoint++ )
						 {

							 //position
							 strcat(info,"-position = ");
							 sprintf(buffer, "%.2f", (*ptrHistoryPoint)->xBase);
							 strcat(info,buffer);
							 strcat(info," , ");
							 sprintf(buffer, "%.2f",(*ptrHistoryPoint)->yBase);
							 strcat(info,buffer);
							 strcat(info,"\r\n");

							 for(ptrPossibility = (*ptrHistoryPoint)->motionPossibilities.begin() ; ptrPossibility != (*ptrHistoryPoint)->motionPossibilities.end() ;ptrPossibility++ )
							 {
								 //possibility
								 strcat(info,"--possibility:");
								 sprintf(buffer, "%.2f", (*ptrPossibility)->vxBase);
								 strcat(info,buffer);
								 strcat(info," , ");
								 sprintf(buffer, "%.2f", (*ptrPossibility)->vyBase);
								 strcat(info,buffer);
								 strcat(info,"\r\n");


							 }
						 }

						 strcat(info,"\r\n");
					 }


					 break; //object has been found. no need to traverse the others




				 }

				 //write to the textbox
				 SingleObjBehaviour_txt->Text = gcnew String(info);
			 }
	private: System::Void loadInitialMask_chk_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
				 if(glbl_var.initialMaskFrame)
					 glbl_var.initialMaskFrame = false;
				 else
					 glbl_var.initialMaskFrame = true;
			 }
private: System::Void Show_prediction_chk_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {

			  if (Show_prediction_chk->Checked && glbl_var.debugScreen)
					 glbl_var.debugPrediction = true;
				 else
					 glbl_var.debugPrediction = false;
		 }
private: System::Void show_speed_height_chk_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
			  if (show_speed_height_chk->Checked && glbl_var.debugScreen)
					 glbl_var.debugSpeed = true;
				 else
					 glbl_var.debugSpeed = false;
		 }
private: System::Void show_dummy_chk_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
			  if (show_dummy_chk->Checked && glbl_var.debugScreen)
					 glbl_var.debugDummy = true;
				 else
					 glbl_var.debugDummy = false;
		 }
private: System::Void show_blob_chk_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
			 if (show_blob_chk->Checked && glbl_var.debugScreen)
				 glbl_var.debugBlob = true;
			 else
				 glbl_var.debugBlob = false;
		 }
private: System::Void ghost_chk_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {

			 if (ghost_chk->Checked)
			 {
				 glbl_var.ghostEnabled = true;
			 }
			 else
			 {
				 glbl_var.ghostEnabled = false;
			 }
		 }
private: System::Void useHistDifference_chk_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {

			  if (useHistDifference_chk->Checked)
			 {
				 glbl_var.useHistDifference = true;
			 }
			 else
			 {
				 glbl_var.useHistDifference = false;
			 }
		 }
private: System::Void faint_chk_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
			 if (faint_chk->Checked && glbl_var.debugScreen)
				 glbl_var.debugFaint = true;
			 else
				 glbl_var.debugFaint = false;
		 }
private: System::Void dummyMultipleObjectBehaviour_grp_Enter(System::Object^  sender, System::EventArgs^  e) {
		 }
private: System::Void twoObjectsHist_btn_Click(System::Object^  sender, System::EventArgs^  e) {
		 }
private: System::Void button1_Click(System::Object^  sender, System::EventArgs^  e) {
			 glbl_var.filename = "D:\\Datasets\\BEHAVE-\\main\\AVI\\67210-76800.avi";
			 glbl_var.videoStream = cvCreateFileCapture("D:\\Datasets\\BEHAVE-\\main\\AVI\\67210-76800.avi");
			 glbl_var.videoReadingMethod = readFromVideo;
			 load_video();
		 }
private: System::Void button2_Click_1(System::Object^  sender, System::EventArgs^  e) {
			 glbl_var.filename = "D:\\Datasets\\BEHAVE-\\main\\AVI\\67210-76800.avi";
			 glbl_var.videoStream = cvCreateFileCapture("D:\\Datasets\\CAVIAR (PETS 2004)-\\Videos\\mpg\\67210-76800.avi");
			 glbl_var.videoReadingMethod = readFromVideo;
			 load_video();
		 }
private: System::Void button3_Click(System::Object^  sender, System::EventArgs^  e) {
			 if (glbl_var.ImageReadingExtension) delete [] glbl_var.ImageReadingExtension;
			 glbl_var.ImageReadingExtension = new char[100];
			 if (glbl_var.filename) delete [] glbl_var.filename;
			 glbl_var.filename = new char[100];

			 strcpy(glbl_var.ImageReadingExtension,".jpeg");
			 strcpy(glbl_var.filename,"D:\\Datasets\\PETS 2006-\\S1-T1-C\\S1-T1-C\\video\\pets2006\\S1-T1-C\\3\\S1-T1-C.");
			 glbl_var.numberOfDigits = 5;
			 glbl_var.startFromFrame = 0;
			 glbl_var.videoReadingMethod = readFromImages;
			 load_video();
		 }
private: System::Void button5_Click(System::Object^  sender, System::EventArgs^  e) {
			  if (glbl_var.ImageReadingExtension) delete [] glbl_var.ImageReadingExtension;
			 glbl_var.ImageReadingExtension = new char[100];
			 if (glbl_var.filename) delete [] glbl_var.filename;
			 glbl_var.filename = new char[100];

			 strcpy(glbl_var.ImageReadingExtension,".jpeg");
			 strcpy(glbl_var.filename,"D:\\Datasets\\PETS 2006-\\S2-T3-C\\S2-T3-C\\video\\pets2006\\S2-T3-C\\3\\S2-T3-C.");
			 glbl_var.numberOfDigits = 5;
			 glbl_var.startFromFrame = 0;
			 glbl_var.videoReadingMethod = readFromImages;
			 load_video();
		 }
private: System::Void button6_Click(System::Object^  sender, System::EventArgs^  e) {
			  if (glbl_var.ImageReadingExtension) delete [] glbl_var.ImageReadingExtension;
			 glbl_var.ImageReadingExtension = new char[100];
			 if (glbl_var.filename) delete [] glbl_var.filename;
			 glbl_var.filename = new char[100];

			 strcpy(glbl_var.ImageReadingExtension,".jpeg");
			 strcpy(glbl_var.filename,"D:\\Datasets\\PETS 2006-\\S3-T7-A\\S3-T7-A\\video\\pets2006\\S3-T7-A\\3\\S3-T7-A.");
			 glbl_var.numberOfDigits = 5;
			 glbl_var.startFromFrame = 0;
			 glbl_var.videoReadingMethod = readFromImages;
			 load_video();
		 }
private: System::Void button9_Click_1(System::Object^  sender, System::EventArgs^  e) {
			   if (glbl_var.ImageReadingExtension) delete [] glbl_var.ImageReadingExtension;
			 glbl_var.ImageReadingExtension = new char[100];
			 if (glbl_var.filename) delete [] glbl_var.filename;
			 glbl_var.filename = new char[100];

			 strcpy(glbl_var.ImageReadingExtension,".jpeg");
			 strcpy(glbl_var.filename,"D:\\Datasets\\PETS 2006-\\S4-T5-A\\S4-T5-A\\video\\pets2006\\S4-T5-A\\3\\S4-T5-A.");
			 glbl_var.numberOfDigits = 5;
			 glbl_var.startFromFrame = 0;
			 glbl_var.videoReadingMethod = readFromImages;
			 load_video();
		 }
private: System::Void button8_Click_1(System::Object^  sender, System::EventArgs^  e) {
			 if (glbl_var.ImageReadingExtension) delete [] glbl_var.ImageReadingExtension;
			 glbl_var.ImageReadingExtension = new char[100];
			 if (glbl_var.filename) delete [] glbl_var.filename;
			 glbl_var.filename = new char[100];

			 strcpy(glbl_var.ImageReadingExtension,".jpeg");
			 strcpy(glbl_var.filename,"D:\\Datasets\\PETS 2006-\\S5-T1-G\\S5-T1-G\\video\\pets2006\\S5-T1-G\\3\\S5-T1-G.");
			 glbl_var.numberOfDigits = 5;
			 glbl_var.startFromFrame = 0;
			 glbl_var.videoReadingMethod = readFromImages;
			 load_video();
		 }
private: System::Void button7_Click_1(System::Object^  sender, System::EventArgs^  e) {
			  if (glbl_var.ImageReadingExtension) delete [] glbl_var.ImageReadingExtension;
			 glbl_var.ImageReadingExtension = new char[100];
			 if (glbl_var.filename) delete [] glbl_var.filename;
			 glbl_var.filename = new char[100];

			 strcpy(glbl_var.ImageReadingExtension,".jpeg");
			 strcpy(glbl_var.filename,"D:\\Datasets\\PETS 2006-\\S6-T3-H\\S6-T3-H\\video\\pets2006\\S6-T3-H\\3\\S6-T3-H.");
			 glbl_var.numberOfDigits = 5;
			 glbl_var.startFromFrame = 0;
			 glbl_var.videoReadingMethod = readFromImages;
			 load_video();
		 }
private: System::Void button4_Click_1(System::Object^  sender, System::EventArgs^  e) {
			   if (glbl_var.ImageReadingExtension) delete [] glbl_var.ImageReadingExtension;
			 glbl_var.ImageReadingExtension = new char[100];
			 if (glbl_var.filename) delete [] glbl_var.filename;
			 glbl_var.filename = new char[100];

			 strcpy(glbl_var.ImageReadingExtension,".jpeg");
			 strcpy(glbl_var.filename,"D:\\Datasets\\PETS 2006-\\S7-T6-B\\S7-T6-B\\video\\pets2006\\S7-T6-B\\3\\S7-T6-B.");
			 glbl_var.numberOfDigits = 5;
			 glbl_var.startFromFrame = 0;
			 glbl_var.videoReadingMethod = readFromImages;
			 load_video();
		 }
private: System::Void button10_Click_1(System::Object^  sender, System::EventArgs^  e) {
		 }
private: System::Void button10_Click_2(System::Object^  sender, System::EventArgs^  e) {
			  glbl_var.filename = "D:\\Datasets\\AVSS 2007 i-LIDS-\\AVSS_AB_Easy_Divx.avi";
			 glbl_var.videoStream = cvCreateFileCapture("D:\\Datasets\\AVSS 2007 i-LIDS-\\AVSS_AB_Easy_Divx.avi");
			 glbl_var.videoReadingMethod = readFromVideo;
			 load_video();
		 }
private: System::Void button11_Click(System::Object^  sender, System::EventArgs^  e) {
			   glbl_var.filename = "D:\\Datasets\\AVSS 2007 i-LIDS-\\AVSS_AB_Medium_Divx.avi";
			 glbl_var.videoStream = cvCreateFileCapture("D:\\Datasets\\AVSS 2007 i-LIDS-\\AVSS_AB_Medium_Divx.avi");
			 glbl_var.videoReadingMethod = readFromVideo;
			 load_video();
		 }
private: System::Void button11_Click_1(System::Object^  sender, System::EventArgs^  e) {
			  if (glbl_var.ImageReadingExtension) delete [] glbl_var.ImageReadingExtension;
			 glbl_var.ImageReadingExtension = new char[100];
			 if (glbl_var.filename) delete [] glbl_var.filename;
			 glbl_var.filename = new char[100];

			 strcpy(glbl_var.ImageReadingExtension,".jpeg");
			 strcpy(glbl_var.filename,"D:\\Datasets\\PETS 2006-\\S1-T1-C\\S1-T1-C\\video\\pets2006\\S1-T1-C\\4\\S1-T1-C.");
			 glbl_var.numberOfDigits = 5;
			 glbl_var.startFromFrame = 0;
			 glbl_var.videoReadingMethod = readFromImages;
			 load_video();
		 }
private: System::Void button11_Click_2(System::Object^  sender, System::EventArgs^  e) {
			    if (glbl_var.ImageReadingExtension) delete [] glbl_var.ImageReadingExtension;
			 glbl_var.ImageReadingExtension = new char[100];
			 if (glbl_var.filename) delete [] glbl_var.filename;
			 glbl_var.filename = new char[100];

			 strcpy(glbl_var.ImageReadingExtension,".jpg");
			 strcpy(glbl_var.filename,"D:\\Datasets\\PETS 2007-\\s02\\S02-GENERAL_LOITERING_2\\3\\thirdView");
			 glbl_var.numberOfDigits = 6;
			 glbl_var.startFromFrame = 0;
			 glbl_var.videoReadingMethod = readFromImages;
			 load_video();
		 }
private: System::Void button12_Click(System::Object^  sender, System::EventArgs^  e) {
			 
				 glbl_var.filename = "D:\\Datasets\\CAVIAR (PETS 2004)-\\Videos\\mpg\\Browse_WhileWaiting2.mpg";
				 glbl_var.videoStream = cvCreateFileCapture("D:\\Datasets\\CAVIAR (PETS 2004)-\\Videos\\mpg\\Browse_WhileWaiting2.mpg");
				 glbl_var.videoReadingMethod = readFromVideo;

				 /* glbl_var.filename = "D:\\Datasets\\PETS 2007\\s01\\S01_GENERAL_LOITERING_1\\3\\thirdView";
				 glbl_var.videoStream = cvCreateFileCapture("D:\\Datasets\\PETS 2007\\s01\\S01_GENERAL_LOITERING_1\\3\\thirdView");
				 glbl_var.ImageReadingExtension = ".jpg";
				 glbl_var.numberOfDigits = 6;*/
				 load_video();

		 }
};



}





