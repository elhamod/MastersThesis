#ifndef B_M
#define B_M


//#include "stdafx.h"
#include <cv.h>
#include <cxcore.h>
#include <highgui.h>
#include <math.h>
#include <iostream>
#pragma once
#using <mscorlib.dll>
using namespace System;
using namespace std;

#define TIME_MEASUREMENT_STEP 1 //in seconds, the time between samplings of speeds, orientations...etc

//object classification enum
typedef enum objClassification { UNKNOWN, PERSON, STILL_PERSON, OBJECT };



class TrackerObject;

public class ConnectedComponent
{
public:

	CvBlob* blob; //x,y,w,h
	float centroid_x;
	float centroid_y;
	float area; //actual object area not the bounding box
	CvHistogram* hist; //used for matching
	int matched;
	float moment; // 2nd moment of the blob, also used for matching
	list<TrackerObject*> objects;
	double distanceFromCC;
	IplImage* blob_img;
	IplImage* blob_mask;
	IplImage* blob_inner_contour;
	IplImage* blob_outer_contour;
	float xBase, yBase; //the bottom most pixel (in pixels only. conversion to meters is done in correlation function)
	float xTop, yTop; //the bottom most pixel (in pixels only. conversion to meters is done in correlation function)
	float contour_distance; //used to elliminate ghosts
	bool ghost;


	ConnectedComponent();
	//ConnectedComponent(const ConnectedComponent &); //for copy by value
	~ConnectedComponent();
	friend void update_Obj_CC(TrackerObject*, ConnectedComponent*, double, bool, bool); //update CC -> Obj

};
class historyPoint; //foreward declaration

public class motionPossibility
{
public:
	double vxBase; //only over an interval
	double vyBase; //only over an interval
	double speed;//only over an interval
	double xDirection, yDirection;//only over an interval

	historyPoint* precedingPoint; //points to the related history point in preceding list of history points.

	bool valid; //if false, then it's just a pathway to a previous reliable one

	int ID; // will be used in behaviour

	//bool valid; //used to check whether a possibility has already been populated or just created (useful for split and then running pending updates)

	motionPossibility();
	//void copyPossibility(motionPossibility*);
	//void updateByObject(TrackerObject *ptr);

};

//a list of this describes a history line of an object
public class historyPoint
{
public:
	double xBase;
	double yBase;
	double xTop;
	double yTop;
	int frameNumber;
	float time;
	bool visibility;
	list<TrackerObject*> occlusion;
	bool falls; //used for fainting
	//offset from starting position might be added later if needed


	int ID; // will be used in behaviour

	//motion possibilities
	list<motionPossibility*> motionPossibilities;

	TrackerObject* relatedParent; //points the parent object in the same point in time
	ConnectedComponent* relatedCC;


	historyPoint();
	void updateFromPointUsingObject(historyPoint*, TrackerObject*,TrackerObject*,bool); 
	void updateByObject(TrackerObject*);//,bool //build from object information (however, xBase and yBase must already be there). The bool parameter is for specifying whether the parent field is updated.
	void updateFromPointUsingObject_split(  TrackerObject* );

	motionPossibility* findMPWithPrecedingObj(TrackerObject*);
	motionPossibility* findMPWithPrecedingHP(historyPoint*);

};

//historyPoint* historyPointOfDifference(historyPoint*,historyPoint*);

public class TrackerObject
{
public:

	CvBlob* blob; //x,y,w,h
	CvBlob* prev_blob; //x,y,w,h in previous frame
	int time; //used for loitering, and maybe other purposes.
	int absence; //can be used to tolerate some absence before declaring a tracked object lost
	int persistence; //used to know how long the object has been visible
	CvHistogram* hist; //used for matching
	CvKalman* kalman; //used for tracking in case of occlusion
	double moment; // 2nd moment of the blob, also used for matching
	bool isVisible; //used for occlusion
	bool matched; //used to treat matched/unmatched objects accordingly
	int occlusion; //used to specify whether it's part of occlusion or not ( the number is the collision group)
	ConnectedComponent* CC; //associated connected components
	double distanceFromCC;
	int type; //0 = bag, 1 = person
	list<TrackerObject*> occlusion_list; //used to insert sub-objects composing this object
	int splitIndex; //used to avoid remerging split objects
	bool occlusion_child;
	list<TrackerObject*> parent;
	//TrackerObject* parent; //assuming u have multiple instances in case of uncertainty
	bool persistent;
	float area;

	//multiple instances
	//bool hasMultipleInstances;
	//list<TrackerObject*> instances;


	bool movingObject; // moving person vs. nonmoving bags
	list<list<historyPoint*>*> history; //history line of the object since its creation (time), with each possibly having multiple history points in case of object uncertainty
	list<historyPoint*>* lastHistoryMeasurement; // a pointer to the history point before speed was last updated, having multiple history points in case of object uncertainty
	list<historyPoint*>* currentHistoryMeasurement; // a pointer to the current history point, having multiple history points in case of object uncertainty
	list<TrackerObject*> candidateResolveHistoryParentList;
	void updateAllSiblingscandidateResolveHistoryParentList(TrackerObject*);
	void prepareHistorySplitChildren(list<ConnectedComponent*>*);
	void prepareHistoryCorrelationChildren();
	void handleHistoryMergeChild(list<TrackerObject*>*,TrackerObject*, bool); //pass a pointer of the list of the objects which participated in the merge
	void resolveHistoryAmbiguities(); //used to resolve any ambiguities from multiple contingencies of an object's whereabouts after a decisive split takes place 
	bool newMeasurement; //used to update measurement every TIME_MEASUREMENT_STEP
	void updateObjAndChildrenHistories(ConnectedComponent*);
	void updateObjAndChildrenHistories_split(ConnectedComponent*);



	//object classification based on the paper by Bird et al. "Real time online detetction.."
	objClassification object_classification;
	objClassification updateCandidate_object_classification;
	int updateCandidate_PersonGroup;
	int PersonGroup; //we are not using it at the moment

	//behaviour related
	TrackerObject* belongs_to;
	CvHistogram* ownerHist; //used for matching the owner to discriminate between owner and thief
	IplImage* blobMask; //used for splitting
	bool walk;
	bool meet;
	bool fight;
	bool faint;
	bool steal;
	bool abandoned;
	bool loiter;



	
	bool toBeDeleted;
	ConnectedComponent* updateCandidate; //used to update after a split only if a merge haven't taken place
	double updateCandidate_colorDistance;
	TrackerObject* rearrangeParents;

	TrackerObject();
	~TrackerObject();
	void Predict(); //predicts using Kalman filter 
	CvBlob predictedPosition(); //gives values of prediction 
	void resetOcclusion();
	bool all_children_matched();
	bool reliable();   //returns false if the object is a child or invisible
	double hasFallen();
	bool simpleIsOrParentVisible();
	bool itOrChildrenPersistent();
	void adjustItAndChildrenRelatedParentToPrecedingParent();

	friend void update_Obj_CC(TrackerObject*, ConnectedComponent*, double, bool,bool); //update CC -> Obj
	friend bool willCollide(list<TrackerObject*>::iterator, list<TrackerObject*>::iterator);
	friend bool intersect(CvBlob*,CvBlob*);
	friend void erase_object(TrackerObject*);
	friend void match_parent(TrackerObject*);
	friend void delete_parent(TrackerObject*);
	friend double colorDistance(TrackerObject*, ConnectedComponent*, int);
	friend void insert_object_occlusionlist(TrackerObject* ,TrackerObject* , TrackerObject* , int );
	
	//friend TrackerObject* findOwner(TrackerObject*); 
	friend void findOwner(TrackerObject*); 
	friend bool areMerged(historyPoint*,historyPoint*);
	friend double calcOffsetFromStratingPoint(historyPoint*, TrackerObject*); 

	
	historyPoint* findHPWithRelatedCC(ConnectedComponent*);
	historyPoint* findHPWithRelatedObj(TrackerObject*);

	//char* displayedText;




};

//void update_Obj_CC(list<TrackerObject>::iterator, list<TrackerObject>::iterator, double);
void update_Obj_CC(TrackerObject*, ConnectedComponent*, double, bool,bool);
TrackerObject* createObject(ConnectedComponent*, int &);
bool willCollide(list<TrackerObject*>::iterator, list<TrackerObject*>::iterator);
bool intersect(CvBlob*,CvBlob*);
void erase_object(TrackerObject*);
void match_parent(TrackerObject*);
void delete_parent(TrackerObject*);
double colorDistance(TrackerObject*, ConnectedComponent*, int);
void insert_object_occlusionlist(TrackerObject* ,TrackerObject* , TrackerObject* ); //, int 

objClassification object_classification_merge(list<TrackerObject*>);
void object_classification_split(TrackerObject* ,TrackerObject* ,int );
void object_classification_update(TrackerObject*);

//TrackerObject* findOwner(TrackerObject*); //will return a pointer to the owner, or NULL if not found or unapplicable
void findOwner(TrackerObject*); //will return a pointer to the owner, or NULL if not found or unapplicable
bool areMerged(historyPoint*,historyPoint*);
double calcOffsetFromStratingPoint(historyPoint*,TrackerObject*);




#endif