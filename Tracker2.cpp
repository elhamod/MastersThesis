#include "stdafx.h"
#include "g_variables.h"
#include <cv.h>
#include <cvaux.h>
#include <highgui.h>



TrackerObject::TrackerObject()
{
	blob = new CvBlob(cvBlob(-1,-1,-1,-1));// an invalid blob
	prev_blob = new CvBlob(cvBlob(-1,-1,-1,-1));// an invalid blob
	time = 0;
	persistence = 0;
	absence = 0;
	hist = 0;
	kalman = 0;
	moment =0;
	isVisible = 0;
	matched = false;
	occlusion = 0;
	CC = NULL;
	occlusion_child = false;
	distanceFromCC = double::MaxValue;
	splitIndex = -1;
	persistent = false;

	toBeDeleted = false;
	updateCandidate = NULL;
	updateCandidate_colorDistance = double::MaxValue;
	updateCandidate_PersonGroup = 0;
	updateCandidate_object_classification = UNKNOWN;
	rearrangeParents = NULL;
	//object_updated_img = NULL;

	movingObject = false; //it is a non-moving object unless proven otherwise

	lastHistoryMeasurement = NULL;
	currentHistoryMeasurement = NULL;
	//candidateResolveHistoryParentList = NULL;


	belongs_to = NULL;
	//displayedText = NULL;
	walk = false;
	meet = false;
	faint = false;
	steal = false;
	abandoned = false;
	loiter = false;
	fight = false;

	ownerHist = NULL;
	blobMask = NULL;

}

TrackerObject::~TrackerObject()
{
	delete blob;
	delete prev_blob;

	//list<historyPoint*>::iterator ptrObj;

	if (hist) cvReleaseHist(&hist);
	if (ownerHist) cvReleaseHist(&ownerHist);
	if (kalman) cvReleaseKalman(&kalman);
	if(blobMask) cvReleaseImage(&blobMask);
	occlusion_list.clear();
	parent.clear();
	//for(ptrObj = history.begin(); ptrObj != history.end(); ptrObj++)
	//	(*ptrObj)->occlusion_IDs.clear();
	history.clear();
	//delete lastHistoryMeasurement;
	lastHistoryMeasurement->clear();
	//lastHistoryMeasurement = NULL;
	//delete currentHistoryMeasurement;
	currentHistoryMeasurement->clear();
	//currentHistoryMeasurement = NULL;
	//if (object_updated_img) cvReleaseImage(&object_updated_img);

	//candidateResolveHistoryParentList = NULL;
	candidateResolveHistoryParentList.clear();

	/*if (displayedText)
		delete[] displayedText;*/


}

ConnectedComponent::ConnectedComponent()
{
	blob = new CvBlob(cvBlob(-1,-1,-1,-1));// an invalid blob
	hist = 0;
	moment =0;
	distanceFromCC =0;
	blob_img = NULL;
	blob_mask = NULL;
	blob_inner_contour = NULL;
	blob_outer_contour = NULL;
	xBase = -1;
	yBase = -1;
	xTop = -1;
	yTop = -1;
	blob_img = NULL;
	blob_inner_contour = NULL;
	blob_outer_contour = NULL;
	blob_mask = NULL;
	matched = 0;
	centroid_x = -1;
	centroid_y = -1;

	contour_distance = -1; 
	ghost = false;
}

ConnectedComponent::~ConnectedComponent()
{
	delete blob;
	if (hist != NULL) 
	{
		cvReleaseHist(&hist);
		hist = 0;
	}
	if (blob_img) cvReleaseImage(&blob_img);
	if (blob_mask) cvReleaseImage(&blob_mask);
	if (blob_inner_contour) cvReleaseImage(&blob_inner_contour);
	if (blob_outer_contour) cvReleaseImage(&blob_outer_contour);

	objects.clear();
}

//ConnectedComponent::ConnectedComponent(const ConnectedComponent &copyin)
//{
//	blob = CvBlob(copyin);
//	cvCopyHist(copyin.hist,&hist);
//	matched = copyin.matched;
//	moment = copyin.moment;
//}

void TrackerObject::Predict()
{
	cvKalmanPredict(kalman);
}

CvBlob TrackerObject::predictedPosition()
{
	CvBlob rtn;
	rtn.x = kalman->PosterState[0];
	rtn.y = kalman->PosterState[1];
	rtn.w = blob->w;
	rtn.h = blob->h;

	return rtn;

}

void erase_object(TrackerObject* obj)
{
	list<TrackerObject*>::iterator ptrObj;
	list<TrackerObject*>::iterator ptrObj2;

	//dereference the children
	ptrObj= obj->occlusion_list.begin();
	while ( ptrObj != obj->occlusion_list.end())
	{
		if ((*ptrObj)->parent.size() == 1) 
		{
			/*(*ptrObj)->occlusion_child = false;
			(*ptrObj)->occlusion = 0;*/
			ptrObj2 = ptrObj;
			ptrObj++;
			erase_object((*ptrObj2));

		}
		else
		{
			(*ptrObj)->parent.remove(obj);
			 ptrObj++;
		}
		/*for (ptrObj2= (*ptrObj)->parent.begin(); ptrObj2 != (*ptrObj)->parent.end(); ptrObj2++)
		{
			
			if ((*ptrObj2) == obj)
			{
				(*ptrObj)->parent.erase(ptrObj2);



				break;
			}
		}*/
	}

	//remove from parents
	for (ptrObj= obj->parent.begin(); ptrObj != obj->parent.end(); ptrObj++)
	{
		(*ptrObj)->occlusion_list.remove(obj);
		/*for (ptrObj2= (*ptrObj)->occlusion_list.begin(); ptrObj2 != (*ptrObj)->occlusion_list.end(); ptrObj2++)
		{
			if ((*ptrObj2) == obj)
			{
				(*ptrObj)->occlusion_list.erase(ptrObj2);
				break;
			}
		}*/
	}


	//delete it
	glbl_var.BGmodel->trackerBlobs.remove(obj);
	//for (ptrObj= glbl_var.BGmodel->trackerBlobs.begin(); ptrObj != glbl_var.BGmodel->trackerBlobs.end(); ptrObj++)
	//{
	//	if ((*ptrObj) == obj)
	//	{

	//		//delete (*ptrObj);
	//		glbl_var.BGmodel->trackerBlobs.erase(ptrObj);
	//		//glbl_var.BehaviourDescriptor.deleteObject((*ptrObj));
	//		break;
	//	}
	//}

}

bool willCollide(list<TrackerObject*>::iterator obj,list<TrackerObject*>::iterator obj2)
{
	CvBlob blob1, blob2;
	blob1 = (*obj)->predictedPosition();
	blob2 = (*obj2)->predictedPosition();
	bool result = false;
	bool x_condition = false;
	bool y_condition = false;

	if ((blob1.x  - blob2.x < 1) && (blob1.x + blob1.w/2 - blob2.x + blob2.w/2> -1))
		x_condition= true;
	if ((blob2.x  - blob1.x < 1) && (blob2.x + blob2.w/2 - blob1.x + blob1.w/2> -1))
		x_condition= true;
	if ((blob1.y - blob2.y  < 1) && (blob1.y + blob1.h/2 - blob2.y + blob2.h/2> -1))
		y_condition= true;
	if ((blob2.y- blob1.y  < 1) && (blob2.y + blob2.h/2  - blob1.y + blob1.h/2> -1))
		y_condition= true;


	/*if ((blob1.x + blob1.w/2 - blob2.x - blob2.w/2 < 1) && (blob1.x + blob1.w - blob2.x > -1))
	x_condition= true;
	if ((blob2.x + blob2.w/2 - blob1.x - blob1.w/2 < 1) && (blob2.x + blob2.w - blob1.x > -1))
	x_condition= true;
	if ((blob1.y + blob1.h/2 - blob2.y - blob2.h/2 < 1) && (blob1.y + blob1.h - blob2.y > -1))
	y_condition= true;
	if ((blob2.y + blob2.h/2 - blob1.y - blob1.h/2 < 1) && (blob2.y + blob2.h - blob1.y > -1))
	y_condition= true;*/

	if (x_condition && y_condition) 
		result=true;

	return result;
}



//the majority of the object is covered by this blob
bool BG_codebook_model::pixelIntersect(TrackerObject* obj,ConnectedComponent* CC, bool usePrediction)
{
	int label=0;
	int best_label = 0;
	int label_area = 0;
	int temp_size = w*h/4;
	int* label_book;
	label_book = new int[temp_size];
	for (int i=0; i <temp_size; i++)
		label_book[i] = 0;
	for (int cnt_x = ( usePrediction ? obj->predictedPosition().x : obj->blob->x ) - (usePrediction ? obj->predictedPosition().w/2 : obj->blob->w/2) ; cnt_x < ( usePrediction ? obj->predictedPosition().x : obj->blob->x) + ( usePrediction ? obj->predictedPosition().w/2 : obj->blob->w/2) ;cnt_x++)
	{
		for (int cnt_y = ( usePrediction ? obj->predictedPosition().y : obj->blob->y ) - (usePrediction ? obj->predictedPosition().h/2 : obj->blob->h/2) ; cnt_y < ( usePrediction ? obj->predictedPosition().y : obj->blob->y) + ( usePrediction ? obj->predictedPosition().h/2 : obj->blob->h/2) ;cnt_y++)
		{	


			if (cnt_y > h-1)
				continue;

			if (cnt_y < 0)
				continue;


			if (cnt_x > w-1)
				continue;

			if (cnt_x < 0)
				continue;

			label = glbl_var.BGlabels[cnt_x + w*cnt_y];
			label_book[abs(label)]++;


			if (label_book[abs(label)] > label_area && label>0)
			{
				best_label = label;
				label_area = label_book[label];
			}
		}
	}

	delete[] label_book;

	if (best_label == CC->blob->ID)
		return true;
	else
		return false;
}

//the majority of the object is covered by this blob
float BG_codebook_model::pixelIntersectratio(TrackerObject* obj,ConnectedComponent* CC, bool usePrediction)
{
	int label_area = 0;
	int label;
	//float Objarea = (usePrediction ? obj->predictedPosition().h : obj->blob->h)*(usePrediction ? obj->predictedPosition().w : obj->blob->w);

	for (int cnt_x = ( usePrediction ? obj->predictedPosition().x : obj->blob->x ) - (usePrediction ? obj->predictedPosition().w/2 : obj->blob->w/2) ; cnt_x < ( usePrediction ? obj->predictedPosition().x : obj->blob->x) + ( usePrediction ? obj->predictedPosition().w/2 : obj->blob->w/2) ;cnt_x++)
	{
		for (int cnt_y = ( usePrediction ? obj->predictedPosition().y : obj->blob->y ) - (usePrediction ? obj->predictedPosition().h/2 : obj->blob->h/2) ; cnt_y < ( usePrediction ? obj->predictedPosition().y : obj->blob->y) + ( usePrediction ? obj->predictedPosition().h/2 : obj->blob->h/2) ;cnt_y++)
		{	


			if (cnt_y > h-1)
				continue;

			if (cnt_y < 0)
				continue;


			if (cnt_x > w-1)
				continue;

			if (cnt_x < 0)
				continue;

			label = glbl_var.BGlabels[cnt_x + w*cnt_y];

			if (label== CC->blob->ID)
				label_area++;
		}
	}

	float result = (label_area/obj->area);

	return result;
}

//the first blob is an object (uses centroids), the second one is a blb (uses top left)
bool intersect(CvBlob* blob1,CvBlob* blob2)
{
	bool result = false;
	bool x_condition = false;
	bool y_condition = false;

	// 2 objects
	/*if ((blob1->x  - blob2->x < 1) && (blob1->x + blob1->w/2 - blob2->x + blob2->w/2> -1))
	x_condition= true;
	if ((blob2->x  - blob1->x < 1) && (blob2->x + blob2->w/2 - blob1->x + blob1->w/2> -1))
	x_condition= true;
	if ((blob1->y - blob2->y  < 1) && (blob1->y + blob1->h/2 - blob2->y + blob2->h/2> -1))
	y_condition= true;
	if ((blob2->y- blob1->y  < 1) && (blob2->y + blob2->h/2  - blob1->y + blob1->h/2> -1))
	y_condition= true;*/

	double x1  = (blob1->x - blob1->w/2) ;
	double x2  = (blob2->x);
	double y1  = (blob1->y - blob1->h/2);
	double y2  = (blob2->y);


	//object, CC
	if ( (x1 - x2 < 1) && ( (x1 + blob1->w) > (x2-1) ))
		x_condition= true;
	if ( (x2 - x1 < 1) && ( (x2 + blob2->w) > (x1-1) ))
		x_condition= true;
	if ( (y1 - y2 < 1) && ( (y1 + blob1->h) > (y2-1) ))
		y_condition= true;
	if ( (y2 - y1 < 1) && ( (y2 + blob2->h) > (y1-1) ))
		y_condition= true;

	////object, CC (only partial intersection)
	//if ( ((blob1->x + blob1->w/2)  - (blob2->x + blob2->w) < 1) && ((blob1->x + blob1->w/2) - blob2->x > -1))
	//	x_condition= true;
	//if ( (blob2->x - (blob1->x - blob1->w/2) < 1) && ((blob2->x + blob2->w) - (blob1->x - blob1->w/2)> -1) )
	//	x_condition= true;
	//if ( ((blob1->y + blob1->h/2)  - (blob2->y + blob2->h) < 1) && ((blob1->y + blob1->h/2) - blob2->y > -1))
	//	y_condition= true;
	//if ( (blob2->y - (blob1->y - blob1->h/2) < 1) && ((blob2->y + blob2->h) - (blob1->y - blob1->h/2)> -1) )
	//	y_condition= true;

	////object, CC
	//if ((blob1->x  - (blob2->x + blob2->w/2) < 1) && ((blob1->x + blob1->w/2) - blob2->x > -1))
	//	x_condition= true;
	//if (((blob2->x + blob2->w/2) - blob1->x < 1) && ((blob2->x + blob2->w) - (blob1->x - blob1->w/2)> -1))
	//	x_condition= true;
	//if ((blob1->y - (blob2->y + blob2->h/2) < 1) && ((blob1->y + blob1->h/2) - blob2->y > -1))
	//	y_condition= true;
	//if (((blob2->y + blob2->h/2) - blob1->y  < 1) && ((blob2->y + blob2->h) - (blob1->y - blob1->h/2)> -1))
	//	y_condition= true;



	/*if ((blob1.x + blob1.w/2 - blob2.x - blob2.w/2 < 1) && (blob1.x + blob1.w - blob2.x > -1))
	x_condition= true;
	if ((blob2.x + blob2.w/2 - blob1.x - blob1.w/2 < 1) && (blob2.x + blob2.w - blob1.x > -1))
	x_condition= true;
	if ((blob1.y + blob1.h/2 - blob2.y - blob2.h/2 < 1) && (blob1.y + blob1.h - blob2.y > -1))
	y_condition= true;
	if ((blob2.y + blob2.h/2 - blob1.y - blob1.h/2 < 1) && (blob2.y + blob2.h - blob1.y > -1))
	y_condition= true;*/

	if (x_condition && y_condition) 
		result=true;

	return result;
}

//the majority of the object is covered by this blob
bool BG_codebook_model::pixelToPixelIntersect(TrackerObject* obj,ConnectedComponent* CC, bool usePrediction)
{


	bool result;
	int label;
	int indx;
	int isObject;
	int left = ( usePrediction ? obj->predictedPosition().x - obj->predictedPosition().w/2 : obj->blob->x -obj->blob->w/2);
	int top = ( usePrediction ? obj->predictedPosition().y  - obj->predictedPosition().h/2: obj->blob->y-obj->blob->h/2 );
	int right = left + ( usePrediction ? obj->predictedPosition().w: obj->blob->w);
	int bottom = top + ( usePrediction ? obj->predictedPosition().h : obj->blob->h);
	for (int cnt_x = left  ; cnt_x < right ;cnt_x++)
	{
		for (int cnt_y = top  ; cnt_y < bottom ;cnt_y++)
		{	


			if (cnt_y > h-1)
				continue;

			if (cnt_y < 0)
				continue;


			if (cnt_x > w-1)
				continue;

			if (cnt_x < 0)
				continue;

			label = glbl_var.BGlabels[cnt_x + w*cnt_y];

			indx = cnt_x - left + (cnt_y - top)*obj->blobMask->widthStep;
			isObject = (int) (unsigned char) (obj->blobMask->imageData[indx]);


			if ( label == CC->blob->ID)
				if(isObject)
					result = true;
			//return true;
		}
	}




	if(result)
		return true;

	return false;
}


TrackerObject* createObject( ConnectedComponent* ptrCC , int &nextID)
{
	TrackerObject* tempObj = new TrackerObject();
	tempObj->time = glbl_var.BGmodel->get_t()-1;
	tempObj->persistence = glbl_var.BGmodel->get_t()-1;
	tempObj->absence = -1;
	tempObj->blob->ID = nextID++;
	if (tempObj->moment < glbl_var.BGmodel->TYPE_THRESHOLD)
		tempObj->type = 0; //bag
	else
		tempObj->type = 1; //person

	//at the beginning, the object is still of an unknown type
	tempObj->object_classification = UNKNOWN;

	tempObj->PersonGroup = 0;


	//tempObj->prepareHistoryCorrelationChildren();
	update_Obj_CC(tempObj,ptrCC,0,1,0);//distance is 0 since it's identical at initialization






	return tempObj;

}

//should only be called if it's an occlusion child
bool TrackerObject::all_children_matched()
{
	list<TrackerObject*>::iterator ptrObj;

	for (ptrObj= occlusion_list.begin(); ptrObj != occlusion_list.end(); ptrObj++)
		if (!(*ptrObj)->matched) return false;

	return true;
}


void update_Obj_CC(TrackerObject* obj ,ConnectedComponent* CC, double distance, bool total_update, bool resetKalman)
{
	if (total_update) //update spatially and appearance model (by means of the CC)
	{
		//if (obj->hist) cvReleaseHist(&(obj->hist));

		//find the sum and scale to a ratio of it (temporal update)
		if (obj->hist)
		{

			//time update of the histogram
			IplImage* Ch[3];
			IplImage* bg_mask;
			for (int i=0; i < 3; i++)
				Ch[i] = cvCreateImage(cvGetSize(CC->blob_img),glbl_var.currentFrame->depth,1);
			bg_mask = cvCreateImage(cvGetSize(CC->blob_img),glbl_var.currentFrame->depth,1);
			cvSplit(CC->blob_img,Ch[0],Ch[1],Ch[2],0);
			cvCopyImage(CC->blob_mask,bg_mask);
			//for (int i=0; i < 4; i++)
			//	cvSetImageROI(Ch[i],cvRect(CC->blob->x,CC->blob->y,CC->blob->w,CC->blob->h));
			//IplImage** planes = {Ch[0], Ch[1], Ch[2]};

			//1
			//float sum = 0;
			//for(int i=0; i < 16; i++)
			//	for(int j=0; j < 16; j++)
			//		for(int k=0; k < 16; k++)
			//			sum += cvQueryHistValue_3D( obj->hist, i, j , k);

			//cvNormalizeHist(obj->hist,0.66*sum);
			////cvCalcHist(Ch,CC->hist,1, Ch[3]);
			//cvCalcHist(Ch,obj->hist,1, bg_mask);
			////cvNormalizeHist((*ptrCC)->hist,1);

			//2
			float sum = 0;
			for(int i=0; i < 16; i++)
				for(int j=0; j < 16; j++)
					for(int k=0; k < 16; k++)
						sum += cvQueryHistValue_3D( CC->hist, i, j , k);
			//sum += cvQueryHistValue_3D( Obj->hist, i, j , k);

			cvNormalizeHist(obj->hist,1.5*sum);
			//cvCalcHist(Ch,CC->hist,1, Ch[3]);
			cvCalcHist(Ch,obj->hist,1, bg_mask);
			cvNormalizeHist(obj->hist,sum);


			for (int i=0; i < 3; i++)
				cvReleaseImage(&Ch[i]);
			cvReleaseImage(&bg_mask);
		}
		else
			cvCopyHist(CC->hist, &obj->hist);

		if(glbl_var.saveFramesToHard && glbl_var.saveObjectHistograms )
		{
			char* tempChar = new char[100];
			char buffer[10];
			strcpy(tempChar,glbl_var.saveFramesTo);
			strcat(tempChar,"histogram");
			strcat(tempChar,itoa(obj->blob->ID,buffer,10));
			strcat(tempChar,"frame");
			strcat(tempChar,itoa(glbl_var.BGmodel->get_t(),buffer,10));
			strcat(tempChar,".txt");
			cv::FileStorage fs(tempChar,1); 
			fs.writeObj("Histogram",obj->hist); 
			delete[] tempChar;
		}







		//cvCopyHist(CC->hist, &obj->hist);
		obj->moment = CC->moment;
	}



	//update previous location
	if (  obj->blob->x >=0 )
	{
		obj->prev_blob->x = obj->blob->x;
		obj->prev_blob->y = obj->blob->y;
		obj->prev_blob->w = obj->blob->w;
		obj->prev_blob->h = obj->blob->h;
	}
	else
	{
		/*obj->prev_blob->x = CC->blob->x;
		obj->prev_blob->y = CC->blob->y;*/
		obj->prev_blob->x = CC->centroid_x;
		obj->prev_blob->y = CC->centroid_y;
		obj->prev_blob->w = CC->blob->w;
		obj->prev_blob->h = CC->blob->h;
	}

	//update location
	//obj->blob->x = CC->blob->x;
	//obj->blob->y = CC->blob->y;
	obj->blob->x = CC->centroid_x;
	obj->blob->y = CC->centroid_y;
	obj->blob->w = CC->blob->w;
	obj->blob->h = CC->blob->h;

	obj->area = CC->area;



	obj->matched = true;
	obj->CC = CC;
	CC->objects.push_front(obj);
	obj->distanceFromCC = distance;
	CC->distanceFromCC = distance;
	CC->matched++; 

	//update "persistent"
	if(!obj->persistent)
	{
		if(glbl_var.BGmodel->get_t() - 1 -obj->persistence >= glbl_var.BGmodel->PERSISTENCE_THRESHOLD)
		{
			obj->persistent = true;
		}
	}


	////if object is an occlusion child and the parent has all its children matched, then stop checking that parent.
	//if (obj->occlusion_child)
	//{
	//	list<TrackerObject*>::iterator ptrObj;
	//	TrackerObject* ptrObj2;

	//	ptrObj= obj->parent.begin();
	//	while ( ptrObj != obj->parent.end())
	//	{
	//		if( all_children_matched(*ptrObj))
	//		{
	//			ptrObj2 = (*ptrObj);



	//		
	//			
	//			ptrObj++;
	//			//erase_object(*ptrObj2);
	//			
	//			//match_parent(*ptrObj2);
	//			ptrObj2->toBeDeleted = 1;

	//			
	//		}
	//		else
	//			ptrObj++;

	//	}
	//}

	//mask
	if(obj->blobMask) cvReleaseImage(&obj->blobMask);
	obj->blobMask = cvCloneImage(CC->blob_mask);

	//Kalman
	if (!obj->kalman  || resetKalman) //if first time or object was invisible, create it or reset it respectively || !(obj->isVisible)
	{

		float delta_t=1; // because we are dealing per frame
		float f;
		/*if (glbl_var.realtimeProcessing)
		f = (glbl_var.BGmodel->get_t()/glbl_var.AbsoluteTime);
		else
		f = glbl_var.OriginalVideoFrameRate; */

		f = glbl_var.sec_to_frame(1,true);

		obj->kalman = cvCreateKalman(4,4);

		CvMat* state = cvCreateMat(4,1,CV_32FC1);
		CvMat* measurment = cvCreateMat(4,1,CV_32FC1);
		CvMat* transition_matrix = cvCreateMat(4,4,CV_32FC1);
		CvMat* measurment_matrix = cvCreateMat(4,4,CV_32FC1);
		CvMat* initial_state = cvCreateMat(4,1,CV_32FC1);

		cvSetIdentity(transition_matrix);
		cvmSet(transition_matrix,0,2,delta_t);
		cvmSet(transition_matrix,1,3,delta_t);

		cvSetIdentity(measurment_matrix);


		cvZero(initial_state);
		cvmSet(initial_state,0,0,obj->blob->x);
		cvmSet(initial_state,1,0,obj->blob->y);
		cvmSet(initial_state,2,0,0);
		cvmSet(initial_state,3,0,0);

		float delta_t_4 = pow(delta_t,4);
		float delta_t_3 = pow(delta_t,3);
		float delta_t_2 = pow(delta_t,2);

		float sigma_2=2500/(f*f);
		//const float wk[] = {sigma_2*delta_t_4/4,0,sigma_2*delta_t_3/2,0, 0,sigma_2*delta_t_4/4,0,sigma_2*delta_t_3/2, sigma_2*delta_t_3/2,0,sigma_2*delta_t_2,0, 0,sigma_2*delta_t_3/2,0,sigma_2*delta_t_2 };
		const float wk[] = {0,0,0,0, 0,0,0,0, 0,0,sigma_2*delta_t_2,0, 0,0,0,sigma_2*delta_t_2 };
		memcpy( obj->kalman->process_noise_cov->data.fl, wk, sizeof(wk));


		obj->kalman->transition_matrix = transition_matrix;
		obj->kalman->measurement_matrix = measurment_matrix;
		obj->kalman->state_post = initial_state;

		obj->prev_blob->x = obj->blob->x;
		obj->prev_blob->y = obj->blob->y;
		cvKalmanPredict(obj->kalman);

	}
	//else //update
	//{
	//const CvMat* prediction =  cvKalmanPredict(obj->kalman);
	CvMat* measurment = cvCreateMat(4,1,CV_32FC1);

	if(glbl_var.BGmodel->get_t() == 71 && obj->blob->ID == 4)
	{
		int uu;
		uu = 0;
	}



	cvmSet(measurment,0,0,obj->blob->x);
	cvmSet(measurment,1,0,obj->blob->y);
	cvmSet(measurment,2,0,obj->blob->x - obj->prev_blob->x);
	cvmSet(measurment,3,0,obj->blob->y - obj->prev_blob->y);

	cvKalmanCorrect(obj->kalman,measurment);
	cvReleaseMat(&measurment);
	//cvReleaseMat(&prediction);
	//}

	obj->isVisible = true;
}

//method:
//0  =correlation, 1= intersection, 2 = difference
double colorDistance(TrackerObject* ptrObj, ConnectedComponent* ptrCC, int method)
{

	float sumObj = 0;
	float sumCC = 0;
	for(int i=0; i < 16; i++)
	{
		for(int j=0; j < 16; j++)
		{
			for(int k=0; k < 16; k++)
			{
				float a = cvQueryHistValue_3D( ptrObj->hist, i, j , k);
				float b = cvQueryHistValue_3D( ptrCC->hist, i, j , k);
				sumObj += a;
				sumCC += b;
			}
		}
	}

	if(method < 2)
	{
		double distance = cvCompareHist(ptrObj->hist,ptrCC->hist , ( method? CV_COMP_INTERSECT : CV_COMP_CORREL ));
		float minsum = 0;
		
		//distance /= ((sumObj < sumCC) ? sumObj: sumCC);
		distance /= ((sumObj > sumCC) ? sumObj: sumCC);
		//distance /= sumObj; //since we are comparing to the object
		if (method) distance = 1- distance;

		return distance;
	}
	else
	{
		double distance = 0;
		for(int i=0; i < 16; i++)
		{
			for(int j=0; j < 16; j++)
			{
				for(int k=0; k < 16; k++)
				{
					float a = cvQueryHistValue_3D(ptrObj->hist,i,j,k);
					float b = cvQueryHistValue_3D(ptrCC->hist,i,j,k);
					distance += abs(a/sumObj - b/sumCC);
				}
			}
		}

		return distance/2;
		
	}
}

//void insert_object_occlusionlist(TrackerObject* parent,TrackerObject* direct_parent, TrackerObject* obj, int occlusion_group_number)
void insert_object_occlusionlist(TrackerObject* parent,TrackerObject* direct_parent, TrackerObject* obj)
{	
	list<TrackerObject*>::iterator ptrObj;
	list<TrackerObject*>::iterator ptrObj2;
	//recursively insert children
	for (ptrObj= obj->occlusion_list.begin(); ptrObj != obj->occlusion_list.end(); ptrObj++)
		insert_object_occlusionlist(parent,obj,(*ptrObj)); //,occlusion_group_number
	//set to not visible
	obj->isVisible = false;
	//delete original parent 
	obj->parent.remove(direct_parent);
	/*for (ptrObj= obj->parent.begin(); ptrObj != obj->parent.end(); ptrObj++)
		if ((*ptrObj) == direct_parent)
		{
			obj->parent.erase(ptrObj);
			break;
		}*/

		//insert new parent
		obj->parent.push_front(parent);
		if (!obj->parent.empty()) 
		{
			obj->parent.sort();
			obj->parent.unique();
		}
		//set occlusion parameters
		//obj->occlusion = occlusion_group_number;
		obj->occlusion_child = true;
		parent->occlusion_list.push_front(obj);
		if (!parent->occlusion_list.empty())
		{
			parent->occlusion_list.sort();
			parent->occlusion_list.unique();
		}
		obj->occlusion_list.clear();




}

/// when objects are merged, they should obey the rules in Bird at al.
objClassification object_classification_merge(list<TrackerObject*> objList)
{
	objClassification highest_object = UNKNOWN;
	list<TrackerObject*>::iterator ptrObj;

	//find highest classification in the pool
	for (ptrObj= objList.begin(); ptrObj != objList.end(); ptrObj++)
	{	
		if ((*ptrObj)->object_classification == PERSON)
		{
			highest_object = PERSON;
			break;
		}
		else if ((*ptrObj)->object_classification == STILL_PERSON)
		{
			highest_object = STILL_PERSON;
		}
		else if (highest_object != STILL_PERSON && (*ptrObj)->object_classification == OBJECT)
		{
			highest_object = OBJECT;
		}
	}

	return highest_object;

	//for (ptrObj= objList.begin(); ptrObj != objList.end(); ptrObj++)
	//{
	//	(*ptrObj)->object_classification = highest_object;
	//}

}

void object_classification_split(TrackerObject* originalObj,TrackerObject* splitObject,int grp)
{
	//object
	if (originalObj->object_classification == OBJECT)
	{
		splitObject->updateCandidate_object_classification = OBJECT;
		//splitObject->belongs_to = findOwner(splitObject);
		findOwner(splitObject);
		return;
	}

	//unknown
	if (originalObj->object_classification == UNKNOWN)
	{
		splitObject->updateCandidate_object_classification = UNKNOWN;
		splitObject->updateCandidate_PersonGroup = originalObj->PersonGroup;
		return;
	}

	//person or still person
	splitObject->updateCandidate_object_classification = UNKNOWN;
	splitObject->updateCandidate_PersonGroup = grp;
}

void object_classification_update(TrackerObject* obj)
{

	//since it is only called for visible objects, it has one speed.
	list<historyPoint*>::iterator historyIterator;
	list<motionPossibility*>::iterator possibilityIterator;
	//double speed= (*(*(*obj->history.begin())->begin())->motionPossibilities.begin())->speed;
	double speed= (*(*obj->currentHistoryMeasurement->begin())->motionPossibilities.begin())->speed;
	double speedThreshold = glbl_var.BGmodel->MOTION_TOLERANCE/TIME_MEASUREMENT_STEP;


	//if(obj->splitIndex == 0) //in case it is not coming from a split
	//{
	if (obj->object_classification == UNKNOWN) //U -> ?
	{
		if(speed < speedThreshold) 	//U -> A
		{
			obj->object_classification = OBJECT;
			//obj->belongs_to = findOwner(obj);
		}
		else 	//U -> P
			obj->object_classification = PERSON;
	}
	else if (obj->object_classification == PERSON && speed < speedThreshold) //P -> SP
		obj->object_classification = STILL_PERSON;
	else if ((obj->object_classification == OBJECT || obj->object_classification == STILL_PERSON )&& speed >= speedThreshold) //(SP,A) -> P
		obj->object_classification = PERSON;
	//}
	//else //coming from a split
	//	{

	//	}

}

//TrackerObject* findOwner(TrackerObject* object)
void findOwner(TrackerObject* object)
{
	if(glbl_var.saveFramesToHard && glbl_var.saveObjectFile )
		glbl_var.BGmodel->objectFile  << "--Finding owner of object" << object->blob->ID <<endl ;


	//if (object->object_classification != OBJECT) 
	//{
	//	if(glbl_var.saveFramesToHard)
	//		glbl_var.BGmodel->objectFile  << "   it is not an object. IT can't have an owner !" <<endl ;


	//	return NULL; //no ownership for persons
	//}

	historyPoint* objectHp = *(object->currentHistoryMeasurement->begin()); //assuming there is only one history point for a visible object

	list<TrackerObject*>::iterator ptrObj;
	list<TrackerObject*>::iterator ptrObj_temp;
	list<historyPoint*>::iterator ptrHp;

	TrackerObject* owner = NULL;
	double bestDistance = -1;
	for (ptrObj= glbl_var.BGmodel->trackerBlobs.begin(); ptrObj != glbl_var.BGmodel->trackerBlobs.end(); ptrObj++)
	{
		if(glbl_var.saveFramesToHard && glbl_var.saveObjectFile )
			glbl_var.BGmodel->objectFile  << "   checking object " << (*ptrObj)->blob->ID <<endl ;

		//if (!(*ptrObj)->reliable() ) continue; //only visible parent objects
		bool notVisibleStandalone = !((*ptrObj)->isVisible && (*ptrObj)->occlusion_list.empty());
		bool notChildOfVisibleParent = true;
		for(ptrObj_temp = (*ptrObj)->parent.begin() ; ptrObj_temp != (*ptrObj)->parent.end() ; ptrObj_temp++)
		{
			if((*ptrObj_temp)->isVisible) 
			{
				notChildOfVisibleParent= false;
				break;
			}
		}
		if ( notVisibleStandalone && notChildOfVisibleParent ) 
		{
			if(glbl_var.saveFramesToHard && glbl_var.saveObjectFile )
				glbl_var.BGmodel->objectFile  << "    it is not considered because it is invisible or a parent "  <<endl ;

			continue; //only visible parent objects
		}
		if ((*ptrObj)->object_classification == OBJECT || (*ptrObj)->object_classification == UNKNOWN) 
		{
			if(glbl_var.saveFramesToHard &&glbl_var.saveObjectFile )
				glbl_var.BGmodel->objectFile  << "    it is not considered because it is not a person "  <<endl ;

			continue; //skip other object (just interested in persons)
		}
		if(object == *ptrObj) continue; //skip the object itself

		for (ptrHp =  (*ptrObj)->currentHistoryMeasurement->begin() ; ptrHp != (*ptrObj)->currentHistoryMeasurement->end(); ptrHp++)
		{

			double newDistance  =calcDistance(objectHp, (*ptrHp));

			if (bestDistance < 0) //first person
			{
				owner = (*ptrObj);
				bestDistance = newDistance;
			}
			else
			{
				if(newDistance < bestDistance)
				{
					bestDistance = newDistance;
					owner = (*ptrObj);
				}
			}
		}

	}

	if(glbl_var.saveFramesToHard && glbl_var.saveObjectFile )
	{
		glbl_var.BGmodel->objectFile  << "   best owner is ";
		if (owner)
			glbl_var.BGmodel->objectFile  << (owner->blob->ID) <<endl;
		else
			glbl_var.BGmodel->objectFile  <<"not found";


		glbl_var.BGmodel->objectFile  << endl;
	}


	//return owner;
	object->belongs_to = owner;
	if (object->belongs_to) cvCopyHist(object->belongs_to->hist,&object->ownerHist);
}

bool areMerged(historyPoint* hp1,historyPoint* hp2)
{
	if(glbl_var.saveFramesToHard )
		glbl_var.BehaviourDescriptor.behaviourFile << "      related parents are " << hp1->relatedParent->blob->ID << " and " <<  hp2->relatedParent->blob->ID <<endl ;

	if (hp1->relatedParent == hp2->relatedParent)
		return true;

	return false;
}


double calcOffsetFromStratingPoint(historyPoint* hp, TrackerObject* obj)
{
	//TrackerObject* dummy = new TrackerObject();
	//dummy->currentHistoryMeasurement = new list<historyPoint*>;
	//historyPoint* newPoint = new historyPoint();
	//dummy->currentHistoryMeasurement->push_front(newPoint);

	//assuming it is just calculated for visible objects
	list<historyPoint*>* startingHistoryLine = (*obj->history.rbegin());
	historyPoint* startingPoint = (*startingHistoryLine->begin());


	/*if(obj->history.empty())
	{*/
	///*list<historyPoint*>::reverse_iterator ptr = obj->history.rbegin();*/
	//newPoint->xBase = startingPoint->xBase; //because starting piint can only have one single possibility
	//newPoint->yBase = startingPoint->yBase;
	//}
	/*else
	{
	dummy->currentHistoryMeasurement->xBase = obj->currentHistoryMeasurement->xBase;
	dummy->currentHistoryMeasurement->yBase = obj->currentHistoryMeasurement->yBase;
	}*/

	double result = calcDistance(hp,startingPoint);

	return result;

}

bool TrackerObject::simpleIsOrParentVisible()
{
	//visible single
	if(occlusion_list.empty())
	{
		if( isVisible)
			return true;

		//child of a visible parent
		else
		{
			list<TrackerObject*>::iterator itr;
			for(itr = parent.begin() ; itr != parent.end() ; itr++)
				if((*itr)->isVisible)
					return true;
		}
	}

	return false;
}

double TrackerObject::hasFallen()
{
	//it has to be a person
	if (object_classification == OBJECT || object_classification == UNKNOWN) return false;

	//check the ratio
	//(i)
	//float ratio = ((float)blob->w)/blob->h;
	//if(ratio < glbl_var.BehaviourDescriptor.W_to_H_FAINTING_RATIO ) return false;
	// or (ii)
	if(!occlusion_list.empty() || !parent.empty()) return false; //no one should be around
	//now the object is guaranteed to be standing alone
	historyPoint* hp = (*currentHistoryMeasurement->begin());
	float head_x = hp->xTop;
	float head_y = hp->yTop;
	float foot_x = hp->xBase;
	float foot_y = hp->yBase;

	float head_feet_distance_2 =  (head_x - foot_x)*(head_x - foot_x) + (head_y - foot_y)*(head_y - foot_y);

	if(glbl_var.saveFramesToHard && glbl_var.saveObjectFile)
		glbl_var.BGmodel->objectFile << "distance between head and feet = " << cvSqrt( head_feet_distance_2) << endl;

	if(head_feet_distance_2 < (glbl_var.BGmodel->FALLING_TOLERANCE*glbl_var.BGmodel->FALLING_TOLERANCE)) return false; //head is close enough to feet in the x-y plane


	return true;
}

bool TrackerObject::reliable()
{
	if (!parent.empty()) return true;
	if (!isVisible) return true;

	return false;
}


historyPoint::historyPoint()
{
	xBase = 0;
	yBase = 0;
	xTop = 0;
	yTop = 0;
	/* vxBase = 0; 
	vyBase = 0; 
	speed = 0;
	xDirection = 0;
	yDirection = 0;*/
	time = glbl_var.sampledAbsoluteTime;
	visibility = false;
	falls = false; 
	relatedParent = NULL;
	relatedCC = NULL;


	ID = -1;//default invalid
}




void historyPoint::updateFromPointUsingObject( historyPoint* p, TrackerObject* obj,TrackerObject* child, bool forMerge)
{

	list<TrackerObject*>::iterator ptrObj;

	//xBase = p->xBase;
	//yBase = p->yBase;

	if(forMerge)
		visibility = false;
	else
	{
		if (obj == child)
			visibility = true;
		else
			visibility = false;
	}


	falls = obj->hasFallen();
	/*occlusion = p->occlusion;*/
	//frameNumber = glbl_var.BGmodel->get_t();
	//time = glbl_var.sampledAbsoluteTime;
	//relatedParent = p->relatedParent;
	relatedParent = obj;

	occlusion = obj->occlusion_list;
	occlusion.push_front(obj);

	if(glbl_var.saveFramesToHard)
	{
		glbl_var.BGmodel->objectFile << "    " << (visibility ? "VISIBLE": "NOT visible" ) <<endl;
		glbl_var.BGmodel->objectFile << "    " << (falls ? "FALLEN": "NOT fallen" ) <<endl;
		glbl_var.BGmodel->objectFile << "    related parent " << obj->blob->ID <<endl;

		//occlusion
		glbl_var.BGmodel->objectFile << "    occlusion group members: ";
		for(ptrObj= occlusion.begin(); ptrObj != occlusion.end(); ptrObj++) 
		{
			glbl_var.BGmodel->objectFile << (*ptrObj)->blob->ID << ",";
		}
		glbl_var.BGmodel->objectFile << endl;


	}

	//duplicate possibilities
	/*list<motionPossibility*>::iterator itr;*/

	/*for(itr = p->motionPossibilities.begin() ; itr != p->motionPossibilities.end() ; itr++)
	{
	motionPossibility* newPossibility = new motionPossibility();
	motionPossibilities.push_front(newPossibility);
	newPossibility->vxBase = (*itr)->vxBase; 
	newPossibility->vyBase = (*itr)->vyBase; 
	newPossibility->speed = (*itr)->speed;
	newPossibility->xDirection = (*itr)->xDirection;
	newPossibility->yDirection = (*itr)->yDirection;
	newPossibility->precedingPoint = p;
	}*/

	motionPossibility* newPossibility = new motionPossibility();
	newPossibility->precedingPoint = p;
	motionPossibilities.push_front(newPossibility);




}


void historyPoint::updateFromPointUsingObject_split( TrackerObject* obj)
{

	list<TrackerObject*>::iterator ptrObj;
	//xBase = p->xBase;
	//yBase = p->yBase;

	//if(forMerge)
	//	visibility = false;
	//else
	visibility = obj->isVisible;

	falls = obj->hasFallen();
	/*occlusion = p->occlusion;*/
	//frameNumber = glbl_var.BGmodel->get_t();
	//time = glbl_var.sampledAbsoluteTime;
	//relatedParent = p->relatedParent;
	relatedParent = obj;

	occlusion = obj->occlusion_list;
	occlusion.push_front(obj);

	if(glbl_var.saveFramesToHard)
	{
		glbl_var.BGmodel->objectFile << "    " << (visibility ? "VISIBLE": "NOT visible" ) <<endl;
		glbl_var.BGmodel->objectFile << "    " << (falls ? "FALLEN": "NOT fallen" ) <<endl;
		glbl_var.BGmodel->objectFile << "    related parent " << obj->blob->ID <<endl;

		//occlusion
		glbl_var.BGmodel->objectFile << "    occlusion group members: ";
		for(ptrObj= occlusion.begin(); ptrObj != occlusion.end(); ptrObj++) 
		{
			glbl_var.BGmodel->objectFile << (*ptrObj)->blob->ID << ",";
		}
		glbl_var.BGmodel->objectFile << endl;



	}

	//duplicate possibilities
	/*list<motionPossibility*>::iterator itr;*/

	/*for(itr = p->motionPossibilities.begin() ; itr != p->motionPossibilities.end() ; itr++)
	{
	motionPossibility* newPossibility = new motionPossibility();
	motionPossibilities.push_front(newPossibility);
	newPossibility->vxBase = (*itr)->vxBase; 
	newPossibility->vyBase = (*itr)->vyBase; 
	newPossibility->speed = (*itr)->speed;
	newPossibility->xDirection = (*itr)->xDirection;
	newPossibility->yDirection = (*itr)->yDirection;
	newPossibility->precedingPoint = p;
	}*/






}

//void motionPossibility::updateByObject(TrackerObject *ptr, historyPoint* historyPtr) //assumes xbase and ybase are already set
//{
//
//
//	list<TrackerObject*>::iterator ptrObj;
//	list<TrackerObject*>::iterator ptrObj2;
//	list<motionPossibility*>::iterator ptrPossibility;
//
//
//
//		double t1 = glbl_var.sampledAbsoluteTime;
//		double t2 = (*ptr->lastHistoryMeasurement->begin())->time;
//		double time_difference=  t1 - t2;
//
//		//find the corresponding last history point
//		list<historyPoint*> correspondingLastHistoryPointList;
//		list<historyPoint*> tempList;
//		list<historyPoint*>::iterator tempListIterator;
//		correspondingLastHistoryPoint.push_front(precedingPoint);
//		while ((*correspondingLastHistoryPointList.begin())->time != t2)
//		{
//			for(tempListIterator = correspondingLastHistoryPointList.begin()  ;tempListIterator != correspondingLastHistoryPointList.end() ; tempListIterator++)
//				tempList.push_front(*tempListIterator);
//
//			correspondingLastHistoryPoint = tempList;
//			tempList.clear();
//
//		}
//
//		////create a possibility for each of the contingencies
//		for(tempListIterator = correspondingLastHistoryPointList.begin()  ;tempListIterator != correspondingLastHistoryPointList.end() ; tempListIterator++)
//		{
//			double x1 = historyPtr->xBase;
//			double x2 = (*tempListIterator)->xBase;
//			double y1 = historyPtr->yBase;
//			double y2 = (*tempListIterator)->Base;
//
//			vxBase = (x1 - x2)/time_difference;
//			vyBase = (y1 - y2)/time_difference;
//			speed = cvSqrt(vxBase*vxBase + yBase*vyBase);
//			xDirection = vxBase/speed;
//			yDirection = vyBase/speed;
//			/*time = glbl_var.sampledAbsoluteTime;
//			frameNumber =	glbl_var.BGmodel->get_t();*/
//
//		/*	if(isParent)
//				visibility = false;
//			else
//				visibility = true;*/
//
//			/*onlyPossibility->falls = hasFallen(ptr);*/
//
//			//if(isParent)
//			//relatedParent = ptr;
//			//else
//			//	relatedParent= NULL; 
//
//
//			////insert parents and their children as occlusion IDs
//			//for(ptrObj= ptr->parent.begin(); ptrObj != ptr->parent.end(); ptrObj++)
//			//{
//			//	occlusion.insert(occlusion.begin(),(*ptrObj));
//			//	for(ptrObj2= (*ptrObj)->occlusion_list.begin(); ptrObj2 != (*ptrObj)->occlusion_list.end(); ptrObj2++)
//			//		occlusion.insert(occlusion.begin(),(*ptrObj2));
//			//}
//
//		}
//
//
//
//
//		///*if(motionPossibilities.empty())
//		//{*/
//		//motionPossibility* newPossibility = new motionPossibility();
//		//motionPossibilities.push_front(newPossibility)
//		//	//}
//
//		//motionPossibility* onlyPossibility = (*motionPossibilities.begin())
//		/*motionPossibility* onlyPossibility = newPossibility;*/
//
//
//
//
//
//}



void historyPoint::updateByObject(TrackerObject *ptrObject) //assumes xbase and ybase are already set , bool isParent ,TrackerObject *ptrParent, 
{
	list<TrackerObject*>::iterator ptrObj;
	list<TrackerObject*>::iterator ptrObj2;
	list<motionPossibility*>::iterator ptrPossibility;
	list<motionPossibility*>::iterator ptrPossibility2;


	double t1 = glbl_var.sampledAbsoluteTime;
	double t2 = (*ptrObject->lastHistoryMeasurement->begin())->time;
	//double time_difference=  t1 - t2;



	//for all the possibilities
	for(ptrPossibility = motionPossibilities.begin() ;ptrPossibility != motionPossibilities.end()  ;ptrPossibility++)
	{

		if(glbl_var.saveFramesToHard)
			glbl_var.BGmodel->objectFile << " get possibility "  << endl;


		//find the corresponding last history points
		list<historyPoint*> correspondingLastHistoryPointList;
		list<historyPoint*> tempList;
		list<historyPoint*>::iterator tempListIterator;
		correspondingLastHistoryPointList.push_front((*ptrPossibility)->precedingPoint);
		//while ((*correspondingLastHistoryPointList.begin())->time != t2)
		//while ( (*correspondingLastHistoryPointList.begin()) && !(*(*correspondingLastHistoryPointList.begin())->motionPossibilities.begin())->valid)
		while ((*correspondingLastHistoryPointList.begin())->time > t2 || !(*(*correspondingLastHistoryPointList.begin())->motionPossibilities.begin())->valid )
		{
			for(tempListIterator = correspondingLastHistoryPointList.begin()  ;tempListIterator != correspondingLastHistoryPointList.end() ; tempListIterator++)
				for(ptrPossibility2 = (*tempListIterator)->motionPossibilities.begin()  ;ptrPossibility2 != (*tempListIterator)->motionPossibilities.end() ; ptrPossibility2++)
					tempList.push_front((*ptrPossibility2)->precedingPoint);

			correspondingLastHistoryPointList = tempList;
			tempList.clear();

		}

		double time_difference=  t1 - t2;

		if(glbl_var.saveFramesToHard)
			glbl_var.BGmodel->objectFile << "  executing time measurement update with: tnew = " << t1 << " and, told = " << t2 <<endl;

		if(glbl_var.saveFramesToHard)
		{
			glbl_var.BGmodel->objectFile << "   corresponding last histories parents: ";
			for(tempListIterator = correspondingLastHistoryPointList.begin() ;tempListIterator !=correspondingLastHistoryPointList.end()  ; tempListIterator++)
			{
				glbl_var.BGmodel->objectFile << (*tempListIterator)->relatedParent->blob->ID << " , ";
			}
			glbl_var.BGmodel->objectFile << endl;
		}

		//create a possibility for each of the contingencies
		if(glbl_var.saveFramesToHard)
			glbl_var.BGmodel->objectFile << "   creating new possibilities: ";

		int i = 0;
		for(tempListIterator = correspondingLastHistoryPointList.begin()  ;tempListIterator != correspondingLastHistoryPointList.end() ; tempListIterator++)
		{
			motionPossibility* usedPossibility;
			if (i == 0) //use the alread found possibility
			{
				usedPossibility = (*ptrPossibility);
			}
			else //add a new one
			{
				usedPossibility = new motionPossibility();
				usedPossibility->precedingPoint = (*ptrPossibility)->precedingPoint;
				this->motionPossibilities.push_front(usedPossibility);
			}
			double x1 = xBase;
			double x2 = (*tempListIterator)->xBase;
			double y1 = yBase;
			double y2 = (*tempListIterator)->yBase;

			if(glbl_var.saveFramesToHard)
				glbl_var.BGmodel->objectFile << "    new possibility between current: " << x1<< "," << y1<< " and old: " << x2 << "," << y2<< endl;

			usedPossibility->vxBase = (x1 - x2)/time_difference;
			usedPossibility->vyBase = (y1 - y2)/time_difference;
			usedPossibility->speed = cvSqrt(usedPossibility->vxBase*usedPossibility->vxBase + usedPossibility->vyBase*usedPossibility->vyBase);
			usedPossibility->xDirection = ((usedPossibility->speed == 0)? 0:usedPossibility->vxBase/usedPossibility->speed);
			usedPossibility->yDirection = ( (usedPossibility->speed == 0)? 0: usedPossibility->vyBase/usedPossibility->speed);
			usedPossibility->valid = true;

			if(glbl_var.saveFramesToHard)
			{
				glbl_var.BGmodel->objectFile << "     velocity= " << usedPossibility->vxBase << " , " << usedPossibility->vyBase <<endl;
				glbl_var.BGmodel->objectFile << "     speed= " << usedPossibility->speed  <<endl;
				glbl_var.BGmodel->objectFile << "     direction= " << usedPossibility->xDirection << " , " << usedPossibility->yDirection <<endl;

			}




			////insert parents and their children as occlusion IDs
			//for(ptrObj= ptr->parent.begin(); ptrObj != ptr->parent.end(); ptrObj++)
			//{
			//	occlusion.insert(occlusion.begin(),(*ptrObj));
			//	for(ptrObj2= (*ptrObj)->occlusion_list.begin(); ptrObj2 != (*ptrObj)->occlusion_list.end(); ptrObj2++)
			//		occlusion.insert(occlusion.begin(),(*ptrObj2));
			//}

			i++;
		}


		//time = glbl_var.sampledAbsoluteTime;
		//frameNumber =	glbl_var.BGmodel->get_t();
		//if(isParent)
		//	visibility = false;
		//else
		//	visibility = true;
		//falls = ptr->hasFallen();
		////if(isParent)
		//relatedParent = ptr;
		////else
		////	relatedParent= NULL; 





		///*if(motionPossibilities.empty())
		//{*/
		//motionPossibility* newPossibility = new motionPossibility();
		//motionPossibilities.push_front(newPossibility)
		//	//}

		//motionPossibility* onlyPossibility = (*motionPossibilities.begin())
		/*motionPossibility* onlyPossibility = newPossibility;*/



	}

}

//
//historyPoint* historyPointOfDifference(historyPoint* current, historyPoint* old) //assumes xbase and ybase are already set
//{
//	historyPoint* newPoint = new historyPoint();
//	list<TrackerObject*>::iterator ptrObj;
//	list<TrackerObject*>::iterator ptrObj2;
//
//	double t1 = glbl_var.sampledAbsoluteTime;
//	double t2 = old->time;
//	double x1 = current->xBase;
//	double x2 = old->xBase;
//	double y1 = current->yBase;
//	double y2 = old->yBase;
//
//	//calculate motion characteristics every TIME_MEASUREMENT_STEP
//	double time_difference = t1 - t2;
//
//
//	newPoint->xBase = x1;
//	newPoint->yBase = y1;
//	newPoint->vxBase = (x1 - x2)/time_difference;
//	newPoint->vyBase = (y1 - y2)/time_difference;
//	newPoint->speed = cvSqrt(newPoint->vxBase*newPoint->vxBase + newPoint->vyBase*newPoint->vyBase);
//	newPoint->xDirection = newPoint->vxBase/newPoint->speed;
//	newPoint->yDirection = newPoint->vyBase/newPoint->speed;
//	newPoint->time = glbl_var.sampledAbsoluteTime;
//	newPoint->frameNumber =	glbl_var.BGmodel->get_t();
//	newPoint->visibility = current->visibility;
//	newPoint->falls = current->falls;
//
//	////insert parents and their children as occlusion IDs
//	//for(ptrObj= ptr->parent.begin(); ptrObj != ptr->parent.end(); ptrObj++)
//	//{
//	//	newPoint->occlusion.insert(occlusion.begin(),(*ptrObj));
//	//	for(ptrObj2= (*ptrObj)->occlusion_list.begin(); ptrObj2 != (*ptrObj)->occlusion_list.end(); ptrObj2++)
//	//		newPoint->occlusion.insert(occlusion.begin(),(*ptrObj2));
//	//}
//
//	return newPoint;
//
//}

void TrackerObject::resolveHistoryAmbiguities()
{

	if(glbl_var.saveFramesToHard)
		glbl_var.BGmodel->objectFile << "-Resolving ambiguity for object " << blob->ID  << endl;

	//currentHistoryPoint and lastHistoryPoint will be automatically corrected by this

	list<list<historyPoint*>*>::iterator historyIterator;
	list<historyPoint*>::iterator historyPointIterator;
	list<historyPoint*>::iterator dummy_historyPointIterator;
	list<motionPossibility*>::iterator pastMPIterator;
	list<TrackerObject*>::iterator ptrObj;
	list<motionPossibility*>::iterator ptrPossibility;
	historyPoint* onlyHistoryPoint = (*(*history.begin())->begin()); //there is only one history point since the object has become visible

	//since there could be many possibilities per level due to merging. We need a list
	list<TrackerObject*>* trueParent = new list<TrackerObject*>;
	list<motionPossibility*>* precedingMP = new list<motionPossibility*>;
	//trueParent->insert(trueParent->begin(),candidateResolveHistoryParentList);
	if(glbl_var.saveFramesToHard)
		glbl_var.BGmodel->objectFile << " possibilities: " << endl;
	for(ptrPossibility =onlyHistoryPoint->motionPossibilities.begin() ; ptrPossibility != onlyHistoryPoint->motionPossibilities.end() ; ptrPossibility++)
	{
		trueParent->push_front((*ptrPossibility)->precedingPoint->relatedParent);
		precedingMP->push_front(*ptrPossibility);

		if(glbl_var.saveFramesToHard)
			glbl_var.BGmodel->objectFile << "  possibility related to " << (*ptrPossibility)->precedingPoint->relatedParent->blob->ID << endl;

	}


	//the current history list should have been entered by now; we need to go back one record
	for(historyIterator = ++(history.begin()); (*historyIterator)->size() > 1; historyIterator++) //iterate until the history becomes a single point again
	{
		if(glbl_var.saveFramesToHard)
			glbl_var.BGmodel->objectFile << "  next level"  << endl;


		historyPointIterator = (*historyIterator)->begin();
		list<TrackerObject*>* temp = new list<TrackerObject*>;
		list<motionPossibility*>* temp2 = new list<motionPossibility*>;
		while( historyPointIterator != (*historyIterator)->end()) //for all contingencies at that point
		{
			if(glbl_var.saveFramesToHard)
				glbl_var.BGmodel->objectFile << "   trying to find history point related to " << (*historyPointIterator)->relatedParent->blob->ID << endl;


			bool found = false;
			for(ptrObj =trueParent->begin() ; ptrObj !=trueParent->end() ; ptrObj++)
			{
				if((*ptrObj) == (*historyPointIterator)->relatedParent)
				{
					found = true;
					break;
				}
			}

			if(!found)
			{
				if(glbl_var.saveFramesToHard)
					glbl_var.BGmodel->objectFile << "    NOT found. delete history point and NULL all possibilities pointing to it"<< endl;

				dummy_historyPointIterator = historyPointIterator;
				historyPointIterator++;

				//delete all possibilities that were pointing at it
				for(pastMPIterator = precedingMP->begin();pastMPIterator != precedingMP->end();pastMPIterator++)
				{
					if((*pastMPIterator)->precedingPoint == (*dummy_historyPointIterator))
						(*pastMPIterator)->precedingPoint = NULL;
				}

				(*historyIterator)->erase(dummy_historyPointIterator); //delete the history point


			}
			else
			{
				if(glbl_var.saveFramesToHard)
					glbl_var.BGmodel->objectFile << "    FOUND. will navigate through its possibilities to related parents ";


				for(ptrPossibility =(*historyPointIterator)->motionPossibilities.begin() ; ptrPossibility != (*historyPointIterator)->motionPossibilities.end() ; ptrPossibility++)
				{
					temp->push_front((*ptrPossibility)->precedingPoint->relatedParent);
					temp2->push_front(*ptrPossibility);

					if(glbl_var.saveFramesToHard)
						glbl_var.BGmodel->objectFile << (*ptrPossibility)->precedingPoint->relatedParent->blob->ID << " , " ;
				}

				if(glbl_var.saveFramesToHard)
					glbl_var.BGmodel->objectFile << endl;

				historyPointIterator++;
			}
		}

		temp->sort();
		temp->unique();
		temp2->sort();
		temp2->unique();

		trueParent->clear();
		delete trueParent;
		trueParent=temp;
		precedingMP->clear();
		delete precedingMP;
		precedingMP=temp2;



	}
	delete trueParent;
	delete precedingMP;

	//reset candidate
	//candidateResolveHistoryParentList = NULL;
}

void TrackerObject::updateAllSiblingscandidateResolveHistoryParentList(TrackerObject* parentObj)
{
	if(glbl_var.saveFramesToHard)
		glbl_var.BGmodel->objectFile << "-Object " << blob->ID <<  "  :Split resolve history candidate creation for parent " << parentObj->blob->ID <<endl;


	list<TrackerObject*>::iterator iterator;
	for(iterator = parentObj->occlusion_list.begin(); iterator != parentObj->occlusion_list.end() ; iterator++)
	{
		//(*iterator)->candidateResolveHistoryParent = parentObj;
		(*iterator)->candidateResolveHistoryParentList.push_front(parentObj);

		if(glbl_var.saveFramesToHard)
			glbl_var.BGmodel->objectFile << "child " << (*iterator)->blob->ID << " has candidate for history resolution " << parentObj->blob->ID <<endl;

	}
}


void TrackerObject::prepareHistoryCorrelationChildren() //called by the parent
{

	//create histories for all split children that have been matched
	list<TrackerObject*>::iterator matchedChildPtr;
	list<TrackerObject*>::iterator itrObjects;
	list<historyPoint*>::iterator previousHistoryPoint;
	list<historyPoint*>* HistoryList;
	list<historyPoint*>* previousHistoryMeasurement;
	list<historyPoint*>::iterator itrHistory; 

	list<TrackerObject*> objectAndChildren;
	objectAndChildren  = this->occlusion_list;
	objectAndChildren.push_front(this);


	for(itrObjects = objectAndChildren.begin() ; itrObjects != objectAndChildren.end() ; itrObjects++)
	{
		//there might be several passes for the same object due to multiple splits. we have to make sure we only add one single history list per frame
		if((*itrObjects)->history.empty())
		{
			previousHistoryMeasurement = NULL;
			(*itrObjects)->currentHistoryMeasurement = new list<historyPoint*>;


			(*itrObjects)->history.push_front((*itrObjects)->currentHistoryMeasurement);

		}
		else if(glbl_var.BGmodel->get_t() != (*(*history.begin())->begin())->frameNumber ) //(*(*(*history.begin())).begin())->frameNumber
		{
			previousHistoryMeasurement = (*(*itrObjects)->history.begin());
			(*itrObjects)->currentHistoryMeasurement = new list<historyPoint*>;

			(*itrObjects)->history.push_front((*itrObjects)->currentHistoryMeasurement);
		}
		else
		{
			previousHistoryMeasurement = (*(history.begin()++));
		}


		historyPoint* newPoint;
		if( !(*itrObjects)->currentHistoryMeasurement || glbl_var.BGmodel->get_t() != (*(*itrObjects)->currentHistoryMeasurement->begin())->frameNumber )
		{
			newPoint = new historyPoint();
			newPoint->frameNumber = glbl_var.BGmodel->get_t();
			newPoint->time = glbl_var.sampledAbsoluteTime;

			(*itrObjects)->currentHistoryMeasurement->push_front(newPoint);
		}
		else
		{
			list<historyPoint*>* currentHistoryPtr = (*itrObjects)->currentHistoryMeasurement;

			bool  found = false;
			for (itrHistory = currentHistoryPtr->begin() ; itrHistory != currentHistoryPtr->end() ; itrHistory++)
			{
				if((*itrHistory)->relatedParent == this)
				{
					found = true;
					break;
				}
			}

			if(found)
			{
				newPoint = (*itrHistory);
			}
			else
			{
				newPoint = new historyPoint();
				newPoint->frameNumber = glbl_var.BGmodel->get_t();
				newPoint->time = glbl_var.sampledAbsoluteTime;

				(*itrObjects)->currentHistoryMeasurement->push_front(newPoint);
			}



		}

		//find a previous history point related to the parent
		if(previousHistoryMeasurement)
		{
			for(previousHistoryPoint = previousHistoryMeasurement->begin() ; previousHistoryPoint != previousHistoryMeasurement->end() ; previousHistoryPoint++ )
			{
				if((*previousHistoryPoint)->relatedParent == this)
				{
					motionPossibility* newPossibility = new motionPossibility();
					newPoint->motionPossibilities.push_front(newPossibility);
					newPossibility->precedingPoint = (*previousHistoryPoint);	
				}
			}
		}


	}


}


void TrackerObject::prepareHistorySplitChildren(list<ConnectedComponent*>* matchedCCs) //called by the object before split
{
	if(glbl_var.saveFramesToHard)
		glbl_var.BGmodel->objectFile << "-Creating history points for unmatched children of " << blob->ID <<endl;


	//create histories for all split children that have been matched
	list<TrackerObject*>::iterator matchedChildPtr;
	list<TrackerObject*>::iterator matchedParentPtr;
	list<ConnectedComponent*>::iterator CCiterator;
	list<historyPoint*>::iterator previousHistoryPoint;
	list<historyPoint*>::iterator currentHistoryPoint;
	list<historyPoint*>* HistoryList;
	list<motionPossibility*>::iterator mp;
	list<historyPoint*>* previousHistoryMeasurement;



	//for(matchedChildPtr = candidateResolveHistoryParentList->occlusion_list.begin() ; matchedChildPtr != candidateResolveHistoryParentList->occlusion_list.end() ; matchedChildPtr++)
	for(matchedChildPtr = occlusion_list.begin() ; matchedChildPtr != occlusion_list.end() ; matchedChildPtr++)
	{


		//there might be several passes for the same object due to multiple splits. we have to make sure we only add one single history list per frame
		if(glbl_var.BGmodel->get_t() != (*(*(*matchedChildPtr)->history.begin())->begin())->frameNumber) //(*(*(*matchedChildPtr)->history.begin())->begin())->frameNumber
		{
			if(glbl_var.saveFramesToHard)
				glbl_var.BGmodel->objectFile << "creating new current history line for object " << (*matchedChildPtr)->blob->ID <<endl;

			(*matchedChildPtr)->currentHistoryMeasurement = new list<historyPoint*>;

			(*matchedChildPtr)->history.push_front((*matchedChildPtr)->currentHistoryMeasurement);
			
			//previousHistoryMeasurement = (*(history.begin()++));
		}

		previousHistoryMeasurement = (*(++(*matchedChildPtr)->history.begin()));
		for(previousHistoryPoint = previousHistoryMeasurement->begin(); previousHistoryPoint != previousHistoryMeasurement->end() ; previousHistoryPoint++)
		{
			if((*previousHistoryPoint)->relatedParent ==this)
				break;
		}


		//if((*matchedChildPtr)->updateCandidate) continue; //only nonmatches 





		//check whether the child is visible or invisible. If it is visible then one history point is needed. Otherwise, as many as the matched CCs are needed
		int count = 0;
		if((*matchedChildPtr)->updateCandidate)
		{
			historyPoint* newPoint = new historyPoint();
			newPoint->frameNumber = glbl_var.BGmodel->get_t();
			newPoint->time = glbl_var.sampledAbsoluteTime;
			//newPoint->relatedParent = (*matchedChildPtr);
			newPoint->relatedCC = (*matchedChildPtr)->updateCandidate;
			(*matchedChildPtr)->currentHistoryMeasurement->clear();//JUST ADDED
			(*matchedChildPtr)->currentHistoryMeasurement->push_front( newPoint );
			count++;

			//if(glbl_var.saveFramesToHard)
			//	glbl_var.BGmodel->objectFile << "  creating new history point for object " << (*matchedChildPtr)->blob->ID << " with itself as parent"<<endl;

			if(glbl_var.saveFramesToHard)
				glbl_var.BGmodel->objectFile << "  creating new history point for object " << (*matchedChildPtr)->blob->ID << "with " << newPoint->relatedCC->blob->ID << " as CCparent." <<endl;

		}
		else
		{
			for(CCiterator = matchedCCs->begin() ;CCiterator != matchedCCs->end()  ; CCiterator++)
			{
				if((*CCiterator)->objects.empty()) continue;
				if((*matchedChildPtr)->findHPWithRelatedCC(*CCiterator)) continue;

				historyPoint* newPoint = new historyPoint();
				newPoint->frameNumber = glbl_var.BGmodel->get_t();
				newPoint->time = glbl_var.sampledAbsoluteTime;
				//newPoint->relatedParent = (*(*CCiterator)->objects.rbegin()); 
				newPoint->relatedCC = (*CCiterator);
				(*matchedChildPtr)->currentHistoryMeasurement->push_front( newPoint );
				count++;

				if(glbl_var.saveFramesToHard)
					glbl_var.BGmodel->objectFile << "  creating new history point for object " << (*matchedChildPtr)->blob->ID << "with " << newPoint->relatedCC->blob->ID << " as CCparent." <<endl;
			}
		}

		//the rest of the properties will be updates later in CC-Obj matching
		//we can't update position because we haven't created the parents yet
		//newPoint->visibility = false;
		//newPoint->relatedParent = candidateResolveHistoryParentList; 
		//newPoint->falls = hasFallen();
		//newPoint->xBase = (*(*matchedChildPtr)->currentHistoryMeasurement->begin())->xBase;
		//newPoint->yBase = (*(*matchedChildPtr)->currentHistoryMeasurement->begin())->yBase;


		//now for all created history points
		int i=0;

		//there is only one previous histor point for the splitting object
		for(currentHistoryPoint = (*matchedChildPtr)->currentHistoryMeasurement->begin() ; i < count ;currentHistoryPoint++ )//currentHistoryPoint != (*matchedChildPtr)->currentHistoryMeasurement->end()
		{
			i++;
			motionPossibility* foundMP= (*currentHistoryPoint)->findMPWithPrecedingObj(this);
			if (!foundMP)
			{


				//i++;
				//find a previous history point related to the parent
				//for(previousHistoryPoint = previousHistoryMeasurement->begin() ; previousHistoryPoint != previousHistoryMeasurement->end() ; previousHistoryPoint++ )
				//{
				//if((*previousHistoryPoint)->relatedParent == candidateResolveHistoryParentList)
				//if((*previousHistoryPoint)->relatedParent == (*matchedChildPtr)->candidateResolveHistoryParentList)
				//if(find((*matchedChildPtr)->candidateResolveHistoryParentList.begin(), (*matchedChildPtr)->candidateResolveHistoryParentList.end(),(*previousHistoryPoint)->relatedParent) !=  (*matchedChildPtr)->candidateResolveHistoryParentList.end())
				//for(matchedParentPtr = (*matchedChildPtr)->candidateResolveHistoryParentList.begin() ; matchedParentPtr != (*matchedChildPtr)->candidateResolveHistoryParentList.end() ;matchedParentPtr++)
				//{
				//	if ((*matchedParentPtr) != this) continue;

				//	//make sure the hp doesn't have this specific mp from before
				//	bool found = false;
				//	//for(mp = (*previousHistoryPoint)->motionPossibilities.begin() ; mp != (*previousHistoryPoint)->motionPossibilities.end() ; mp++)
				//	for(mp = (*currentHistoryPoint)->motionPossibilities.begin() ; mp != (*currentHistoryPoint)->motionPossibilities.end() ; mp++)
				//	{
				//		if ((*mp)->precedingPoint == (*previousHistoryPoint))
				//		{
				//			found = true;
				//			break;
				//		}
				//	}
				//	if (found) continue;

				motionPossibility* newPossibility = new motionPossibility();
				//newPoint->motionPossibilities.push_front(newPossibility);
				(*currentHistoryPoint)->motionPossibilities.push_front(newPossibility);
				newPossibility->precedingPoint = (*previousHistoryPoint);	

				if(glbl_var.saveFramesToHard)
					glbl_var.BGmodel->objectFile << "  creating new possibility for object " << (*matchedChildPtr)->blob->ID << " leading back in time to object" << (*previousHistoryPoint)->relatedParent->blob->ID <<endl;

				break;



			}
		}



	}




}

void  TrackerObject::handleHistoryMergeChild(list<TrackerObject*>* mergingObjectsList,TrackerObject* parentObject, bool resumeSplit)//called by the child of the "object of merge"
{
	if(glbl_var.saveFramesToHard)
		glbl_var.BGmodel->objectFile << "preparing merging child " << blob->ID  <<endl;

	//populate the occlusion list of the parent if not done yet
	(*parentObject->currentHistoryMeasurement->begin())->occlusion.push_front(this);
	if(glbl_var.saveFramesToHard)
		glbl_var.BGmodel->objectFile << "object " <<  blob->ID << " added to occlusion of object " << parentObject->blob->ID;

	list<TrackerObject*>::iterator itrList;
	list<historyPoint*>::iterator itrHistory;
	list<motionPossibility*>::iterator mp;
	list<historyPoint*>::iterator itrHistory2;
	list<motionPossibility*>::iterator itrPossibilities;

	list<historyPoint*>* tminus1History;
	list<historyPoint*>* tHistory;
	list<list<historyPoint*>*>::iterator historyHead = history.begin();

	historyPoint* foundHP;
	motionPossibility* foundMP;

	//handle newest history line
	if((*historyHead)->empty() )
	{
		tHistory = *historyHead;
		historyHead++; //move it to the previous one
		tminus1History = *historyHead;

		historyPoint* newPoint = new historyPoint();
	}
	else if( (*(*historyHead)->begin())->frameNumber != glbl_var.BGmodel->get_t())
	{
		tminus1History = *historyHead;
		tHistory = new list<historyPoint*>;


		history.push_front(tHistory);

		currentHistoryMeasurement = tHistory;

		if(glbl_var.saveFramesToHard)
			glbl_var.BGmodel->objectFile << " new history line has been created " <<endl;
	}
	else //if a current history line has already been created in the split
	{
		tHistory = *historyHead;
		historyHead++; //move it to the previous one
		tminus1History = *historyHead;

	}


	//list<historyPoint*>::iterator ptr = obj->history.begin();
	list<historyPoint*>::iterator ptr;
	list<list<historyPoint*>>::iterator ptr2;
	list<TrackerObject*>::iterator ptrObj;

	double t1 = glbl_var.sampledAbsoluteTime;
	double t2 = (*lastHistoryMeasurement->begin())->time;


	//calculate motion characteristics every TIME_MEASUREMENT_STEP
	double time_difference = t1 - t2;


	list<motionPossibility*>::iterator possibilityIterator;
	list<historyPoint*>::iterator historyIterator; 

	//if(glbl_var.saveFramesToHard)
	//	glbl_var.BGmodel->objectFile << " no time measurement update" ;

	//in case of merging, an object has different possible speeds if it was associated with different objects in the previous frame
	for(itrList = mergingObjectsList->begin() ; itrList != mergingObjectsList->end() ; itrList++) //for each merging object
	{
		TrackerObject* matchTemp;
		if((*itrList)->rearrangeParents) //if it was in a split
		{
			matchTemp = (*itrList)->rearrangeParents;
		}
		else
		{
			matchTemp = (*itrList);
		}

		for(itrHistory = tminus1History->begin() ; itrHistory != tminus1History->end() ; itrHistory++) //for each possible historyPoint
		{
			//if((*itrHistory)->relatedParent == (*itrList)) //if the original object in the previous frame has been found
			if((*itrHistory)->relatedParent == matchTemp)
			{
				// find out whether the history point has already established during the split
				//bool foundHP = false;
				foundHP = findHPWithRelatedCC(parentObject->CC);
				if(!foundHP) //create
				{
					foundHP = new historyPoint();
					//tHistory->push_front(newPoint);
					tHistory->push_back(foundHP);

				}
				else
				{
					if(glbl_var.saveFramesToHard)
						glbl_var.BGmodel->objectFile << " history point with related CCparent " << parentObject->CC->blob->ID<< " has already been found"  <<endl;

					foundMP = foundHP->findMPWithPrecedingObj(matchTemp);
					//foundMP = foundHP->findMPWithPrecedingObj(*itrList);
					if(foundMP)
					{
						if(glbl_var.saveFramesToHard)
							glbl_var.BGmodel->objectFile << "  it alread has a precedign point to " << (*itrHistory)->relatedParent->blob->ID << " and will be recreated" <<endl;

						foundHP->motionPossibilities.remove(foundMP);
						

					}
				}


				//for(itrHistory2 = tHistory->begin() ; itrHistory2 != tHistory->end() ; itrHistory2++) 
				//{

				//	if((*itrHistory2)->relatedParent == parentObject)
				//	{
				//		foundHP = true;
				//		newPoint = (*itrHistory2);

				//		if(glbl_var.saveFramesToHard)
				//			glbl_var.BGmodel->objectFile << " history point with related parent " << (*itrHistory2)->relatedParent->blob->ID<< " has already been found"  <<endl;



				//		for(mp = (*itrHistory2)->motionPossibilities.begin() ; mp != (*itrHistory2)->motionPossibilities.end() ; mp++ )
				//		{

				//			if(((*mp)->precedingPoint == (*itrHistory) ) ) //&& (parentObject == (*itrHistory2)->relatedParent)
				//				//if((*(*itrHistory2)->motionPossibilities.begin())->precedingPoint == (*itrHistory)) //there can only be one motion possibility at this point
				//			{
				//				foundMP = true;
				//				if(glbl_var.saveFramesToHard)
				//					glbl_var.BGmodel->objectFile << "  it alread has a precedign point to " << (*itrHistory)->relatedParent->blob->ID  <<endl;


				//				//newPoint = (*itrHistory2);

				//				//if(glbl_var.saveFramesToHard)
				//				//	glbl_var.BGmodel->objectFile << " history point with related parent " << (*itrHistory2)->relatedParent->blob->ID<< " has already been found"  <<endl;


				//				//for(itrPossibilities = (*itrHistory2)->motionPossibilities.begin() ; itrPossibilities != (*itrHistory2)->motionPossibilities.end() ; itrPossibilities++) //for each possible historyPoint
				//				//{
				//				//if((*itrPossibilities)->precedingPoint->relatedParent == matchTemp)
				//				//if((*mp)->precedingPoint->relatedParent == matchTemp)
				//				//{
				//				//(*itrHistory2)->motionPossibilities.erase(itrPossibilities); //erase it because it will be created again
				//				(*itrHistory2)->motionPossibilities.erase(mp);
				//				//break;
				//				//}
				//				//}
				//				break;

				//				/*goto label2;*/

				//			}

				//		}

				//		//goto label2;
				//	}
				//}

				//label2:
				//if(!foundHP) //create
				//{
				//	newPoint = new historyPoint();
				//	//tHistory->push_front(newPoint);
				//	tHistory->push_back(newPoint);

				//}

			

				//since the parent is visible, there is only one history point
				//xBase and yBase are the parent's if the parent doesn't cover the old position. Otherwise, keep the old position
				//if(!intersect(blob,parentObject->CC->blob) )
				if(!glbl_var.BGmodel->pixelIntersect(this,parentObject->CC,false))
				{
					foundHP->xBase = (*(parentObject->currentHistoryMeasurement->begin()))->xBase;
					foundHP->yBase = (*(parentObject->currentHistoryMeasurement->begin()))->yBase;
				}
				else
				{
					foundHP->xBase = (*itrHistory)->xBase;
					foundHP->yBase = (*itrHistory)->yBase;
				}

				foundHP->frameNumber = glbl_var.BGmodel->get_t();
				foundHP->time = glbl_var.sampledAbsoluteTime;
				foundHP->relatedCC = parentObject->CC;
				foundHP->updateFromPointUsingObject(*itrHistory,parentObject,this,true);


				if(glbl_var.saveFramesToHard)
					glbl_var.BGmodel->objectFile << "  new possibility has been created, with preceding point to object " << (*itrHistory)->relatedParent->blob->ID <<endl;

				if ( time_difference >= TIME_MEASUREMENT_STEP)
				{
					if(glbl_var.saveFramesToHard)
						glbl_var.BGmodel->objectFile << " executing time measurement update "  <<endl;

					foundHP->updateByObject(this); //,true
					newMeasurement = true;

				}

				//for resuming pending splits, move the point to the back because it shouldn't be reached again
			/*	if(resumeSplit && foundHP)
				{
					tHistory->push_back(*itrHistory2);
					tHistory->erase(itrHistory2);
				}*/


				//break;
			}
		}
	}

}


motionPossibility::motionPossibility()
{
	vxBase = 0; 
	vyBase = 0; 
	speed = 0;
	xDirection = 0;
	yDirection = 0;
	//valid = false;


	precedingPoint = NULL; 
	valid = false;

	ID = -1;//default invalid
}

//void motionPossibility::copyPossibility(motionPossibility* mp)
//{
//	vxBase = mp->vxBase;
//	vyBase = mp->vyBase;
//	speed = mp->speed;
//	xDirection = mp->xDirection;
//	yDirection = mp->yDirection;
//
//}

void TrackerObject::updateObjAndChildrenHistories_split(ConnectedComponent* CC)
{
	//base
	float dummy_z[2];
	dummy_z[0] = 0;
	dummy_z[1] = glbl_var.BGmodel->AVERAGE_PERSON_HEIGHT;


	list<TrackerObject*>::iterator itrObjects;
	list<historyPoint*>::iterator itrHistory;
	list<historyPoint*>::iterator itrHistory2;
	list<motionPossibility*>::iterator itrPossibility;
	bool addToHistory;

	list<TrackerObject*> objectAndChildren;
	objectAndChildren  = occlusion_list;
	objectAndChildren.push_front(this);

	historyPoint* hp;
	historyPoint* mp;

	if(glbl_var.saveFramesToHard)
		glbl_var.BGmodel->objectFile << "-updating history of split object " << blob->ID << " and its children"  <<endl;
	for(itrObjects = objectAndChildren.begin() ; itrObjects != objectAndChildren.end() ; itrObjects++)
	{
		hp = findHPWithRelatedCC(CC);
		if(hp)
		{

		//for(itrHistory2 = (*itrObjects)->currentHistoryMeasurement->begin(); itrHistory2 != (*itrObjects)->currentHistoryMeasurement->end() ; itrHistory2++)
		//{
		//	if((*itrHistory2)->relatedParent != this) continue; //check only history points with relatedParent as the caller 

			if(glbl_var.CameraCalibrated) //meters
			{
				glbl_var.cam->imageToWorld(CC->xBase*glbl_var.camCalXRatio,CC->yBase*glbl_var.camCalYRatio,dummy_z[0],hp->xBase,hp->yBase);
				glbl_var.cam->imageToWorld(CC->xTop*glbl_var.camCalXRatio,CC->yTop*glbl_var.camCalYRatio,dummy_z[1],hp->xTop,hp->yTop);
			}
			else //pixels
			{
				hp->xBase = CC->xBase;
				hp->yBase = CC->yBase;

				hp->xTop = CC->xTop;
				hp->yTop = CC->yTop;

			}
			//newPoint->frameNumber = glbl_var.BGmodel->get_t();
			//newPoint->time = glbl_var.sampledAbsoluteTime;


			if(glbl_var.saveFramesToHard)
				glbl_var.BGmodel->objectFile << " history point location: " << hp->xBase << " , " << hp->yBase <<endl;


			//hp->motionPossibilities.pop_back();
			//for(itrPossibility = hp->motionPossibilities.begin(); itrPossibility != hp->motionPossibilities.end() ; itrPossibility++)
			//{

				//for all history points and possibilities, 




				//base speed
				//list<historyPoint*>::iterator ptr = obj->history.begin();
				list<historyPoint*>::iterator ptr;
				list<list<historyPoint*>>::iterator ptr2;
				list<TrackerObject*>::iterator ptrObj;

				double t1 = glbl_var.sampledAbsoluteTime;
				double t2 = (*(*itrObjects)->lastHistoryMeasurement->begin())->time;


				//calculate motion characteristics every TIME_MEASUREMENT_STEP
				double time_difference = t1 - t2;
				/*			if ( time_difference >= TIME_MEASUREMENT_STEP)
				{
				if(glbl_var.saveFramesToHard)
				glbl_var.BGmodel->objectFile << " executing time measurement update "  <<endl;

				newPoint->updateByObject(obj,(*itrObjects) != obj); 
				obj->newMeasurement = true;

				}
				*/		/*else
				{*/
				list<historyPoint*>* tminus1History;
				/*if (addToHistory)
				tminus1History = (*(*itrObjects)->history.begin());
				else*/
				tminus1History = (*(++(*itrObjects)->history.begin()));

				list<motionPossibility*>::iterator possibilityIterator;
				list<historyPoint*>::iterator historyIterator; 

				//if(glbl_var.saveFramesToHard)
				//	glbl_var.BGmodel->objectFile << " no time measurement update" ;


			/*	for(historyIterator = tminus1History->begin(); historyIterator != tminus1History->end() ; historyIterator++)
				{


					if( (*itrPossibility)->precedingPoint == (*historyIterator))
					{
						if(glbl_var.saveFramesToHard)
							glbl_var.BGmodel->objectFile << " history point pointing back to its match in the previous frame with related parent " << (*historyIterator)->relatedParent->blob->ID <<endl;
*/

						hp->updateFromPointUsingObject_split(this);
			/*			break;
					}
				}*/

				if(glbl_var.saveFramesToHard)
					glbl_var.BGmodel->objectFile << endl ;



				/*}*/

				if ( time_difference >= TIME_MEASUREMENT_STEP)
				{
					if(glbl_var.saveFramesToHard)
						glbl_var.BGmodel->objectFile << " executing time measurement update "  <<endl;

					hp->updateByObject(*itrObjects); //,(*itrObjects) != this;  this, 

					if(this == (*itrObjects)) 
						object_classification_update(this);
					(*itrObjects)->newMeasurement = true;

				}

			//}


		}
	}	
}

bool TrackerObject::itOrChildrenPersistent()
{
	list<TrackerObject*>::iterator ptrObj;
	if (persistent) return true;
	for(ptrObj = occlusion_list.begin() ; ptrObj != occlusion_list.end(); ptrObj++)
	{
		if ((*ptrObj)->persistent) 
		{
			persistent = true;
			return true;
		}
	}

	return false;
}


void TrackerObject::updateObjAndChildrenHistories(ConnectedComponent* CC)
{
	//base
	float dummy_z[2];
	dummy_z[0] = 0;
	dummy_z[1] = glbl_var.BGmodel->AVERAGE_PERSON_HEIGHT;



	list<TrackerObject*>::iterator itrObjects;
	list<historyPoint*>::iterator itrHistory;
	bool addToHistory;

	list<TrackerObject*> objectAndChildren;
	objectAndChildren  = occlusion_list;
	objectAndChildren.push_front(this);

	if(glbl_var.saveFramesToHard)
		glbl_var.BGmodel->objectFile << "-updating history of object " << blob->ID << " and its children"  <<endl;
	for(itrObjects = objectAndChildren.begin() ; itrObjects != objectAndChildren.end() ; itrObjects++)
	{
		historyPoint* newPoint;

		//if a history point is already found for the object, return it. If not, create it
		if( !(*itrObjects)->currentHistoryMeasurement || (!(*itrObjects)->currentHistoryMeasurement->empty() && glbl_var.BGmodel->get_t() != (*(*itrObjects)->currentHistoryMeasurement->begin())->frameNumber)  )
		{
			if(glbl_var.saveFramesToHard)
				glbl_var.BGmodel->objectFile << " new history line and point created for object " << (*itrObjects)->blob->ID  <<endl;

			(*itrObjects)->currentHistoryMeasurement = new list<historyPoint*>;
			newPoint = new historyPoint();
			newPoint->relatedParent = this;
			(*itrObjects)->currentHistoryMeasurement->push_front(newPoint);
			addToHistory = true;

		}
		else if ((*itrObjects)->currentHistoryMeasurement->empty() )
		{
			if(glbl_var.saveFramesToHard)
				glbl_var.BGmodel->objectFile << " new history point created for object " << (*itrObjects)->blob->ID  <<endl;


			newPoint = new historyPoint();
			newPoint->relatedParent = this;
			(*itrObjects)->currentHistoryMeasurement->push_front(newPoint);
			addToHistory = false;

		}
		else
		{
			list<historyPoint*>* currentHistoryPtr = (*itrObjects)->currentHistoryMeasurement;

			bool  found = false;
			for (itrHistory = currentHistoryPtr->begin() ; itrHistory != currentHistoryPtr->end() ; itrHistory++)
			{
				//if((*itrHistory)->relatedParent == this)
				if((*itrHistory)->relatedCC == CC)
				{
					found = true;
					break;
				}
			}
			if(found)
			{
				if(glbl_var.saveFramesToHard)
					glbl_var.BGmodel->objectFile << " history point already exists for object " << (*itrObjects)->blob->ID  <<endl;

				newPoint = (*itrHistory);
			}
			else
			{
				if(glbl_var.saveFramesToHard)
					glbl_var.BGmodel->objectFile << " no matching history point found. New history point created for object " << (*itrObjects)->blob->ID  <<endl;


				newPoint = new historyPoint();
				newPoint->relatedParent = this;
				(*itrObjects)->currentHistoryMeasurement->push_front(newPoint);

			}

			addToHistory = false;



		}



		newPoint->frameNumber = glbl_var.BGmodel->get_t();
		newPoint->time = glbl_var.sampledAbsoluteTime;



		if(glbl_var.saveFramesToHard)
			glbl_var.BGmodel->objectFile << " history point location: " << newPoint->xBase << " , " << newPoint->yBase <<endl;


		//base speed
		bool groundPossibility = false; //used for the default possibility of the first history point
		if(!(*itrObjects)->history.empty()) //not first time
		{
			//list<historyPoint*>::iterator ptr = obj->history.begin();
			list<historyPoint*>::iterator ptr;
			list<list<historyPoint*>>::iterator ptr2;
			list<TrackerObject*>::iterator ptrObj;

			double t1 = glbl_var.sampledAbsoluteTime;
			double t2 = (*(*itrObjects)->lastHistoryMeasurement->begin())->time;


			//calculate motion characteristics every TIME_MEASUREMENT_STEP
			double time_difference = t1 - t2;
			/*			if ( time_difference >= TIME_MEASUREMENT_STEP)
			{
			if(glbl_var.saveFramesToHard)
			glbl_var.BGmodel->objectFile << " executing time measurement update "  <<endl;

			newPoint->updateByObject(obj,(*itrObjects) != obj); 
			obj->newMeasurement = true;

			}
			*/		/*else
			{*/

			//find the PARENT'S t-1 history
			list<historyPoint*>* parent_tminus1History;
			list<historyPoint*>* tminus1History;
			list<list<historyPoint*>*>::iterator hist_itr;
			if (addToHistory && this == (*itrObjects))
				parent_tminus1History = (*(this->history.begin()));
			else
				parent_tminus1History = (*(++(this->history.begin())));

			if(this == (*itrObjects))
			{
				tminus1History = parent_tminus1History;
			}
			else
			{
				int frame_tminus1 = (*parent_tminus1History->begin())->frameNumber;
				list<list<historyPoint*>*>::iterator itrBegin;
				if (addToHistory)
					itrBegin = ((*itrObjects)->history.begin());
				else
					itrBegin = (++((*itrObjects)->history.begin()));
				for(hist_itr = itrBegin; hist_itr != (*itrObjects)->history.end() ; hist_itr++)
				{
					if (frame_tminus1 == (*(*hist_itr)->begin())->frameNumber)
					{
						tminus1History = (*hist_itr);
						break;
					}
				}

			}

			list<motionPossibility*>::iterator possibilityIterator;
			list<historyPoint*>::iterator historyIterator; 

			//if(glbl_var.saveFramesToHard)
			//	glbl_var.BGmodel->objectFile << " no time measurement update" ;

			for(historyIterator = tminus1History->begin(); historyIterator != tminus1History->end() ; historyIterator++)
			{
			


				//if( this == (*historyIterator)->relatedParent)
				if( ( rearrangeParents ? rearrangeParents : this) == (*historyIterator)->relatedParent)
				{
					if(glbl_var.saveFramesToHard)
						glbl_var.BGmodel->objectFile << " history point pointing back to its match in the previous frame with related parent " << (*historyIterator)->relatedParent->blob->ID <<endl;

					

					if((*itrObjects) == this) //if the parent update by means of the CC
					{
						if(glbl_var.CameraCalibrated) //meters
						{
							glbl_var.cam->imageToWorld(CC->xBase*glbl_var.camCalXRatio,CC->yBase*glbl_var.camCalYRatio,dummy_z[0],newPoint->xBase,newPoint->yBase);
							glbl_var.cam->imageToWorld(CC->xTop*glbl_var.camCalXRatio,CC->yTop*glbl_var.camCalYRatio,dummy_z[1],newPoint->xTop,newPoint->yTop);
						}
						else //pixels
						{
							newPoint->xBase = CC->xBase;
							newPoint->yBase = CC->yBase;

							newPoint->xTop = CC->xTop;
							newPoint->yTop = CC->yTop;

						}
					}
					else //update by means of last history
					{

						//if(!intersect((*itrObjects)->blob,CC->blob) )
						if(!glbl_var.BGmodel->pixelIntersect((*itrObjects),CC,false) )
						{
							newPoint->xBase = (*this->currentHistoryMeasurement->begin())->xBase; //since (this) is the parent, it has only one hp
							newPoint->yBase = (*this->currentHistoryMeasurement->begin())->yBase;//since (this) is the parent, it has only one hp

							newPoint->xTop = (*this->currentHistoryMeasurement->begin())->xTop; //since (this) is the parent, it has only one hp
							newPoint->yTop = (*this->currentHistoryMeasurement->begin())->yTop;//since (this) is the parent, it has only one hp
						}
						else
						{

							newPoint->xBase = (*historyIterator)->xBase;
							newPoint->yBase = (*historyIterator)->yBase;

							newPoint->xTop = (*historyIterator)->xTop;
							newPoint->yTop = (*historyIterator)->yTop;
						}

					}


					newPoint->relatedCC = CC;
					newPoint->updateFromPointUsingObject(*historyIterator,this,(*itrObjects),false);
					break;
				}
			}

			if(glbl_var.saveFramesToHard)
				glbl_var.BGmodel->objectFile << endl ;




			if ( time_difference >= TIME_MEASUREMENT_STEP)
			{
				if(glbl_var.saveFramesToHard && glbl_var.saveObjectFile)
					glbl_var.BGmodel->objectFile << " executing time measurement update "  <<endl;



				newPoint->updateByObject((*itrObjects) ); //,(*itrObjects) != this;  this, 
				(*itrObjects)->newMeasurement = true;

				//object_classification_update(this);
				if(this == (*itrObjects)) 
					object_classification_update(this);


			}

		}
		else //first time
		{

			if(glbl_var.CameraCalibrated) //meters
			{
				glbl_var.cam->imageToWorld(CC->xBase*glbl_var.camCalXRatio,CC->yBase*glbl_var.camCalYRatio,dummy_z[0],newPoint->xBase,newPoint->yBase);
				glbl_var.cam->imageToWorld(CC->xTop*glbl_var.camCalXRatio,CC->yTop*glbl_var.camCalYRatio,dummy_z[1],newPoint->xTop,newPoint->yTop);
			}
			else //pixels
			{
				newPoint->xBase = CC->xBase;
				newPoint->yBase = CC->yBase;

				newPoint->xTop = CC->xTop;
				newPoint->yTop = CC->yTop;

			}
			newPoint->visibility = true;
			newPoint->falls = this->hasFallen();
			newPoint->occlusion.push_front(this);

			//add a first dummy possibility
			motionPossibility* newPossibility = new motionPossibility();
			newPossibility->valid = true;
			newPoint->motionPossibilities.push_front(newPossibility);

			(*itrObjects)->newMeasurement = true;
			groundPossibility = true;
		}

		if(addToHistory)
		{
			if (!newPoint->motionPossibilities.empty() &&  !groundPossibility) 
			{
				//(*newPoint->motionPossibilities.begin())->precedingPoint = (*(*(*itrObjects)->history.begin())->begin());
				int uu ;
				uu = 0;
			}


			(*itrObjects)->history.push_front((*itrObjects)->currentHistoryMeasurement);
		}
	}
}

void TrackerObject::adjustItAndChildrenRelatedParentToPrecedingParent() //replace its history points' and its children's relatedParents with the rearrangeParents which is the previous parent
{

	list<TrackerObject*>::iterator itrObjects;
	list<historyPoint*>::iterator itrHistory;
	list<TrackerObject*> objList = occlusion_list;
	objList.push_front(this);

	for(itrObjects = objList.begin(); itrObjects != objList.end() ; itrObjects++ )
	{
		for(itrHistory = (*itrObjects)->currentHistoryMeasurement->begin(); itrHistory != (*itrObjects)->currentHistoryMeasurement->end() ; itrHistory++ )
		{
			if((*itrHistory)->relatedParent == rearrangeParents)
				(*itrHistory)->relatedParent = this;
		}
	}

}


historyPoint* TrackerObject::findHPWithRelatedCC(ConnectedComponent* CC)
{
	historyPoint* result = NULL;
	list<historyPoint*>::iterator itr;
	for(itr = currentHistoryMeasurement->begin() ; itr != currentHistoryMeasurement->end() ; itr++)
	{
		if((*itr)->relatedCC == CC)
		{
			result = (*itr);
			break;
		}
	}

	return result;
}

historyPoint* TrackerObject::findHPWithRelatedObj(TrackerObject* Obj)
{
	historyPoint* result = NULL;
	list<historyPoint*>::iterator itr;
	for(itr = currentHistoryMeasurement->begin() ; itr != currentHistoryMeasurement->end() ; itr++)
	{
		if((*itr)->relatedParent == Obj)
		{
			result = (*itr);
			break;
		}
	}

	return result;
}


motionPossibility* historyPoint::findMPWithPrecedingObj(TrackerObject* Obj)
{
	motionPossibility* result = NULL;
	list<motionPossibility*>::iterator itr;
	for(itr = motionPossibilities.begin() ; itr != motionPossibilities.end() ; itr++)
	{
		if((*itr)->precedingPoint->relatedParent == Obj)
		{
			result = (*itr);
			break;
		}
	}

	return result;


}


motionPossibility* historyPoint::findMPWithPrecedingHP(historyPoint* hp)
{
	motionPossibility* result = NULL;
	list<motionPossibility*>::iterator itr;
	for(itr = motionPossibilities.begin() ; itr != motionPossibilities.end() ; itr++)
	{
		if((*itr)->precedingPoint == hp)
		{
			result = (*itr);
			break;
		}
	}

	return result;


}

