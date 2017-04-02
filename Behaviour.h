#ifndef B_BEH
#define B_BEH

//#include "stdafx.h"
#include <cv.h>
#include <cxcore.h>
#include <highgui.h>
#include <math.h>
#include <iostream>
#include "Tracker2.h"
#include <fstream>
#pragma once
#using <mscorlib.dll>
using namespace System;
using namespace std;


class twoHistoryPointsInteraction;
class object1twoObjectHistoryPoint;

public class twoObjecttimeDependentInformation
{
public:
	double distance_change;
	bool alignment_or_ParentDirection_Changed; // used for fighting
	twoHistoryPointsInteraction* precedingPoint;

	twoObjecttimeDependentInformation()
	{
		distance_change = 0;
		alignment_or_ParentDirection_Changed = false;
		precedingPoint = NULL;
	}

	void updateTimeDependentInformation(twoHistoryPointsInteraction*,twoHistoryPointsInteraction*);

};

public class twoPossibilitiesInteraction
{
public:
	double alignment; //angle between directions of two objects	
	double velocity_difference; //difference in velocity between two objects

	//these statuses are used to make inferences that need time accumulation
	bool sameSpeedDirection; //used for walking together
	bool moving; //if false, means one of the possibilities is not moving.
	bool drags; //used for a person dragging an object

	motionPossibility* mp2;
	motionPossibility* mp1;

	twoHistoryPointsInteraction* preceding;

	twoPossibilitiesInteraction()
	{
		alignment = -100; //invalid default
		velocity_difference = -1;
		sameSpeedDirection = false;
		mp1 = NULL;
		mp2 = NULL;
		preceding = NULL;
		moving = false;
		drags = false;
	}

	void updateTwoPossibilitiesInteraction(motionPossibility* , motionPossibility*, historyPoint*, historyPoint*, TrackerObject*, TrackerObject*);
	//twoPossibilitiesInteraction();
};



public class Object1twoObjectPossibility
{
public:
	list<twoPossibilitiesInteraction*> possibilitiesObj2;
	
	motionPossibility* mp1;

	Object1twoObjectPossibility()
	{
		mp1 = NULL;
	}
};

public class twoHistoryPointsInteraction
{
public:
	double distance; // distance between positions of two objects
	bool takes; // used for theft
	bool merged; //used for fighting, samespeeddirection
	bool isAbandonedBy; //hp1 for object, hp2 for person
	bool hasBeenAbandonedBy;
	bool meeting;
	bool hasBeenMeeting;

	historyPoint* hp2;
	historyPoint* hp1;

	list<twoObjecttimeDependentInformation*> timeDependent; //each distance can result several distance changes in time;
	list<Object1twoObjectPossibility*> possibilitiesObj1;

	//TrackerObject* relatedParent1;
	//TrackerObject* relatedParent2;
	float time;
	int frameNumber;


	twoHistoryPointsInteraction()
	{
		hp1 = NULL;
		hp2 = NULL;
		distance = -1;//invalid default
		takes = false;
		merged = false;

		//the first for an instance, the second for the periond
		isAbandonedBy = false;
		hasBeenAbandonedBy = false;

		//the first for an instance, the second for the periond
		meeting = false;
		hasBeenMeeting = false;

		time = -1; //invalid default
		frameNumber = -1; //invalid default
	}

	void updateTwoHistoryPointsInteraction(historyPoint* , historyPoint*, list<object1twoObjectHistoryPoint*>*, TrackerObject*, TrackerObject*);
	bool checkPairPrecedingAbandoned(); //used to go back in time and check if abandoned object event has lasted long enough
	bool checkPairPrecedingClose(); //used to go back in time and check if meeting event has lasted long enough
	int checkPairPrecedingMergeChanges(); //returns the number of times merging has finished


};

public class object1twoObjectHistoryPoint
{

public:
	list<twoHistoryPointsInteraction*> historyPointsObj2;

	historyPoint* hp1;

	object1twoObjectHistoryPoint()
	{
		hp1 = NULL;
	}

};

public class object2InteractionHistory
{
public:
	list<list<object1twoObjectHistoryPoint*>*> interactionHistory; //1 Dimension second object
	list<object1twoObjectHistoryPoint*>* lastHistoryMeasurement; // to allow for updates only every TIME_MEASUREMENT_STEP
	list<object1twoObjectHistoryPoint*>* currentHistoryMeasurement; // to allow for updates only every TIME_MEASUREMENT_STEP

	TrackerObject* obj2;
	TrackerObject* obj1;

	bool newMeasurement; //used to update measurement every TIME_MEASUREMENT_STEP

	object2InteractionHistory()
	{
		obj2 = NULL;
		obj1 = NULL;
		lastHistoryMeasurement = NULL;
		currentHistoryMeasurement = NULL;
		newMeasurement = false;

	}

	//~object2InteractionHistory()
	//{
	//	//list<twoPossibilitiesInteraction*>::iterator ptrObj;
	//	//for( ptrObj = objectsInteraction.begin(); ptrObj != objectsInteraction.end() ; ptrObj++)
	//	//	delete (*ptrObj);

	//	objectsInteraction.clear();
	//}

	void updateTwoObjectInteraction(TrackerObject* , TrackerObject*);
	object1twoObjectHistoryPoint* retrieveHistoryPoint(historyPoint* , historyPoint*);
};

public class object1InteractionHistory
{
public:
	list<object2InteractionHistory*> object2list; //1 Dimension first object
	bool active;
	TrackerObject* obj1;

	object1InteractionHistory()
	{
		obj1 = NULL;
		active = true;
	}

	/*~object1InteractionHistory()
	{
	list<object2InteractionHistory*>::iterator ptrObj;
	for( ptrObj = object2list.begin(); ptrObj != object2list.end() ; ptrObj++)
	delete (*ptrObj);

	object2list.clear();
	}*/

	void insertNewObject2(TrackerObject*);

};


public class BehaviourMatrix
{

public:
	list<object1InteractionHistory*> object1list; //2 dimensions for objects 
	double lastUpdateTime;

	//Thresholds
	double OBJECT_MOVED_THRESHOLD; //used for detecting weather an object has been carried/moved away
	double DIFFERENT_ANGLES_THRESHOLD; //used for sameSpeedDirection detection
	double DIFFERENT_SPEEDS_THRESHOLD; //used for sameSpeedDirection detection
	
	int FAINT_DURATION_THRESHOLD; // amount of time before a person is considered fainted

	int LOITERING_THRESHOLD; // threshold for raising the alarm in terms of number of frames
	
	float ABANDONED_LUGGAGE_DISTANCE_THRESHOLD; //threshold for the distance  before a bag is considered abandoned
	int ABANDONED_LUGGAGE_DURATION_THRESHOLD; //threshold for the amount of time before a bag is considered abandoned
	
	float MEETING_DISTANCE_THRESHOLD; // max distance for two people to be potentially fighting
	int WALKING_TOGETHER_TIME_THRESHOLD; //time people need to walk together to detect walking together
	int MEETING_DURATION_THRESHOLD; //time for meeting
	
	float DRAGGING_DISTANCE_THRESHOLD; //threshold to consider an object being dragged

	float FIGHTING_SPEED; //people need to be moving faster than this to be potentially fighting
	int FIGHTING_TIME_SPAN; //needed for checking frequency of occurances for last TIME_SPAN seconds
	float FIGHTING_DISTANCE_THRESHOLD; // max distance for two people to be potentially fighting
	int SPLIT_MERGE_FREQUENCY; //number of merge-split pairs required in the last FIGHTING_TIME_SPAN to detect fighting
	float ALIGNMENT_OR_DIRECTION_CHANGE_THRESHOLD; //amount alignment changes required to detect fighting

	double W_to_H_FAINTING_RATIO; //used for fainitng

	bool detectAbandonedBasedOnHist;



	//auxiliary
	BehaviourMatrix();
	~BehaviourMatrix();
	list<object1twoObjectHistoryPoint*>* retrieveLastHistoryLine(TrackerObject* , TrackerObject*);
	list<object1twoObjectHistoryPoint*>* retrievecurrentHistoryLine(TrackerObject* , TrackerObject*);
	list<list<object1twoObjectHistoryPoint*>*>* retrieveHistoryLine(TrackerObject* , TrackerObject*);

	void updateMatrix();
	void detectBehaviours( );//fills a string with the log
	void createObject(TrackerObject*);
	//void deleteObject(TrackerObject*);
	//void updateTwoObjectInteraction(TrackerObject* , TrackerObject*);
	fstream behaviourFile;

	//high level activities
	bool abandonedObject(TrackerObject* , list<twoHistoryPointsInteraction*>* ); //populates a list of twoHistoryPointsInteraction with the coordinates corrisponding to the abandoned luggage event
	bool stoleLuggage(TrackerObject* , TrackerObject*, list<twoHistoryPointsInteraction*>*,list<TrackerObject*>*);  //populates a list of twoHistoryPointsInteraction with the coordinates corrisponding to the abandoned luggage event
	bool loitering(TrackerObject*);
	bool hasFainted(TrackerObject*);
	bool areFighting(TrackerObject*,TrackerObject*,list<twoHistoryPointsInteraction*>&);
	bool meeting(TrackerObject*,TrackerObject*,list<twoHistoryPointsInteraction*>*);
	bool walkingTogether(TrackerObject*,TrackerObject*,list<twoHistoryPointsInteraction*>* );


	//lower level activities
	bool areSameSpeedDirection(motionPossibility* , motionPossibility* ,TrackerObject*, TrackerObject*);
	//bool areMoving(motionPossibility* , motionPossibility* ,TrackerObject*, TrackerObject*);
	//bool checkAlignmentParentDirectionChange(TrackerObject*, TrackerObject*);
	bool isCarrying(historyPoint* , historyPoint* , TrackerObject* , TrackerObject*);
	//bool isCarrying(TrackerObject* , TrackerObject*);
	bool isFarFrom(TrackerObject* , TrackerObject* , float , list<twoHistoryPointsInteraction*>* ); //populates a list of twoHistoryPointsInteraction with the coordinates corrisponding to the far from
	bool isCloseTo( TrackerObject* , TrackerObject* , float , list<twoHistoryPointsInteraction*>* );
	//bool stoleLuggage(TrackerObject* , TrackerObject*); 
	bool hasTaken(TrackerObject* , TrackerObject* , list<historyPoint*>&, list<historyPoint*>&); //the two lists have coordinates corresponding to "true" //,   
	bool sameDirection(motionPossibility* , motionPossibility* ,double&);





};

double calcAlignment(motionPossibility*,motionPossibility*);
double calcDistance(historyPoint*, historyPoint*);
double calcVelocityDifference(motionPossibility*, motionPossibility*);
double calcDistanceChange(twoHistoryPointsInteraction*, twoHistoryPointsInteraction*);
//bool checkAlignmentParentDirectionChange(twoHistoryPointsInteraction*, twoHistoryPointsInteraction*);
bool areMoving(motionPossibility* , motionPossibility*, historyPoint*, historyPoint* ,  TrackerObject* , TrackerObject* );
bool isDragging(motionPossibility* , motionPossibility* , historyPoint*, historyPoint*, TrackerObject* , TrackerObject* ); //obj1 = person, obj2 = object









#endif