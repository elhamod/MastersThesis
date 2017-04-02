#include "stdafx.h"
#include "g_variables.h"
#include <cv.h>
#include <cvaux.h>
#include <highgui.h>


//
//twoHistoryPointsInteraction* retrieveTwoHistoryPointsInteraction(list<object1twoObjectHistoryPoint*> historyLine,historyPoint1* hp1, historyPoint* hp2)
//{
//
//	list<object1twoObjectHistoryPoint*>::iterator ptrHp1;
//	list<twoHistoryPointsInteraction*>::iterator ptrHp2;
//
//	for (ptrHp1= historyLine.begin(); ptrHp1 != historyLine.end(); ptrHp1++)
//	{
//		if ((*ptrHp1)-> == obj1->blob->ID)
//		{
//			for (ptrHp2= (*ptrObj1)->object2list.begin(); ptrHp2 != (*ptrObj1)->object2list.end(); ptrHp2++)
//			{
//				if((*ptrObj2)->obj2->blob->ID == obj2->blob->ID)
//					return &((*ptrObj2)->interactionHistory);
//			}
//		}
//
//	}
//
//	//if not found return NULL
//	return NULL;
//}
//
//object1twoObjectHistoryPoint* BehaviourMatrix::retrieveHistoryPoint(historyPoint* hp1, historyPoint* hp2)
//{
//
//
//	list<object1InteractionHistory*>::iterator ptrObj1;
//	list<object2InteractionHistory*>::iterator ptrObj2;
//	for (ptrObj1= object1list.begin(); ptrObj1 != object1list.end(); ptrObj1++)
//	{
//		if ((*ptrObj1)->obj1->blob->ID == obj1->blob->ID)
//		{
//			for (ptrObj2= (*ptrObj1)->object2list.begin(); ptrObj2 != (*ptrObj1)->object2list.end(); ptrObj2++)
//			{
//				if((*ptrObj2)->obj2->blob->ID == obj2->blob->ID)
//					return &((*ptrObj2)->interactionHistory);
//			}
//		}
//
//	}
//
//	//if not found return NULL
//	return NULL;
//}

list<list<object1twoObjectHistoryPoint*>*>* BehaviourMatrix::retrieveHistoryLine(TrackerObject* obj1, TrackerObject* obj2)
{
	if(glbl_var.saveFramesToHard)
	{
		behaviourFile << "{fetching history lines for objects " << obj1->blob->ID << " and " << obj2->blob->ID  << endl; 
	}


	list<object1InteractionHistory*>::iterator ptrObj1;
	list<object2InteractionHistory*>::iterator ptrObj2;

	TrackerObject* lower_ID;
	TrackerObject* higher_ID;
	//determine the smallest
	if(obj1->blob->ID < obj2->blob->ID)
	{
		lower_ID = obj1;
		higher_ID = obj2;
	}
	else
	{
		lower_ID = obj2;
		higher_ID = obj1;
	}

	for (ptrObj1= object1list.begin(); ptrObj1 != object1list.end(); ptrObj1++)
	{
		if ((*ptrObj1)->obj1 == lower_ID)
		{
			for (ptrObj2= (*ptrObj1)->object2list.begin(); ptrObj2 != (*ptrObj1)->object2list.end(); ptrObj2++)
			{
				if((*ptrObj2)->obj2 == higher_ID)
				{
					if(glbl_var.saveFramesToHard)
					{
						behaviourFile << "history lines for objects " << (*ptrObj1)->obj1->blob->ID << " and " << (*ptrObj2)->obj2->blob->ID  << "FOUND!}"  << endl; 
					}

					return &((*ptrObj2)->interactionHistory);
				}
			}
		}

	}

	//if not found return NULL
	if(glbl_var.saveFramesToHard)
	{
		behaviourFile << "history lines for objects " << obj1->blob->ID << " and " << obj2->blob->ID  << " NOT found!}"  << endl; 
	}
	return NULL;
}

list<object1twoObjectHistoryPoint*>* BehaviourMatrix::retrieveLastHistoryLine(TrackerObject* obj1, TrackerObject* obj2)
{
	if(glbl_var.saveFramesToHard)
	{
		behaviourFile << "{fetching last history line for objects " << obj1->blob->ID << " and " << obj2->blob->ID  << endl; 
	}

	list<object1InteractionHistory*>::iterator ptrObj1;
	list<object2InteractionHistory*>::iterator ptrObj2;

	TrackerObject* lower_ID;
	TrackerObject* higher_ID;
	//determine the smallest
	if(obj1->blob->ID < obj2->blob->ID)
	{
		lower_ID = obj1;
		higher_ID = obj2;
	}
	else
	{
		lower_ID = obj2;
		higher_ID = obj1;
	}

	for (ptrObj1= object1list.begin(); ptrObj1 != object1list.end(); ptrObj1++)
	{
		if ((*ptrObj1)->obj1 == lower_ID)
		{
			for (ptrObj2= (*ptrObj1)->object2list.begin(); ptrObj2 != (*ptrObj1)->object2list.end(); ptrObj2++)
			{
				if((*ptrObj2)->obj2 == higher_ID)
				{
					if(glbl_var.saveFramesToHard)
					{
						behaviourFile << "last history line for objects " << (*ptrObj1)->obj1->blob->ID << " and " << (*ptrObj2)->obj2->blob->ID  << "FOUND!}"  << endl; 
					}

					return (*ptrObj2)->lastHistoryMeasurement;
				}
			}
		}

	}

	//if not found return NULL
	if(glbl_var.saveFramesToHard)
	{
		behaviourFile << "last history line for objects " << (*ptrObj1)->obj1->blob->ID << " and " << (*ptrObj2)->obj2->blob->ID  << " NOT found!}"  << endl; 
	}
	return NULL;
}

list<object1twoObjectHistoryPoint*>* BehaviourMatrix::retrievecurrentHistoryLine(TrackerObject* obj1, TrackerObject* obj2)
{

	if(glbl_var.saveFramesToHard)
	{
		behaviourFile << "{fetching current history line for objects " << obj1->blob->ID << " and " << obj2->blob->ID  << endl; 
	}

	list<object1InteractionHistory*>::iterator ptrObj1;
	list<object2InteractionHistory*>::iterator ptrObj2;

	TrackerObject* lower_ID;
	TrackerObject* higher_ID;
	//determine the smallest
	if(obj1->blob->ID < obj2->blob->ID)
	{
		lower_ID = obj1;
		higher_ID = obj2;
	}
	else
	{
		lower_ID = obj2;
		higher_ID = obj1;
	}

	for (ptrObj1= object1list.begin(); ptrObj1 != object1list.end(); ptrObj1++)
	{
		if ((*ptrObj1)->obj1 == lower_ID)
		{
			for (ptrObj2= (*ptrObj1)->object2list.begin(); ptrObj2 != (*ptrObj1)->object2list.end(); ptrObj2++)
			{
				if((*ptrObj2)->obj2 == higher_ID)
				{
					if(glbl_var.saveFramesToHard)
					{
						behaviourFile << "current history line for objects " << (*ptrObj1)->obj1->blob->ID << " and " << (*ptrObj2)->obj2->blob->ID  << "FOUND!}"  << endl; 
					}

					return (*ptrObj2)->currentHistoryMeasurement;
				}
			}
		}

	}

	//if not found return NULL
	if(glbl_var.saveFramesToHard)
	{
		behaviourFile << "current history line for objects " << obj1->blob->ID << " and " << obj2->blob->ID  << " NOT found!}"  << endl; 
	}
	return NULL;
}


double calcAlignment(motionPossibility* mp1, motionPossibility* mp2)
{
	double x1x2 = mp1->xDirection*mp2->xDirection; 
	double y1y2 = mp1->yDirection*mp2->yDirection; 

	return  x1x2 + y1y2  ;
}

//double calcDistance(TrackerObject* obj1, TrackerObject* obj2)
double calcDistance(historyPoint* hp1, historyPoint* hp2) 
{

	double x2 = hp1->xBase - hp2->xBase; x2 *= x2;
	double y2 = hp1->yBase - hp2->yBase; y2 *= y2;

	return cvSqrt( x2 + y2  );

}



double calcDistanceChange(twoHistoryPointsInteraction* newHp, twoHistoryPointsInteraction* oldHp)
{
	// d(t-w) - d(t)
	return newHp->distance - oldHp->distance; 

}

double calcVelocityDifference(motionPossibility* mp1, motionPossibility* mp2)
{
	return abs(mp1->speed - mp2->speed);
}

void BehaviourMatrix::updateMatrix()
{
	if(glbl_var.saveFramesToHard)
	{
		behaviourFile << "{"; 
	}




	//we assume it is only called every TIME_MEASUREMENT_STEP
	lastUpdateTime = glbl_var.sampledAbsoluteTime;

	bool found, found2;

	list<object1InteractionHistory*>::iterator ptrObj1;
	list<object2InteractionHistory*>::iterator ptrObj2;
	list<TrackerObject*>::iterator ptrObj_1;
	list<TrackerObject*>::iterator ptrObj_2;

	//smaller IDs are always first
	for (ptrObj_1= glbl_var.BGmodel->trackerBlobs.begin(); ptrObj_1 != glbl_var.BGmodel->trackerBlobs.end(); ptrObj_1++)
	{
		//only visible or occluded
		if (!(*ptrObj_1)->simpleIsOrParentVisible())
			continue;

		//check if object already exists
		found = false;
		for (ptrObj1= object1list.begin(); ptrObj1 != object1list.end(); ptrObj1++)
		{
			//skip deactivated object
			//if(!(*ptrObj1)->active)  continue;

			if((*ptrObj_1) == (*ptrObj1)->obj1) 
			{
				found = true;
				break;
			}
		}
		//if not already found, create it
		if (!found) 
		{
			createObject(*ptrObj_1);
			ptrObj1 = object1list.begin();

			if(glbl_var.saveFramesToHard)
			{
				behaviourFile << "Object " << (*ptrObj1)->obj1->blob->ID << " added as Object1 " << endl; 
			}
		}

		ptrObj_2 = ptrObj_1;
		ptrObj_2++;
		while ( ptrObj_2 != glbl_var.BGmodel->trackerBlobs.end())
		{
			//only visible or occluded
			if (!(*ptrObj_2)->simpleIsOrParentVisible())
			{
				ptrObj_2++;
				continue;
			}

			//skip the object itself
			//if((*ptrObj_2)->blob->ID == (*ptrObj_1)->blob->ID) continue;

			//check if object already exists
			found2 = false;
			for (ptrObj2= (*ptrObj1)->object2list.begin(); ptrObj2 != (*ptrObj1)->object2list.end(); ptrObj2++)
			{

				if((*ptrObj_2) == (*ptrObj2)->obj2) 
				{
					found2 = true;
					break;
				}

			}
			if (!found2) //if the object already exists in the behaviour matrix
			{
				(*ptrObj1)->insertNewObject2(*ptrObj_2);
				ptrObj2 = (*ptrObj1)->object2list.begin();

				if(glbl_var.saveFramesToHard)
				{
					behaviourFile << "Object " << (*ptrObj2)->obj2->blob->ID << " added as Object2" << endl; 
				}
			}

			//create the object history
			//if (found&&found2) //if the object interaction has been established before
			//{

			//}
			//else
			//{
			list<object1twoObjectHistoryPoint*>* newHistory = new list<object1twoObjectHistoryPoint*>;
			(*ptrObj2)->currentHistoryMeasurement = newHistory;
			(*ptrObj2)->interactionHistory.push_front(newHistory);
			/*}*/

			if(glbl_var.saveFramesToHard)
			{
				behaviourFile << "Creating new History moment for Object " << (*ptrObj1)->obj1->blob->ID << " and " << (*ptrObj2)->obj2->blob->ID << endl; 
			}

			(*ptrObj2)->updateTwoObjectInteraction( *ptrObj_1,  *ptrObj_2);


			//insert history point at beginning to preserve chronological order
			//(*ptrObj2)->objectsInteraction.push_front(newPoint);


			/*if (!(*ptrObj2)->lastHistoryMeasurement || (*ptrObj2)->newMeasurement)
			{*/
			//assuming that function is only called every TIME_MEASURMENT_STEP
			(*ptrObj2)->lastHistoryMeasurement = (*ptrObj2)->currentHistoryMeasurement;
			(*ptrObj2)->newMeasurement = false;
			//}

			ptrObj_2++;

		}

	}

	if (glbl_var.saveFramesToHard)
	{
		behaviourFile << "}"; 
	}

}



//void BehaviourMatrix::deleteObject(TrackerObject* obj)
//{
//	list<object1InteractionHistory*>::iterator ptrObj1;
//	for (ptrObj1= object1list.begin(); ptrObj1 != object1list.end(); ptrObj1++)
//	{
//		if((*ptrObj1)->obj1->blob->ID == obj->blob->ID)
//		{
//			//(i) just deactivate
//			//should be faster than going through the trouble of deleting 
//			(*ptrObj1)->active = false;
//
//
//
//			//(ii) or actually delete
//			//will be implemented if needed
//
//
//			return;
//		}
//	}
//}

void BehaviourMatrix::createObject(TrackerObject* obj)
{
	object1InteractionHistory* newObject = new object1InteractionHistory();
	newObject->active = true;
	newObject->obj1= obj;

	object1list.push_front(newObject); //insert at the top because most recent objects are more likely to be accessed

	if(glbl_var.saveFramesToHard)
	{
		behaviourFile << "{Creating new object " << newObject->obj1->blob->ID  << "}" << endl; 
	}

}


bool BehaviourMatrix::isFarFrom(TrackerObject* obj1, TrackerObject* obj2, float threshold, list<twoHistoryPointsInteraction*>* correspondingPointsInteraction)
{
	if(glbl_var.saveFramesToHard)
	{
		behaviourFile << "{Checking \"isFarFrom\" for objects " << obj1->blob->ID << " and " << obj2->blob->ID  << endl; 
	}


	list<object1twoObjectHistoryPoint*>* current_hl = retrievecurrentHistoryLine(obj1,obj2);
	if(!current_hl) 
	{
		if(glbl_var.saveFramesToHard)
		{
			behaviourFile << "No current history line found}"  << endl; 

		}
		return false;
	}

	list<object1twoObjectHistoryPoint*>::iterator obj1_hp_itr;
	list<twoHistoryPointsInteraction*>::iterator obj2_hp_itr;
	
	bool result = false;

	for(obj1_hp_itr = current_hl->begin() ; obj1_hp_itr != current_hl->end() ; obj1_hp_itr++)
	{
		for(obj2_hp_itr = (*obj1_hp_itr)->historyPointsObj2.begin() ; obj2_hp_itr != (*obj1_hp_itr)->historyPointsObj2.end() ; obj2_hp_itr++)
		{
			if((*obj2_hp_itr)->distance > threshold )
			{
				result = true; 
				correspondingPointsInteraction->push_front(*obj2_hp_itr);
			}
		}
	}

	//otherwise they are far from each other
	//otherwise they are far from each other
	if(glbl_var.saveFramesToHard && glbl_var.saveBehaviourFile)
	{
		behaviourFile << "objects " << obj1->blob->ID  << " and " << obj2->blob->ID << " are " << ( result ? "far" : "close") ; 
		behaviourFile << "at distances  = ";
		for(obj2_hp_itr = correspondingPointsInteraction->begin(); obj2_hp_itr != correspondingPointsInteraction->end() ; obj2_hp_itr++)
		{
			behaviourFile <<(*obj2_hp_itr)->distance << " , ";
		}
		behaviourFile << "}" <<endl;
	}

	return result;
	
}


bool BehaviourMatrix::isCloseTo(TrackerObject* obj1, TrackerObject* obj2, float threshold, list<twoHistoryPointsInteraction*>* correspondingPointsInteraction)
{
	if(glbl_var.saveFramesToHard)
	{
		behaviourFile << "{Checking \"isCloseTo\" for objects " << obj1->blob->ID << " and " << obj2->blob->ID  << endl; 
	}


	list<object1twoObjectHistoryPoint*>* current_hl = retrievecurrentHistoryLine(obj1,obj2);
	if(!current_hl) 
	{
		if(glbl_var.saveFramesToHard)
		{
			behaviourFile << "No current history line found}"  << endl; 

		}
		return false;
	}

	list<object1twoObjectHistoryPoint*>::iterator obj1_hp_itr;
	list<twoHistoryPointsInteraction*>::iterator obj2_hp_itr;
	
	bool result = false;

	for(obj1_hp_itr = current_hl->begin() ; obj1_hp_itr != current_hl->end() ; obj1_hp_itr++)
	{
		for(obj2_hp_itr = (*obj1_hp_itr)->historyPointsObj2.begin() ; obj2_hp_itr != (*obj1_hp_itr)->historyPointsObj2.end() ; obj2_hp_itr++)
		{
			if((*obj2_hp_itr)->distance <= threshold )
			{
				result = true; 
				correspondingPointsInteraction->push_front(*obj2_hp_itr);
			}
		}
	}

	//otherwise they are far from each other
	if(glbl_var.saveFramesToHard && glbl_var.saveBehaviourFile)
	{
		behaviourFile << "objects " << obj1->blob->ID  << " and " << obj2->blob->ID << " are " << ( result ? "close" : "far") ; 
		behaviourFile << "at distances  = ";
		for(obj2_hp_itr = correspondingPointsInteraction->begin(); obj2_hp_itr != correspondingPointsInteraction->end() ; obj2_hp_itr++)
		{
			behaviourFile <<(*obj2_hp_itr)->distance << " , ";
		}
		behaviourFile << "}" <<endl;
	}

	return result;

}

bool BehaviourMatrix::hasTaken(TrackerObject* obj1, TrackerObject* obj2, list<historyPoint*> &obj1_corresponding_hps, list<historyPoint*> &obj2_corresponding_hps) //
{
	/*list<historyPoint*>::iterator hp1_itr;
	list<historyPoint*>::iterator hp2_itr;*/
	list<object1twoObjectHistoryPoint*>::iterator hp1_itr;
	list<twoHistoryPointsInteraction*>::iterator hp2_itr;

	bool result = false;

	list<object1twoObjectHistoryPoint*>* hl = retrievecurrentHistoryLine(obj1,obj2);
	if(!hl) 
	{
		if(glbl_var.saveFramesToHard)
		{
			behaviourFile << "No current history line found}"  << endl; 

		}

		return false;
	}

	/*for(hp1_itr = obj1->currentHistoryMeasurement->begin() ; hp1_itr != obj1->currentHistoryMeasurement->end() ; hp1_itr++)*/
	for(hp1_itr = hl->begin() ; hp1_itr != hl->end() ; hp1_itr++)
	{
		/*for(hp2_itr = obj2->currentHistoryMeasurement->begin() ; hp2_itr != obj2->currentHistoryMeasurement->end() ; hp2_itr++)*/
		for(hp2_itr = (*hp1_itr)->historyPointsObj2.begin() ; hp2_itr != (*hp1_itr)->historyPointsObj2.end() ; hp2_itr++)
		{
			/*bool result = isCarrying( (*hp1_itr),  (*hp2_itr), obj1, obj2);*/

			//two conditions: either takes, or drags
			bool condition1 = (*hp2_itr)->takes;
			bool condition2 = false;
			list<Object1twoObjectPossibility*>::iterator itr_mp1;
			list<twoPossibilitiesInteraction*>::iterator itr_mp2;
			for(itr_mp1 = (*hp2_itr)->possibilitiesObj1.begin() ; itr_mp1 != (*hp2_itr)->possibilitiesObj1.end() ; itr_mp1++)
			{
				for(itr_mp2 = (*itr_mp1)->possibilitiesObj2.begin() ; itr_mp2 != (*itr_mp1)->possibilitiesObj2.end() ; itr_mp2++)
				{
					if((*itr_mp2)->drags)
					{
						condition2 = true;
						continue;
					}
				}
				if (condition2) continue;
			}

			if(condition1 || condition2)
			{
				result = true;
				obj1_corresponding_hps.push_front((*hp1_itr)->hp1);
				obj2_corresponding_hps.push_front((*hp2_itr)->hp2);
			}
		}
	}

	if(glbl_var.saveFramesToHard)
	{
		if (result)
			behaviourFile << "objects " << obj1->blob->ID << " and " << obj2->blob->ID << " have a HAS TAKEN relation}"  << endl; 
		else
			behaviourFile << "objects " << obj1->blob->ID << " and " << obj2->blob->ID << " DON'T have a has taken relation}"  << endl;


	}

	return result;



}

bool  BehaviourMatrix::isCarrying(historyPoint* hp1, historyPoint* hp2, TrackerObject* obj1, TrackerObject* obj2) 
{

	if(glbl_var.saveFramesToHard)
	{
		behaviourFile << "{Checking \"isCarrying\" between " << obj1->blob->ID << " and " << obj2->blob->ID  << endl; 
	}

	
	TrackerObject* person = ( obj1->object_classification == OBJECT ? obj2 : obj1);
	TrackerObject* object = ( obj1->object_classification == OBJECT ? obj1 : obj2);
	historyPoint* person_hp = ( obj1->object_classification == OBJECT ? hp2 : hp1);
	historyPoint* object_hp = ( obj1->object_classification == OBJECT ? hp1 : hp2);

	if(person->object_classification == OBJECT || person->object_classification == UNKNOWN )  //first is a person
	{
		if(glbl_var.saveFramesToHard)
		{
			behaviourFile << "FALSE: object1 ID " << person->blob->ID <<  " is not a person}"  << endl; 
		}

		return false; 
	}
	if(object->object_classification != OBJECT || object->object_classification == UNKNOWN )  //second is not a person
	{
		if(glbl_var.saveFramesToHard)
		{
			behaviourFile << "FALSE: object2 ID " << object->blob->ID <<  " is not an object}"  << endl; 
		}

		return false;
	}
	if(!areMerged(person_hp,object_hp)) //they are not merged
	{
		if(glbl_var.saveFramesToHard)
		{
			behaviourFile << "FALSE: objects are not merged at positions" << person_hp->xBase << "," <<person_hp->yBase  << " and " << object_hp->xBase << "," <<object_hp->yBase  << "}"  << endl; 
		}

		return false; 
	}
	if(calcOffsetFromStratingPoint(object_hp,object) < OBJECT_MOVED_THRESHOLD)  //object hasn't moved
	{
		if(glbl_var.saveFramesToHard)
		{
			behaviourFile << "FALSE: the object" << object->blob->ID << " hasn't moved}" << endl; 
		}

		return false; 
	}

	if(glbl_var.saveFramesToHard)
	{
		behaviourFile << "TRUE!}" << endl; 
	}
	return true;


}

bool  isDragging(motionPossibility* mp1, motionPossibility* mp2, historyPoint* hp1, historyPoint* hp2, TrackerObject* obj1, TrackerObject* obj2) //obj1 = person, obj2 = object 
{
	if(glbl_var.saveFramesToHard)
	{
		glbl_var.BehaviourDescriptor.behaviourFile << "{Checking \"isDragging\" between " << obj1->blob->ID << " and " << obj2->blob->ID  << endl; 
	}

	if((obj1->object_classification == OBJECT || obj1->object_classification == UNKNOWN) && (obj2->object_classification == OBJECT || obj2->object_classification == UNKNOWN)) //both are objects
	{
		if(glbl_var.saveFramesToHard)
		{
			glbl_var.BehaviourDescriptor.behaviourFile << "FALSE: both are objects!}" << endl; 
		}

		return false; 
	}
	if((obj1->object_classification != OBJECT || obj1->object_classification == UNKNOWN) && (obj2->object_classification != OBJECT || obj1->object_classification == UNKNOWN)) //both are people
	{
		if(glbl_var.saveFramesToHard)
		{
			glbl_var.BehaviourDescriptor.behaviourFile << "FALSE: both are persons!}" << endl; 
		}

		return false; 
	}

	if(!areMoving(mp1,mp2, hp1, hp2,obj1,obj2)) //they are not moving
	{
		if(glbl_var.saveFramesToHard)
		{
			glbl_var.BehaviourDescriptor.behaviourFile << "FALSE: they are not moving}" << endl; 
		}

		return false; 
	}
	if(calcDistance(hp1,hp2) > glbl_var.BehaviourDescriptor.DRAGGING_DISTANCE_THRESHOLD) //distance
	{
		if(glbl_var.saveFramesToHard)
		{
			glbl_var.BehaviourDescriptor.behaviourFile << "FALSE: their distance = "<< calcDistance(hp1,hp2) << "  is too big}" << endl; 
		}

		return false;
	}

	if(glbl_var.saveFramesToHard)
	{
		glbl_var.BehaviourDescriptor.behaviourFile << "TRUE!}" << endl; 
	}
	return true;


}





BehaviourMatrix::BehaviourMatrix()
{
	W_to_H_FAINTING_RATIO = 2.5;
	DIFFERENT_ANGLES_THRESHOLD = 0.34*2; //20*2 degrees
	SPLIT_MERGE_FREQUENCY= 4; 			
	ALIGNMENT_OR_DIRECTION_CHANGE_THRESHOLD = 1.04; //60

	lastUpdateTime = 0;

	detectAbandonedBasedOnHist = false;

}

BehaviourMatrix::~BehaviourMatrix()
{
	//list<object1InteractionHistory*>::iterator ptrObj1;
	//list<object2InteractionHistory*>::iterator ptrObj2;
	//list<twoPossibilitiesInteraction*>::iterator ptrObj3;


	//for (ptrObj1= object1list.begin(); ptrObj1 != object1list.end(); ptrObj1++)
	//{
	//	for (ptrObj2= (*ptrObj1)->object2list.begin(); ptrObj2 != (*ptrObj1)->object2list.end(); ptrObj2++)
	//	{
	//		for (ptrObj3= (*ptrObj2)->objectsInteraction.begin(); ptrObj3 != (*ptrObj2)->objectsInteraction.end(); ptrObj3++)
	//			delete (*ptrObj3);

	//		(*ptrObj2)->objectsInteraction.clear();
	//	}
	//	(*ptrObj1)->object2list.clear();
	//}
	//object1list.clear();

}
bool areMoving(motionPossibility* mp1, motionPossibility* mp2 ,historyPoint* hp1,historyPoint* hp2,  TrackerObject* obj1, TrackerObject* obj2)
{
	if(glbl_var.saveFramesToHard)
	{
		glbl_var.BehaviourDescriptor.behaviourFile << "{Checking \"areMoving\" between " << obj1->blob->ID << " and " << obj2->blob->ID  << endl; 
	}

	//they have to be people
	//if(obj1->object_classification == OBJECT || obj1->object_classification == UNKNOWN ) return false;
	//if(obj2->object_classification == OBJECT || obj2->object_classification == UNKNOWN ) return false;

	if(areMerged(hp1,hp2) && (     (*(*hp1->relatedParent->lastHistoryMeasurement->begin())->motionPossibilities.begin())->speed > glbl_var.BGmodel->MOTION_TOLERANCE/TIME_MEASUREMENT_STEP ))
	{
		if(glbl_var.saveFramesToHard)
		{
			glbl_var.BehaviourDescriptor.behaviourFile << "TRUE: parent object " << hp1->relatedParent->blob->ID  << " is moving}" << endl; 
		}

		return true; 
	}

	if(mp1->speed < glbl_var.BGmodel->MOTION_TOLERANCE/TIME_MEASUREMENT_STEP) //first mp is not moving
	{
		if(glbl_var.saveFramesToHard)
		{
			glbl_var.BehaviourDescriptor.behaviourFile << "FALSE: object " << obj1->blob->ID  << " is not moving}" << endl; 
		}

		return false; 
	}
	if(mp2->speed < glbl_var.BGmodel->MOTION_TOLERANCE/TIME_MEASUREMENT_STEP)  //second mp is not moving
	{
		if(glbl_var.saveFramesToHard)
		{
			glbl_var.BehaviourDescriptor.behaviourFile << "FALSE: object " << obj2->blob->ID  << " is not moving}" << endl; 
		}

		return false; 
	}

	if(glbl_var.saveFramesToHard)
	{
		glbl_var.BehaviourDescriptor.behaviourFile << "TRUE!}" << endl; 
	}
	return true;
} 

bool BehaviourMatrix::sameDirection(motionPossibility* mp1, motionPossibility* mp2,double&angle_difference)
{

	bool result = true;
	/*if( (mp1->yDirection*mp2->yDirection < 0) ||  (mp2->xDirection*mp1->xDirection < 0) ) 
		result = false;*/

	int PI = 3.1417;
	double obj1angle = ( mp1->xDirection == 0 ? ( mp1->yDirection == 0 ? 0 : PI ) : atan2(mp1->yDirection,mp1->xDirection ));
	double obj2angle = ( mp2->xDirection == 0 ? ( mp2->yDirection == 0 ? 0 : PI ) : atan2(mp2->yDirection,mp2->xDirection ));

	angle_difference = abs(obj1angle -  obj2angle);
	//if(!result) angle_difference = PI -angle_difference;
	if(angle_difference > PI) angle_difference = PI -angle_difference;

	//if(abs(obj1angle -  obj2angle) > DIFFERENT_ANGLES_THRESHOLD) 
	if(angle_difference > DIFFERENT_ANGLES_THRESHOLD) 
	{
		result = false;
	}

	return result;
}

bool BehaviourMatrix::areSameSpeedDirection(motionPossibility* mp1, motionPossibility* mp2,  TrackerObject* obj1, TrackerObject* obj2)
{
	if(glbl_var.saveFramesToHard)
	{
		behaviourFile << "{Checking \"areSameSpeedDirection\" between " << obj1->blob->ID << " and " << obj2->blob->ID  << endl; 
	}
	//they have to be people
	//if(obj1->object_classification == OBJECT || obj1->object_classification == UNKNOWN ) return false;
	//if(obj2->object_classification == OBJECT || obj2->object_classification == UNKNOWN ) return false;


	//speed condition
	if(abs(calcVelocityDifference(mp1,mp2)) > DIFFERENT_SPEEDS_THRESHOLD) 
	{
		if(glbl_var.saveFramesToHard)
		{
			behaviourFile << "FALSE: speed difference =  " << abs(calcVelocityDifference(mp1,mp2))  << " is too big}" << endl; 
		}

		return false;
	}

	////direction condition
	//if( (mp1->xDirection < 0) != (mp2->xDirection < 0) ) 
	//{
	//	if(glbl_var.saveFramesToHard)
	//	{
	//		behaviourFile << "FALSE: different X directions}" << endl; 
	//	}

	//	return false;
	//}
	//if( (mp1->yDirection < 0) != (mp2->yDirection < 0) ) 
	//{
	//	if(glbl_var.saveFramesToHard)
	//	{
	//		behaviourFile << "FALSE: different Y directions}" << endl; 
	//	}

	//	return false;
	//}

	//(i)
	/*if( (mp1->yDirection*mp2->yDirection < 0) ||  (mp2->xDirection*mp1->xDirection < 0) ) 
	{
		if(glbl_var.saveFramesToHard)
		{
			behaviourFile << "FALSE: different directions}" << endl; 
		}

		return false;
	}

	int PI = 3.1417;
	double obj1angle = ( mp1->xDirection == 0 ? ( mp1->yDirection == 0 ? 0 : PI ) : atan(mp1->yDirection/mp1->xDirection ));
	double obj2angle = ( mp2->xDirection == 0 ? ( mp2->yDirection == 0 ? 0 : PI ) : atan(mp2->yDirection/mp2->xDirection ));

	if(abs(obj1angle -  obj2angle) > DIFFERENT_ANGLES_THRESHOLD) 
	{
		if(glbl_var.saveFramesToHard)
		{
			behaviourFile << "FALSE: angle in between = " << abs(obj1angle -  obj2angle)<< " is too big" << endl; 
		}

		return false;
	}*/
	//(ii)

	double angleDiff;
	if(!sameDirection(mp1, mp2, angleDiff)) 
	{
		if(glbl_var.saveFramesToHard)
		{
			behaviourFile << "FALSE: angle in between = " << angleDiff << " is too big" << endl; 
		}

		return false;
	}

	if(glbl_var.saveFramesToHard)
	{
		behaviourFile << "TRUE!}" << endl; 
	}
	return true;
}

//twoPossibilitiesInteraction::twoPossibilitiesInteraction(TrackerObject* obj1, TrackerObject* obj2)
//{
//	alignment = calcAlignment(obj1,obj2);
//	distance = calcDistance(obj1,obj2);
//
//	distance_change = glbl_var.BehaviourDescriptor.calcDistanceChange(obj1,obj2);
//
//	velocity_difference = calcVelocityDifference(obj1,obj2);
//
//	merged = areMerged(obj1,obj2);
//	takes = glbl_var.BehaviourDescriptor.isCarrying(obj1,obj2);
//	sameSpeedDirection = glbl_var.BehaviourDescriptor.areSameSpeedDirection(obj1,obj2);
//	//alignment_or_ParentDirection_Changed = glbl_var.BehaviourDescriptor.checkAlignmentParentDirectionChange(obj1,obj2);
//
//	time = glbl_var.sampledAbsoluteTime;
//
//
//}

//twoPossibilitiesInteraction::twoPossibilitiesInteraction()
//{
//	alignment = 0; 
//	//double distance = 0; 
//	//double distance_change = 0;
//	velocity_difference = 0; 
//
//	//bool merged = false; 
//	//bool takes = false; 
//	//bool alignment_or_ParentDirection_Changed = false; 
//	//isAbandonedBy = false; 
//	sameSpeedDirection = false; 
//	preceding = NULL;
//
//	//relatedParent1 = NULL;
//	//relatedParent2 = NULL;
//
//	//time = 0;
//}

//twoPossibilitiesInteraction::~twoPossibilitiesInteraction()
//{
//	//relatedParent1 = NULL;
//	//relatedParent2 = NULL;
//}

void object1InteractionHistory::insertNewObject2(TrackerObject* obj)
{
	object2InteractionHistory* newPoint = new object2InteractionHistory();
	newPoint->obj2 = obj;
	newPoint->obj1 = obj1;
	newPoint->lastHistoryMeasurement = NULL;
	object2list.push_front(newPoint);

	if(glbl_var.saveFramesToHard)
	{
		glbl_var.BehaviourDescriptor.behaviourFile << "{Inserting new object " << obj->blob->ID  << " to object " << obj1->blob->ID << "}" << endl; 
	}
}


void object2InteractionHistory::updateTwoObjectInteraction(TrackerObject* obj1 , TrackerObject* obj2)
{


	list<historyPoint*>::iterator hpObj1;
	list<historyPoint*>::iterator hpObj2;


	//for(hpObj1 = obj1->currentHistoryMeasurement->begin() ; hpObj1 != obj1->currentHistoryMeasurement->end() ; hpObj1++)
	for(hpObj1 = obj1->lastHistoryMeasurement->begin() ; hpObj1 != obj1->lastHistoryMeasurement->end() ; hpObj1++)
	{
		object1twoObjectHistoryPoint* newPoint1 = new object1twoObjectHistoryPoint();
		newPoint1->hp1 = (*hpObj1);
		currentHistoryMeasurement->push_front(newPoint1);

		if(glbl_var.saveFramesToHard)
			glbl_var.BehaviourDescriptor.behaviourFile << "  new history point of object " << obj1->blob->ID  << " related to parent " << (*hpObj1)->relatedParent->blob->ID  <<endl ;


		//for(hpObj2 = obj2->currentHistoryMeasurement->begin() ; hpObj2 != obj2->currentHistoryMeasurement->end() ; hpObj2++)
		for(hpObj2 = obj2->lastHistoryMeasurement->begin() ; hpObj2 != obj2->lastHistoryMeasurement->end() ; hpObj2++)
		{
			if(glbl_var.saveFramesToHard)
				glbl_var.BehaviourDescriptor.behaviourFile << "   new sub history point of object " << obj2->blob->ID  << " related to parent " << (*hpObj2)->relatedParent->blob->ID  <<endl ;


			twoHistoryPointsInteraction* newPoint2 = new twoHistoryPointsInteraction();
			newPoint2->hp2 = (*hpObj2);
			newPoint2->hp1 = (*hpObj1);

			newPoint1->historyPointsObj2.push_front(newPoint2);
			newPoint2->updateTwoHistoryPointsInteraction(*hpObj1,*hpObj2,currentHistoryMeasurement,obj1 ,obj2 );
		}
	}

}

void twoHistoryPointsInteraction::updateTwoHistoryPointsInteraction(historyPoint* hp1 , historyPoint* hp2, list<object1twoObjectHistoryPoint*>* lastHistory, TrackerObject* obj1, TrackerObject* obj2)
{

	distance = calcDistance(hp1,hp2); // distance between positions of two objects

	if(glbl_var.saveFramesToHard)
		glbl_var.BehaviourDescriptor.behaviourFile << "    distance between " << hp1->xBase << "," << hp1->yBase << " and " << hp2->xBase << "," << hp2->yBase << " = " << distance <<endl ;

	merged = areMerged(hp1,hp2); //used for fighting, samespeeddirection

	if(glbl_var.saveFramesToHard)
		glbl_var.BehaviourDescriptor.behaviourFile << "    object " << obj1->blob->ID << ( merged ? "IS" : "is NOT") << " merges with object " <<  obj2->blob->ID <<endl ;

	takes = glbl_var.BehaviourDescriptor.isCarrying(hp1,hp2, obj1, obj2)  ; // used for theft

	if(glbl_var.saveFramesToHard)                
		glbl_var.BehaviourDescriptor.behaviourFile << "    object " << obj1->blob->ID << ( takes ? "HAS" : "DOESN'T have") << " object " <<  obj2->blob->ID <<endl ;


	time = glbl_var.sampledAbsoluteTime;
	frameNumber = glbl_var.BGmodel->get_t();
	//relatedParent1 = hp1->relatedParent;
	//relatedParent2 = hp2->relatedParent;




	//possibilities
	list<motionPossibility*>::iterator possibilityIterator1;
	list<motionPossibility*>::iterator possibilityIterator2;
	for(possibilityIterator1 = hp1->motionPossibilities.begin() ; possibilityIterator1 != hp1->motionPossibilities.end(); possibilityIterator1++)
	{
		if(glbl_var.saveFramesToHard)
			glbl_var.BehaviourDescriptor.behaviourFile << "    new possibility with speed " << (*possibilityIterator1)->vxBase << "," << (*possibilityIterator1)->vyBase <<  endl ;


		Object1twoObjectPossibility* newPossibility1 = new Object1twoObjectPossibility();
		newPossibility1->mp1 = (*possibilityIterator1);
		possibilitiesObj1.push_front(newPossibility1);

		for(possibilityIterator2 = hp2->motionPossibilities.begin() ; possibilityIterator2 != hp2->motionPossibilities.end() ; possibilityIterator2++)
		{


			if(glbl_var.saveFramesToHard)
				glbl_var.BehaviourDescriptor.behaviourFile << "    new sub possibility" << (*possibilityIterator2)->vxBase << "," << (*possibilityIterator2)->vyBase << endl ;


			twoPossibilitiesInteraction* newPossibility2 = new twoPossibilitiesInteraction();
			newPossibility2->mp2 = (*possibilityIterator2);
			newPossibility2->mp1 = (*possibilityIterator1);
			newPossibility1->possibilitiesObj2.push_front(newPossibility2);

			newPossibility2->updateTwoPossibilitiesInteraction(*possibilityIterator1,*possibilityIterator2, hp1, hp2, obj1, obj2);
		}
	}

	//time dependent properties
	/*list<Object1twoObjectPossibility*>::iterator iterator1;  
	list<twoPossibilitiesInteraction*>::iterator iterator2;
	for(iterator1 = possibilitiesObj1.begin();iterator1 != possibilitiesObj1.end(); iterator1++)
		for(iterator2 = (*iterator1)->possibilitiesObj2.begin();iterator2 != (*iterator1)->possibilitiesObj2.end(); iterator2++)
		{
			if(glbl_var.saveFramesToHard)
				glbl_var.BehaviourDescriptor.behaviourFile << "    new time dependent information" <<  endl ;

			twoObjecttimeDependentInformation* newTimeDependentInformation = new twoObjecttimeDependentInformation();
			timeDependent.push_front(newTimeDependentInformation);
			if((*iterator2)->preceding)
				newTimeDependentInformation->updateTimeDependentInformation(this,(*iterator2)->preceding);

		}*/


	






}


void twoPossibilitiesInteraction::updateTwoPossibilitiesInteraction(motionPossibility* mp1, motionPossibility* mp2, historyPoint* hp1, historyPoint* hp2, TrackerObject* obj1, TrackerObject* obj2)
{
	alignment = calcAlignment(mp1,mp2); //angle between directions of two objects	
	velocity_difference = calcVelocityDifference(mp1,mp2);; //difference in velocity between two objects

	//these statuses are used to make inferences that need time accumulation

	//bool isAbandonedBy; //used for left luggage
	sameSpeedDirection = glbl_var.BehaviourDescriptor.areSameSpeedDirection(mp1,mp2, obj1, obj2); //used for walking together
	moving = areMoving(mp1 , mp2 , hp1, hp2 ,obj1, obj2);
	drags = isDragging(mp1 , mp2 , hp1, hp2 ,obj1, obj2); //obj1 = person, obj2 = object 


	//twoHistoryPointsInteraction* preceding;
	////assign preceding
	//list<list<object1twoObjectHistoryPoint*>*>* history = glbl_var.BehaviourDescriptor.retrieveHistoryLine(obj1,obj2);
	//list<list<object1twoObjectHistoryPoint*>*>::iterator itr_history;
	//list<object1twoObjectHistoryPoint*>::iterator itr_hp1;
	//list<twoHistoryPointsInteraction*>::iterator itr_hp2;
	//bool found  = false;
	//for(itr_history = ++(history->begin()) ; itr_history != history->end() ; itr_history++)
	//{
	//	for(itr_hp1 = (*itr_history)->begin() ; itr_hp1 != (*itr_history)->end() ; itr_hp1++)
	//	{
	//		//if((*itr_hp1)->hp1 != hp1)
	//		if(mp1->precedingPoint != (*itr_hp1)->hp1)
	//			continue;
	//		for(itr_hp2 = (*itr_hp1)->historyPointsObj2.begin() ; itr_hp2 != (*itr_hp1)->historyPointsObj2.end() ; itr_hp2++)
	//		{
	//			//if((*itr_hp2)->hp2 != hp2)
	//			if(mp2->precedingPoint != (*itr_hp2)->hp2)
	//				continue;

	//			preceding = (*itr_hp2);
	//			found = true;


	//		}
	//		if (found)
	//			break;
	//	}
	//	if (found)
	//		break;
	//}



		if(glbl_var.saveFramesToHard)
		{
			glbl_var.BehaviourDescriptor.behaviourFile << "     alignment between direction " << mp1->xDirection << "," << mp1->yDirection << " and " << mp2->xDirection << "," << mp2->yDirection << " = " << alignment  <<  endl ;
			glbl_var.BehaviourDescriptor.behaviourFile << "     velocity_difference " << mp1->speed << " and " << mp2->speed <<  " = " << velocity_difference  <<  endl ;
			glbl_var.BehaviourDescriptor.behaviourFile << "     "<< (sameSpeedDirection  ? "ARE" : "are NOT")  << " same speed and direction" <<endl ;
			glbl_var.BehaviourDescriptor.behaviourFile << "     preceding is " << (preceding ? preceding->frameNumber : 0)  <<endl;
		}

}


void twoObjecttimeDependentInformation::updateTimeDependentInformation(twoHistoryPointsInteraction* newInteraction, twoHistoryPointsInteraction* oldInteaction)
{
	distance_change = calcDistanceChange(newInteraction,oldInteaction) ;
	//alignment_or_ParentDirection_Changed = ; // used for fighting
	precedingPoint = oldInteaction;

	if(glbl_var.saveFramesToHard)
	{
		glbl_var.BehaviourDescriptor.behaviourFile << "     distance change from distance " << precedingPoint->distance << " = " << distance_change << endl ;
	}

}









bool BehaviourMatrix::loitering(TrackerObject* obj)
{
	if(glbl_var.saveFramesToHard)
	{
		behaviourFile << "{Checking \"loitering\" for object " << obj->blob->ID  << endl; 
	}

	if(obj->object_classification == UNKNOWN || obj->object_classification == OBJECT) //it has to be a person
	{
		if(glbl_var.saveFramesToHard)
		{
			behaviourFile << "FALSE: it is not a person}" << endl; 
		}

		return false; 
	}

	//float time_in_sec = glbl_var.frame_to_sec(glbl_var.BGmodel->get_t() - (obj->time - 1),true);
	float duration = glbl_var.BGmodel->get_t() - (obj->time - 1);
	//if( time_in_sec < LOITERING_THRESHOLD)
	if( duration < LOITERING_THRESHOLD)
	{
		if(glbl_var.saveFramesToHard)
		{
			//behaviourFile << "FALSE: time = " <<time_in_sec << " is less than loitering threshold" << endl; 
			behaviourFile << "FALSE: time = " <<duration << " is less than loitering threshold" << endl;
		}

		return false;
	}

	if(glbl_var.saveFramesToHard)
	{
		behaviourFile << "TRUE!}" << endl; 
	}
	return true;
}

bool twoHistoryPointsInteraction::checkPairPrecedingAbandoned()
{


	if (!isAbandonedBy) return false;
	if (hasBeenAbandonedBy) return true;

	//(i) using preceding
	list<Object1twoObjectPossibility*>::iterator mp1;
	list<twoPossibilitiesInteraction*>::iterator mp2;
	historyPoint* preceding1;
	historyPoint* preceding2;
	bool found = false;
	for(mp1 = possibilitiesObj1.begin() ; mp1 != possibilitiesObj1.end()  ; mp1++)
	{
		for(mp2 = (*mp1)->possibilitiesObj2.begin() ; mp2 != (*mp1)->possibilitiesObj2.end() ; mp2++)
		{
			if(!(*mp2)->preceding)
			{
				continue;
			}

			//if ( (glbl_var.sampledAbsoluteTime - (*mp2)->preceding->frameNumber) > glbl_var.BehaviourDescriptor.ABANDONED_LUGGAGE_DURATION_THRESHOLD     ) 
			if ( (glbl_var.BGmodel->get_t() - (*mp2)->preceding->frameNumber) > glbl_var.BehaviourDescriptor.ABANDONED_LUGGAGE_DURATION_THRESHOLD     ) 
			{
				return true;
				//continue; //if the preceding doesn't exist of its time is beyond the interval of interest, the it is successful
			}

			if ((*mp2)->preceding->checkPairPrecedingAbandoned())
			{
				return true;			
				//found  = true;
			}

		}
	}





	//return found;
	return false;
}

int twoHistoryPointsInteraction::checkPairPrecedingMergeChanges() //returns the number of times merging has finished
{

	list<Object1twoObjectPossibility*>::iterator mp1;
	list<twoPossibilitiesInteraction*>::iterator mp2;
	historyPoint* preceding1;
	historyPoint* preceding2;

	//bool found = false;
	int highest_num_of_changes = 0;
	int num_of_changes = 0;
	for(mp1 = possibilitiesObj1.begin() ; mp1 != possibilitiesObj1.end()  ; mp1++)
	{
		for(mp2 = (*mp1)->possibilitiesObj2.begin() ; mp2 != (*mp1)->possibilitiesObj2.end() ; mp2++)
		{

			//if ( (!(*mp2)->preceding)    ||    ((glbl_var.sampledAbsoluteTime - (*mp2)->preceding->frameNumber) > glbl_var.BehaviourDescriptor.FIGHTING_TIME_SPAN )     ) 
			if ( (!(*mp2)->preceding)    ||    ((glbl_var.BGmodel->get_t() - (*mp2)->preceding->frameNumber) > glbl_var.BehaviourDescriptor.FIGHTING_TIME_SPAN )     ) 
			{
				num_of_changes =  0;
				//continue; //if the preceding doesn't exist of its time is beyond the interval of interest, the it is successful
			}
			else
			{
				num_of_changes = (*mp2)->preceding->checkPairPrecedingMergeChanges();
				if((*mp2)->preceding->merged != merged)
					num_of_changes++;
			}

			if(highest_num_of_changes < num_of_changes) 
				highest_num_of_changes = num_of_changes;
		}
	}

	//return found;
	return highest_num_of_changes;
}


bool twoHistoryPointsInteraction::checkPairPrecedingClose()
{
	list<Object1twoObjectPossibility*>::iterator mp1;
	list<twoPossibilitiesInteraction*>::iterator mp2;
	historyPoint* preceding1;
	historyPoint* preceding2;

	if (!meeting) return false;
	if (hasBeenMeeting) return true;

	//bool found = false;
	for(mp1 = possibilitiesObj1.begin() ; mp1 != possibilitiesObj1.end()  ; mp1++)
	{
		for(mp2 = (*mp1)->possibilitiesObj2.begin() ; mp2 != (*mp1)->possibilitiesObj2.end() ; mp2++)
		{
			if(!(*mp2)->preceding)
			{
				//return false;
				continue;
			}

			//if (     (glbl_var.sampledAbsoluteTime - (*mp2)->preceding->frameNumber) > glbl_var.BehaviourDescriptor.MEETING_DURATION_THRESHOLD      ) 
			if (     (glbl_var.BGmodel->get_t() - (*mp2)->preceding->frameNumber) > glbl_var.BehaviourDescriptor.MEETING_DURATION_THRESHOLD      ) 
			{
				return true;
				//continue; //if the preceding doesn't exist of its time is beyond the interval of interest, the it is successful
			}

			if ((*mp2)->preceding->checkPairPrecedingClose())
			{
				return true;
				//found  = true;
			}

		}
	}

	//return found;
	return false;
}


void  BehaviourMatrix::detectBehaviours()
{
	//if(log_text) 
	//{
	//	delete[] log_text;
	//}
	//log_text = new char[1000];
	//strcpy(log_text,"");

	//empty all global ID lists
	glbl_var.abandonedLuggaeID.clear();
	glbl_var.theftLuggageID.clear();
	glbl_var.meetID.clear();
	glbl_var.walkTogetherID.clear();
	glbl_var.fightID.clear();
	glbl_var.faintID.clear();
	glbl_var.loiterID.clear();

	//single object events
	list<TrackerObject*>::iterator itr_obj;
	list<twoHistoryPointsInteraction*> temp_objects_list;
	list<twoHistoryPointsInteraction*>::iterator result_list_itr;
	char buffer[100];
	char log_text[10000];
	strcpy(log_text,"");
	int x = 10*glbl_var.resize_factor;
	int y = 10*glbl_var.resize_factor;
	CvFont font;
	cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.5, 0.5, 0, 2, CV_AA);
	bool abandoned_temp;
	bool loiter_temp;
	bool faint_temp;
	bool stolen_temp;
	bool fighting_temp;
	bool meeting_temp;
	bool walking_temp;
	CvScalar useColor = cvScalar(0,0,255);
	CvScalar useColor2 = cvScalar(255,255,255);
	//CvScalar useColor = cvScalar(255,0,0);
	list<TrackerObject*> abandoned_objects;
	for(itr_obj = glbl_var.BGmodel->trackerBlobs.begin() ; itr_obj != glbl_var.BGmodel->trackerBlobs.end()  ; itr_obj++)
	{
		//only visible or occluded
		if (!(*itr_obj)->simpleIsOrParentVisible())
				continue;



		//adandoned luggage
		abandoned_temp = abandonedObject((*itr_obj), &temp_objects_list ); 
		if(abandoned_temp)
		{
			abandoned_objects.push_front(*itr_obj);
		}

		//loitering
		temp_objects_list.clear();
		loiter_temp = loitering(*itr_obj); 
		if(loiter_temp)
		{
			strcat(log_text,"Person ");
			strcat(log_text,itoa((*itr_obj)->blob->ID,buffer,10));
			strcat(log_text," is LOITERING ");

			if(glbl_var.debugScreen)
				cvPutText(glbl_var.displayedFrame,log_text, cvPoint(x,y), &font,useColor);
			(*itr_obj)->loiter = true;
			

			strcat(log_text,"\r\n\r\n");
			strcpy(glbl_var.behaviour_log,log_text);
			strcpy(log_text,"");
			y += 10*glbl_var.resize_factor;

			//add ID for purposes of CPUfile
			glbl_var.loiterID.insert(glbl_var.loiterID.begin(),(*itr_obj)->blob->ID);

		}


		//has fainted
		faint_temp = hasFainted(*itr_obj); 
		if(faint_temp)
		{
			strcat(log_text,"Person ");
			strcat(log_text,itoa((*itr_obj)->blob->ID,buffer,10));
			strcat(log_text," has FAINTED ");

			if(glbl_var.debugScreen)
				cvPutText(glbl_var.displayedFrame,log_text, cvPoint(x,y), &font,useColor);
			(*itr_obj)->faint = true;

			strcat(log_text,"\r\n\r\n");
			strcpy(glbl_var.behaviour_log,log_text);
			strcpy(log_text,"");
			y += 10*glbl_var.resize_factor;

			//add ID for purposes of CPUfile
			glbl_var.faintID.insert(glbl_var.faintID.begin(),(*itr_obj)->blob->ID);

		}
	}



	//two objects interaction
	list<TrackerObject*> stolen_objects;
	list<TrackerObject*> fighting_objects1;
	list<TrackerObject*> walking_objects1;
	list<TrackerObject*> meeting_objects1;
	list<TrackerObject*> fighting_objects2;
	list<TrackerObject*> walking_objects2;
	list<TrackerObject*> meeting_objects2;
	list<TrackerObject*>::iterator itrObj1;
	list<TrackerObject*>::iterator itrObj2;
	for(itrObj1 = glbl_var.BGmodel->trackerBlobs.begin() ; itrObj1 != glbl_var.BGmodel->trackerBlobs.end() ;itrObj1++)
	{
		//only visible or occluded
		if (!(*itrObj1)->simpleIsOrParentVisible())
				continue;


		itrObj2 = itrObj1;
		itrObj2++;
		while(itrObj2 != glbl_var.BGmodel->trackerBlobs.end() )
		{
			//only visible or occluded
			if (!(*itrObj2)->simpleIsOrParentVisible())
			{
				itrObj2++;
				continue;
			}

			//stolen luggage
			temp_objects_list.clear();
			stolen_temp = stoleLuggage(*itrObj1 , *itrObj2 , &temp_objects_list, &abandoned_objects);
			if(stolen_temp)
			{
				stolen_objects.push_front((*itrObj1)->object_classification == OBJECT ? (*itrObj1) : (*itrObj2));

				strcat(log_text,"Object ");
				strcat(log_text,itoa(  ((*itrObj1)->object_classification == OBJECT ? (*itrObj1)->blob->ID : (*itrObj2)->blob->ID),buffer,10) );
				strcat(log_text," is STOLEN by person ");
				strcat(  log_text  ,  itoa(  ((*itrObj1)->object_classification == OBJECT ? (*itrObj2)->blob->ID : (*itrObj1)->blob->ID)  ,buffer,10   )  );
				/*strcat(log_text, " at possible points: ");
				for(result_list_itr = temp_objects_list.begin(); result_list_itr != temp_objects_list.end() ; result_list_itr++ )
				{
					strcat(log_text, "(" );
					sprintf(buffer,"%.2f",(*result_list_itr)->hp1->xBase);
					strcat(log_text,  buffer);
					strcat(log_text, "," );
					sprintf(buffer,"%.2f",(*result_list_itr)->hp1->yBase);
					strcat(log_text,  buffer);
					strcat(log_text, ")-(" );
					sprintf(buffer,"%.2f",(*result_list_itr)->hp2->xBase);
					strcat(log_text,  buffer);
					strcat(log_text, "," );
					sprintf(buffer,"%.2f",(*result_list_itr)->hp2->yBase);
					strcat(log_text, buffer);
					strcat(log_text, ")\t");
					}*/

				if(glbl_var.debugScreen)
					cvPutText(glbl_var.displayedFrame,log_text, cvPoint(x,y), &font,useColor);
				((*itrObj1)->object_classification == OBJECT ? (*itrObj2) : (*itrObj1))->steal = true;

				strcat(log_text,"\r\n\r\n");
				strcpy(glbl_var.behaviour_log,log_text);
				strcpy(log_text,"");
				y += 10*glbl_var.resize_factor;

				//add ID for purposes of CPUfile
				glbl_var.theftLuggageID.insert(glbl_var.theftLuggageID.begin(),(*itrObj1)->object_classification == OBJECT ? (*itrObj1)->blob->ID : (*itrObj2)->blob->ID);


			}

			//fighting
			temp_objects_list.clear();
			fighting_temp = areFighting(*itrObj1 , *itrObj2 , temp_objects_list);
			if(fighting_temp)
			{
				fighting_objects1.push_front(*itrObj1 );
				fighting_objects2.push_front(*itrObj2 );

				strcat(log_text,"Person ");
				strcat(log_text,itoa((*itrObj1)->blob->ID,buffer,10)  );
				strcat(log_text," is FIGHTING with person ");
				strcat(log_text,itoa((*itrObj2)->blob->ID,buffer,10) );
				/*strcat(log_text, " at possible points: ");
				for(result_list_itr = temp_objects_list.begin(); result_list_itr != temp_objects_list.end() ; result_list_itr++ )
				{
				strcat(log_text, "(" );
				sprintf(buffer,"%.2f",(*result_list_itr)->hp1->xBase);
				strcat(log_text,  buffer);
				strcat(log_text, "," );
				sprintf(buffer,"%.2f",(*result_list_itr)->hp1->yBase);
				strcat(log_text,  buffer);
				strcat(log_text, ")-(" );
				sprintf(buffer,"%.2f",(*result_list_itr)->hp2->xBase);
				strcat(log_text,  buffer);
				strcat(log_text, "," );
				sprintf(buffer,"%.2f",(*result_list_itr)->hp2->yBase);
				strcat(log_text, buffer);
				strcat(log_text, ")\t");
				}*/

				if(glbl_var.debugScreen)
					cvPutText(glbl_var.displayedFrame,log_text, cvPoint(x,y), &font,useColor);
				(*itrObj1)->fight = true;
				(*itrObj2)->fight = true;

				strcat(log_text,"\r\n\r\n");
				strcpy(glbl_var.behaviour_log,log_text);
				strcpy(log_text,"");
				y += 10*glbl_var.resize_factor;

				//add ID for purposes of CPUfile
				glbl_var.fightID.insert(glbl_var.fightID.begin(), (*itrObj1)->blob->ID );
				glbl_var.fightID.insert(glbl_var.fightID.begin(), (*itrObj2)->blob->ID );


			}

			//meeting
			temp_objects_list.clear();
			meeting_temp = meeting(*itrObj1 , *itrObj2 , &temp_objects_list);
			if(meeting_temp)
			{
				meeting_objects1.push_front(*itrObj1);
				meeting_objects2.push_front(*itrObj2);




			}

			//walking together
			temp_objects_list.clear();
			walking_temp = walkingTogether(*itrObj1 , *itrObj2 , &temp_objects_list);
			if(walking_temp)
			{
				walking_objects1.push_front(*itrObj1);
				walking_objects2.push_front(*itrObj2);


			}

			itrObj2++;
		}
	}

	//now check if you want abandoned
	for(itr_obj = abandoned_objects.begin() ; itr_obj != abandoned_objects.end()  ; itr_obj++)
	{
		if(find(stolen_objects.begin() ,stolen_objects.end(),(*itr_obj)) == stolen_objects.end()) //if not found, print it
		{
			strcat(log_text,"Object ");
			strcat(log_text,itoa((*itr_obj)->blob->ID,buffer,10));
			strcat(log_text," is ABANDONED by person ");
			strcat(log_text,itoa(( (*itr_obj)->belongs_to ? (*itr_obj)->belongs_to->blob->ID: -1),buffer,10));
			/*strcat(log_text, " at possible points: ");
			for(result_list_itr = temp_objects_list.begin(); result_list_itr != temp_objects_list.end() ; result_list_itr++ )
			{
			strcat(log_text, "(" );
			sprintf(buffer,"%.2f",(*result_list_itr)->hp1->xBase);
			strcat(log_text,  buffer);
			strcat(log_text, "," );
			sprintf(buffer,"%.2f",(*result_list_itr)->hp1->yBase);
			strcat(log_text,  buffer);
			strcat(log_text, ")-(" );
			sprintf(buffer,"%.2f",(*result_list_itr)->hp2->xBase);
			strcat(log_text,  buffer);
			strcat(log_text, "," );
			sprintf(buffer,"%.2f",(*result_list_itr)->hp2->yBase);
			strcat(log_text, buffer);
			strcat(log_text, ")\t");
			}*/

			if(glbl_var.debugScreen)
				cvPutText(glbl_var.displayedFrame,log_text, cvPoint(x,y), &font,useColor);
			(*itr_obj)->abandoned = true;

			strcat(log_text,"\r\n\r\n");
			strcpy(glbl_var.behaviour_log,log_text);
			strcpy(log_text,"");
			y += 10*glbl_var.resize_factor;

			//add ID for purposes of CPUfile
			glbl_var.abandonedLuggaeID.insert(glbl_var.abandonedLuggaeID.begin(),(*itr_obj)->blob->ID );


		}
	}

	//now check if you want walking
	itrObj2 = walking_objects2.begin();
	for(itrObj1 = walking_objects1.begin() ; itrObj1 != walking_objects1.end()  ; itrObj1++)
	{
			bool condition1 = ((find(fighting_objects1.begin() ,fighting_objects1.end(),(*itrObj1)) == fighting_objects1.end()) && (find(fighting_objects2.begin() ,fighting_objects2.end(),(*itrObj2)) == fighting_objects2.end()));
			bool condition2 = ((find(fighting_objects1.begin() ,fighting_objects1.end(),(*itrObj2)) == fighting_objects1.end()) && (find(fighting_objects2.begin() ,fighting_objects2.end(),(*itrObj1)) == fighting_objects2.end()));
			if(condition1 && condition2) //if not found, print it
			{
				strcat(log_text,"Person ");
				strcat(log_text,itoa((*itrObj1)->blob->ID, buffer,10)  );
				strcat(log_text," is WALKING with person ");
				strcat(log_text,itoa((*itrObj2)->blob->ID,buffer,10) );
				/*strcat(log_text, " at possible points: ");
				for(result_list_itr = temp_objects_list.begin(); result_list_itr != temp_objects_list.end() ; result_list_itr++ )
				{
				strcat(log_text, "(" );
				sprintf(buffer,"%.2f",(*result_list_itr)->hp1->xBase);
				strcat(log_text,  buffer);
				strcat(log_text, "," );
				sprintf(buffer,"%.2f",(*result_list_itr)->hp1->yBase);
				strcat(log_text,  buffer);
				strcat(log_text, ")-(" );
				sprintf(buffer,"%.2f",(*result_list_itr)->hp2->xBase);
				strcat(log_text,  buffer);
				strcat(log_text, "," );
				sprintf(buffer,"%.2f",(*result_list_itr)->hp2->yBase);
				strcat(log_text, buffer);
				strcat(log_text, ")\t");
				}*/

				if(glbl_var.debugScreen)
					cvPutText(glbl_var.displayedFrame,log_text, cvPoint(x,y), &font,useColor);
				(*itrObj1)->walk = true;
				(*itrObj2)->walk = true;

				strcat(log_text,"\r\n\r\n");
				strcpy(glbl_var.behaviour_log,log_text);
				strcpy(log_text,"");
				y += 10*glbl_var.resize_factor;

				//add ID for purposes of CPUfile
				glbl_var.walkTogetherID.insert(glbl_var.walkTogetherID.begin(),(*itrObj1)->blob->ID );
				glbl_var.walkTogetherID.insert(glbl_var.walkTogetherID.begin(),(*itrObj2)->blob->ID );



			}
			itrObj2++;
	}

	//now check if you want meeting
	itrObj2 = meeting_objects2.begin();
	for(itrObj1 = meeting_objects1.begin() ; itrObj1 != meeting_objects1.end()  ; itrObj1++)
	{

		bool condition1_1 = (find(fighting_objects1.begin() ,fighting_objects1.end(),(*itrObj1)) == fighting_objects1.end()) && (find(fighting_objects2.begin() ,fighting_objects2.end(),(*itrObj2)) == fighting_objects2.end());
		bool condition1_2 = (find(fighting_objects1.begin() ,fighting_objects1.end(),(*itrObj2)) == fighting_objects1.end()) && (find(fighting_objects2.begin() ,fighting_objects2.end(),(*itrObj1)) == fighting_objects2.end());
		bool condition2_1 = (find(walking_objects1.begin() ,walking_objects1.end(),(*itrObj1)) == walking_objects1.end()) && (find(walking_objects2.begin() ,walking_objects2.end(),(*itrObj2)) == walking_objects2.end());
		bool condition2_2 = (find(walking_objects1.begin() ,walking_objects1.end(),(*itrObj2)) == walking_objects1.end()) && (find(walking_objects2.begin() ,walking_objects2.end(),(*itrObj1)) == walking_objects2.end());
		if(condition1_1 && condition1_2 && condition2_1 && condition2_2) //if not found, print it
		{
			strcat(log_text,"Person ");
			strcat(log_text,itoa((*itrObj1)->blob->ID ,buffer,10) );
			strcat(log_text," is MEETING with person ");
			strcat(log_text,itoa((*itrObj2)->blob->ID,buffer,10) );
			/*strcat(log_text, " at possible points: ");
			for(result_list_itr = temp_objects_list.begin(); result_list_itr != temp_objects_list.end() ; result_list_itr++ )
			{
			strcat(log_text, "(" );
			sprintf(buffer,"%.2f",(*result_list_itr)->hp1->xBase);
			strcat(log_text,  buffer);
			strcat(log_text, "," );
			sprintf(buffer,"%.2f",(*result_list_itr)->hp1->yBase);
			strcat(log_text,  buffer);
			strcat(log_text, ")-(" );
			sprintf(buffer,"%.2f",(*result_list_itr)->hp2->xBase);
			strcat(log_text,  buffer);
			strcat(log_text, "," );
			sprintf(buffer,"%.2f",(*result_list_itr)->hp2->yBase);
			strcat(log_text, buffer);
			strcat(log_text, ")\t");
			}*/

			if(glbl_var.debugScreen)
				cvPutText(glbl_var.displayedFrame,log_text, cvPoint(x,y), &font,useColor);
			(*itrObj1)->meet = true;
			(*itrObj2)->meet = true;

			strcat(log_text,"\r\n\r\n");
			strcpy(glbl_var.behaviour_log,log_text);
			strcpy(log_text,"");
			y += 10*glbl_var.resize_factor;

			//add ID for purposes of CPUfile
			glbl_var.meetID.insert(glbl_var.meetID.begin(),(*itrObj1)->blob->ID );
			glbl_var.meetID.insert(glbl_var.meetID.begin(),(*itrObj2)->blob->ID );

		}
		itrObj2++;
	}

	//make sure list has only unique numbers
	glbl_var.fightID.sort(); glbl_var.fightID.unique();
	glbl_var.walkTogetherID.sort(); glbl_var.walkTogetherID.unique();
	glbl_var.meetID.sort(); glbl_var.meetID.unique();



	//fill the blobs with text
	//for all parent and standalone objects, if it or any of the children has the behaviour, write it on its CC
	if(!glbl_var.debugScreen)
	{
		for(itrObj1 = glbl_var.BGmodel->trackerBlobs.begin() ; itrObj1 != glbl_var.BGmodel->trackerBlobs.end()  ; itrObj1++) 
		{
			if(!(*itrObj1)->parent.empty() || !(*itrObj1)->CC )
				continue;

			int temp_x = (*itrObj1)->CC->blob->x*glbl_var.resize_factor;
			int temp_y = ((*itrObj1)->CC->blob->y + (*itrObj1)->CC->blob->h/2)*glbl_var.resize_factor ;
			list<TrackerObject*> temp_list = (*itrObj1)->occlusion_list;
			temp_list.push_front(*itrObj1);

			for(itrObj2 = temp_list.begin()  ; itrObj2 != temp_list.end() ; itrObj2++)
			{
				if((*itrObj2)->walk)
				{
					cvPutText(glbl_var.displayedFrame,"WALK_TOGETHER", cvPoint(temp_x,temp_y), &font,( glbl_var.BGmodel->get_t()%2  ? useColor : useColor2));
					temp_y+=10*glbl_var.resize_factor;
					break;
				}
			}
			for(itrObj2 = temp_list.begin()  ; itrObj2 != temp_list.end() ; itrObj2++)
			{
				if((*itrObj2)->meet)
				{
					cvPutText(glbl_var.displayedFrame,"MEET", cvPoint(temp_x,temp_y), &font,( glbl_var.BGmodel->get_t()%2  ? useColor : useColor2));
					temp_y+=10*glbl_var.resize_factor;
					break;
				}
			}
			for(itrObj2 = temp_list.begin()  ; itrObj2 != temp_list.end() ; itrObj2++)
			{
				if((*itrObj2)->fight)
				{
					cvPutText(glbl_var.displayedFrame,"FIGHT", cvPoint(temp_x,temp_y), &font,( glbl_var.BGmodel->get_t()%2  ? useColor : useColor2));
					temp_y+=10*glbl_var.resize_factor;
					break;
				}
			}
			for(itrObj2 = temp_list.begin()  ; itrObj2 != temp_list.end() ; itrObj2++)
			{
				if((*itrObj2)->faint)
				{
					cvPutText(glbl_var.displayedFrame,"FAINT", cvPoint(temp_x,temp_y), &font,( glbl_var.BGmodel->get_t()%2  ? useColor : useColor2));
					temp_y+=10*glbl_var.resize_factor;
					break;
				}
			}
			for(itrObj2 = temp_list.begin()  ; itrObj2 != temp_list.end() ; itrObj2++)
			{
				if((*itrObj2)->loiter)
				{
					cvPutText(glbl_var.displayedFrame,"LOITER", cvPoint(temp_x,temp_y), &font,( glbl_var.BGmodel->get_t()%2  ? useColor : useColor2));
					temp_y+=10*glbl_var.resize_factor;
					break;
				}
			}
			for(itrObj2 = temp_list.begin()  ; itrObj2 != temp_list.end() ; itrObj2++)
			{
				if((*itrObj2)->abandoned)
				{
					cvPutText(glbl_var.displayedFrame,"ABANDONED", cvPoint(temp_x,temp_y), &font,( glbl_var.BGmodel->get_t()%2  ? useColor : useColor2));
					temp_y+=10*glbl_var.resize_factor;
					break;
				}
			}
			for(itrObj2 = temp_list.begin()  ; itrObj2 != temp_list.end() ; itrObj2++)
			{
				if((*itrObj2)->steal)
				{
					cvPutText(glbl_var.displayedFrame,"STEAL", cvPoint(temp_x,temp_y), &font,( glbl_var.BGmodel->get_t()%2  ? useColor : useColor2));
					temp_y+=10*glbl_var.resize_factor;
					break;
				}
			}

		}
	}
	//reset actions for all objects
	for(itrObj1 = glbl_var.BGmodel->trackerBlobs.begin() ; itrObj1 != glbl_var.BGmodel->trackerBlobs.end()  ; itrObj1++)
	{

		(*itrObj1)->walk = false;
		(*itrObj1)->meet = false;
		(*itrObj1)->loiter = false;
		(*itrObj1)->faint = false;
		(*itrObj1)->steal = false;
		(*itrObj1)->abandoned = false;
		(*itrObj1)->fight = false;

	}



}



bool BehaviourMatrix::abandonedObject(TrackerObject* obj, list<twoHistoryPointsInteraction*>* matched_twoObjHistoryPoints)
{
	if(glbl_var.saveFramesToHard)
	{
		behaviourFile << "{Checking \"abandonedObject\" for object " << obj->blob->ID  << endl; 
	}

	if(obj->object_classification != OBJECT) //it has to be an object
	{	
		if(glbl_var.saveFramesToHard)
		{
			behaviourFile << "FALSE: it is not an object}" << endl; 
		}

		return false; 
	}

	if(obj->belongs_to == NULL) 
	{
		if(glbl_var.saveFramesToHard)
		{
			behaviourFile << "FALSE: it has no owner}" << endl; 
		}

		return false; //if it has no owner, then no it isn't abandoned
	}


	list<twoHistoryPointsInteraction*>::iterator itr;
	list<twoHistoryPointsInteraction*>::iterator itr_2;


	bool isAbandoned = isFarFrom( obj, obj->belongs_to,ABANDONED_LUGGAGE_DISTANCE_THRESHOLD , matched_twoObjHistoryPoints);
	for(itr = matched_twoObjHistoryPoints->begin() ; itr != matched_twoObjHistoryPoints->end(); itr++)
	{
		(*itr)->isAbandonedBy = true;
	}
	

	if (isAbandoned)
	{
		//(i)
		/*itr = matched_twoObjHistoryPoints->begin() ;
		while( itr != matched_twoObjHistoryPoints->end() )
		{
			bool result= (*itr)->checkPairPrecedingAbandoned();

			if(!result)
			{

				if(glbl_var.saveFramesToHard)
				{
					behaviourFile << "it HASN'T been abandoned at object" << (*itr)->hp1->xBase << "," << (*itr)->hp1->yBase <<" and person " << (*itr)->hp2->xBase << "," << (*itr)->hp2->yBase << endl; 
				}

				itr_2 = itr;
				itr++;
				matched_twoObjHistoryPoints->erase(itr_2);
			}
			else
			{
				if(glbl_var.saveFramesToHard)
				{
					behaviourFile << "+1 it HAS been abandoned at object" << (*itr)->hp1->xBase << "," << (*itr)->hp1->yBase <<" and person " << (*itr)->hp2->xBase << "," << (*itr)->hp2->yBase << endl; 
				}

				(*itr)->hasBeenAbandonedBy;
				itr++;
			}
		}
		if(glbl_var.saveFramesToHard)
		{
			behaviourFile << (matched_twoObjHistoryPoints->empty() ? "FALSE" : "TRUE" ) << "}" <<endl;
		}

		return !(matched_twoObjHistoryPoints->empty());
		*/
		//(ii)
		bool result = true;
		list<list<object1twoObjectHistoryPoint*>*>* hl = retrieveHistoryLine(obj,obj->belongs_to);
		list<list<object1twoObjectHistoryPoint*>*>::iterator hl_itr;
		if(hl)
		{
			hl_itr = hl->begin();
			hl_itr++;
		}
		else
		{
			if(glbl_var.saveFramesToHard)
			{
				behaviourFile <<  "FALSE: no enough history}" <<endl;
			}
			return false;
		}
		list<object1twoObjectHistoryPoint*>::iterator hp1_itr;
		list<twoHistoryPointsInteraction*>::iterator hp2_itr;
		while(hl_itr != hl->end() )
		{
			//int time_difference = glbl_var.sampledAbsoluteTime - (*(*(*hl_itr)->begin())->historyPointsObj2.begin())->frameNumber;
			if ( glbl_var.BGmodel->get_t() - (*(*(*hl_itr)->begin())->historyPointsObj2.begin())->frameNumber > ABANDONED_LUGGAGE_DURATION_THRESHOLD)
			{
				if(glbl_var.saveFramesToHard)
				{
					behaviourFile <<  "TRUE}" <<endl;
				}

				for(itr = matched_twoObjHistoryPoints->begin() ; itr != matched_twoObjHistoryPoints->end(); itr++)
				{
					(*itr)->hasBeenAbandonedBy = true;
				}
				return true;
			}

			bool found = false;
			for(hp1_itr  = (*hl_itr)->begin() ; hp1_itr  != (*hl_itr)->end() ; hp1_itr++)
			{
				for(hp2_itr = (*hp1_itr)->historyPointsObj2.begin()  ; hp2_itr != (*hp1_itr)->historyPointsObj2.end() ; hp2_itr++)
				{
					if ((*hp2_itr)->hasBeenAbandonedBy)
					{
						if(glbl_var.saveFramesToHard)
						{
							behaviourFile <<  "TRUE}" <<endl;
						}

						return true;
					}
					if ( (*hp2_itr)->isAbandonedBy) 
						found = true;
				}
			}
			if (!found) 
			{
				if(glbl_var.saveFramesToHard)
				{
					behaviourFile <<  "FALSE: it hasn't been abandoned} " <<endl;
				}
				return false;
			}

			hl_itr++;

		}
		//if (glbl_var.sampledAbsoluteTime - (*(*(*hl_itr)->begin())->historyPointsObj2.begin())->frameNumber > ABANDONED_LUGGAGE_DURATION_THRESHOLD)
		//if (glbl_var.BGmodel->get_t() - (*(*(*hl_itr)->begin())->historyPointsObj2.begin())->frameNumber > ABANDONED_LUGGAGE_DURATION_THRESHOLD)
		//{
			if(glbl_var.saveFramesToHard)
			{
				behaviourFile <<  "FALSE: no enough history}" <<endl;
			}
			return false;
		//}

		//hl_itr++;

		
	}
	else
	{
		if(glbl_var.saveFramesToHard)
		{
			behaviourFile << "FALSE: it is not abandoned}" << endl; 
		}

		return false;
	}
	
}

bool  BehaviourMatrix::stoleLuggage(TrackerObject* obj1, TrackerObject* obj2 , list<twoHistoryPointsInteraction*>* matched_twoObjHistoryPoints ,list<TrackerObject*>* abandonedObjects) // abandonedObject() should be called first to determine whether it is abandoned
{
	if(glbl_var.saveFramesToHard)
	{
		behaviourFile << "{Checking \"stoleLuggage\" for objects " << obj1->blob->ID << " and " << obj2->blob->ID  << endl; 
	}
	
	TrackerObject* obj = ( (obj1->object_classification == OBJECT) ? obj1: obj2);
	TrackerObject* thief = ( (obj1->object_classification == OBJECT) ? obj2: obj1);
	bool modifyAbandonedList =false; // if based on histogram, abandoned objects will not be considered abandoned if they are retrieved by a person with the same histogram.
	list<TrackerObject*>::iterator objItr;


	if(obj->object_classification != OBJECT)  //it has to be an object
	{
		if(glbl_var.saveFramesToHard)
		{
			behaviourFile << "FALSE: first is not an object}" << endl; 
		}

		return false; 
	}
	if(!thief->occlusion_list.empty()) 
	{
		if(glbl_var.saveFramesToHard)
		{
			behaviourFile << "FALSE: dummy object}" << endl; 
		}

		return false; //the thief must be a person
	}
	if(thief->object_classification == UNKNOWN || thief->object_classification == OBJECT) 
	{
		if(glbl_var.saveFramesToHard)
		{
			behaviourFile << "FALSE: second is not a person}" << endl; 
		}

		return false; //the thief must be a person
	}
	if(obj->belongs_to == thief)
	{
		if(glbl_var.saveFramesToHard)
		{
			behaviourFile << "FALSE: owner retrieved it (same ID)}" << endl; 
		}

		return false; //the thief must be a person
	}



	if(!obj->ownerHist) // a controversial case //(i) false because there has to be an owner, (ii) skip
	{
		//(i)
		if(glbl_var.saveFramesToHard && glbl_var.saveBehaviourFile)
		{
			behaviourFile << "FALSE: doesn't have an owner}" << endl; 
		}
		return false; //the thief must be a person

		//(ii)
		//just skip (do nothing)
	}
	//do we wanna detect based on ID or histogram ?
	else if(glbl_var.BehaviourDescriptor.detectAbandonedBasedOnHist)
	{
		bool condition;
		CvHistogram* hist1 = 0;
		cvCopyHist(thief->hist,&hist1);
		cvNormalizeHist(hist1,1);
		CvHistogram* hist2 = 0;
		cvCopyHist(obj->ownerHist,&hist2);
		cvNormalizeHist(hist2,1);
		condition =  1 - cvCompareHist(hist1, hist2,CV_COMP_INTERSECT) < glbl_var.BGmodel->CC_Obj_max_acceptable_distance_onetoone;
		cvReleaseHist(&hist2);
		if (condition)
		{
			for(objItr = abandonedObjects->begin(); objItr != abandonedObjects->end(); objItr++)
			{
				if((*objItr)->blob->ID == obj->blob->ID)
				{
					abandonedObjects->erase(objItr);
					if(glbl_var.saveFramesToHard && glbl_var.saveBehaviourFile)
					{
						behaviourFile << "FALSE: owner retrieved it (same Hist)}" << endl; 
					}
					return false;
				}
			}
		}

	}


	//select abandoned candidate hp
	list<object1twoObjectHistoryPoint*>::iterator itr;
	list<twoHistoryPointsInteraction*>::iterator itr2;
	list<object1twoObjectHistoryPoint*>* current_cl;
	if(obj->belongs_to)
	{
		current_cl =  retrievecurrentHistoryLine(obj,obj->belongs_to);
		if(!current_cl) 
		{
			if(glbl_var.saveFramesToHard)
			{
				behaviourFile << "FALSE: there is no current history}" << endl; 
			}

			return false;
		}
		list<historyPoint*> obj1_valid_hps;
		for(itr = current_cl->begin() ; itr != current_cl->end() ; itr++)
		{
			for(itr2 = (*itr)->historyPointsObj2.begin() ; itr2 != (*itr)->historyPointsObj2.end() ; itr2++)
			{
				if((*itr2)->isAbandonedBy)
				{
					if(glbl_var.saveFramesToHard)
					{
						behaviourFile << "object " << obj->blob->ID <<" is abandoned at " << (*itr2)->hp1->xBase << "," << (*itr2)->hp1->yBase  << endl; 
					}

					//matched_twoObjHistoryPoints->push_front(*itr); //add only abandoned historypoints to the candidates
					obj1_valid_hps.push_front((*itr2)->hp1);
				}
			}
		}
		if(obj1_valid_hps.empty())
		{
			if(glbl_var.saveFramesToHard)
			{
				behaviourFile << "FALSE: not abandoned!}" << endl; 
			}
			return false;
		}
		obj1_valid_hps.sort();
		obj1_valid_hps.unique();
	}
	 //if it has no owner then just continue

	//list<historyPoint*>::iterator obj1_valid_hps_itr;
	current_cl =  retrievecurrentHistoryLine(obj,thief);
	if(!current_cl) 
	{
		if(glbl_var.saveFramesToHard)
		{
			behaviourFile << "FALSE: there is no current history}" << endl; 
		}
		return false;
	}

	list<historyPoint*> temp_list1,temp_list2;
	list<historyPoint*>::iterator temp_list1_itr,temp_list2_itr;
	if(hasTaken(obj1,obj2,temp_list1,temp_list2))
	{

		temp_list1_itr = temp_list1.begin();
		temp_list2_itr = temp_list2.begin();
		for(int i=0; i < temp_list1.size(); i++)
		{
			bool toBreak = false;;
			for(itr = current_cl->begin() ; itr != current_cl->end() ; itr++)
			{
				for(itr2 = (*itr)->historyPointsObj2.begin() ; itr2 != (*itr)->historyPointsObj2.end() ; itr2++)
				{
					//if(find(obj1_valid_hps.begin(), obj1_valid_hps.end(), (*itr2)->hp1) != obj1_valid_hps.end())
					//{
					//if((*itr2)->takes)
					if((*itr2)->hp1 == (*temp_list1_itr) && (*itr2)->hp2 == (*temp_list2_itr))
					{

						if(glbl_var.saveFramesToHard)
						{
							behaviourFile << "object " << obj->blob->ID <<" takes " << thief->blob->ID << " at " << (*itr2)->hp1->xBase << "," << (*itr2)->hp1->yBase << "-" << (*itr2)->hp2->xBase << "," << (*itr2)->hp2->yBase << endl; 
						}

						matched_twoObjHistoryPoints->push_front(*itr2); 
						toBreak = true;
						break;
					}
				}
				if (toBreak)
					break;
				//}
			}
			temp_list1_itr++;
			temp_list2_itr++;
		}
	}

	matched_twoObjHistoryPoints->sort();
	matched_twoObjHistoryPoints->unique();

	if(glbl_var.saveFramesToHard)
	{
		behaviourFile << (matched_twoObjHistoryPoints->empty() ? "FALSE" : "TRUE" ) << "}" <<endl;
	}

	return !(matched_twoObjHistoryPoints->empty());


}

bool BehaviourMatrix::hasFainted(TrackerObject* person)
{
	if(glbl_var.saveFramesToHard)
	{
		behaviourFile << "{Checking \"hasFainted\" for object " << person->blob->ID  << endl; 
	}

	if(!person->occlusion_list.empty()) 
	{
		if(glbl_var.saveFramesToHard)
		{
			behaviourFile << "FALSE: dummy object}" << endl; 
		}

		return false; //the thief must be a person
	}

	if(person->object_classification != STILL_PERSON) //must be a still person
	{
		if(glbl_var.saveFramesToHard)
		{
			behaviourFile << "FALSE: it is not a still person}" << endl; 
		}

		return false; 
	}

	if(!(*person->currentHistoryMeasurement->begin())->falls)
	{

		if(glbl_var.saveFramesToHard)
		{
			behaviourFile << "FALSE: it is not falling}" << endl; 
		}

		return false;
	}


	//temporal condition
	//check for the prespecified duration and make sure it has fallen for the whole period
	list<list<historyPoint*>*>::iterator historyLine = person->history.begin();
	while(    (historyLine != person->history.end()) &&  ((glbl_var.BGmodel->get_t() - (*(*historyLine)->begin())->frameNumber) <  FAINT_DURATION_THRESHOLD)   )
	{
		if(!(*(*historyLine)->begin())->falls)
		{
			if(glbl_var.saveFramesToHard)
			{
				behaviourFile << "FALSE: it hasn't been fainting}" << endl; 
			}

			return false;
		}

		historyLine++;
	}

	if(glbl_var.saveFramesToHard)
	{
		behaviourFile << "TRUE}" << endl; 
	}
	return true;

}

bool BehaviourMatrix::meeting(TrackerObject* person1,TrackerObject* person2 , list<twoHistoryPointsInteraction*>* matched_twoObjHistoryPoints)
{
	if(glbl_var.saveFramesToHard)
	{
		behaviourFile << "{Checking \"meeting\" for objects " << person1->blob->ID << " and " << person2->blob->ID << endl; 
	}

	if(person1->object_classification == UNKNOWN || person1->object_classification == OBJECT ) //must be a person
	{
		if(glbl_var.saveFramesToHard)
		{
			behaviourFile << "FALSE: object1 is not a person}" << endl; 
		}

		return false; 
	}

	if(!person1->occlusion_list.empty() || !person2->occlusion_list.empty()) 
	{
		if(glbl_var.saveFramesToHard)
		{
			behaviourFile << "FALSE: dummy object}" << endl; 
		}

		return false; //the thief must be a person
	}


	if(person2->object_classification == UNKNOWN || person2->object_classification == OBJECT ) 
	{
		if(glbl_var.saveFramesToHard)
		{
			behaviourFile << "FALSE: object2 is not a person}" << endl; 
		}

		return false; //must be a person
	}

	list<twoHistoryPointsInteraction*>::iterator itr;
	list<twoHistoryPointsInteraction*>::iterator itr_2;


	//bool areClose = !isFarFrom( person1, person2,MEETING_DISTANCE_THRESHOLD, matched_twoObjHistoryPoints);
	bool areClose = isCloseTo( person1, person2,MEETING_DISTANCE_THRESHOLD, matched_twoObjHistoryPoints);
	//they should be close
	for(itr = matched_twoObjHistoryPoints->begin() ; itr != matched_twoObjHistoryPoints->end(); itr++)
	{
		(*itr)->meeting = true;
	}
	

	if (areClose)
	{
		//(i)
		/*itr = matched_twoObjHistoryPoints->begin() ;
		while( itr != matched_twoObjHistoryPoints->end() )
		{
			bool result = (*itr)->checkPairPrecedingClose();

			if(!result)
			{
				itr_2 = itr;
				itr++;
				matched_twoObjHistoryPoints->erase(itr_2);
			}
			else
			{
				if(glbl_var.saveFramesToHard)
				{
					behaviourFile << "they have met at " << (*itr)->hp1->xBase << "," << (*itr)->hp1->yBase << "=" << (*itr)->hp2->xBase << "," << (*itr)->hp2->yBase <<  " }" << endl; 

				}
				(*itr)->hasBeenMeeting;
				itr++;
			}
			}

			return !(matched_twoObjHistoryPoints->empty());*/
		//(ii)
		bool result = true;
		list<list<object1twoObjectHistoryPoint*>*>* hl = retrieveHistoryLine(person1,person2);
		list<list<object1twoObjectHistoryPoint*>*>::iterator hl_itr;
		if(hl)
		{
			hl_itr = hl->begin();
			hl_itr++;
		}
		else
		{
			if(glbl_var.saveFramesToHard)
			{
				behaviourFile <<  "FALSE: no enough history}" <<endl;
			}
			return false;
		}
		list<object1twoObjectHistoryPoint*>::iterator hp1_itr;
		list<twoHistoryPointsInteraction*>::iterator hp2_itr;
		while(hl_itr != hl->end() )
		{
			//if (glbl_var.sampledAbsoluteTime - (*(*(*hl_itr)->begin())->historyPointsObj2.begin())->frameNumber > MEETING_DURATION_THRESHOLD)
			if (glbl_var.BGmodel->get_t() - (*(*(*hl_itr)->begin())->historyPointsObj2.begin())->frameNumber > MEETING_DURATION_THRESHOLD)
			{
				if(glbl_var.saveFramesToHard)
				{
					behaviourFile <<  "TRUE}" <<endl;
				}

				//matched_twoObjHistoryPoints = (*hl_itr);
				for(itr = matched_twoObjHistoryPoints->begin() ; itr != matched_twoObjHistoryPoints->end(); itr++)
				{
					(*itr)->hasBeenMeeting = true;
				}
				return true;
			}

			bool found = false;
			for(hp1_itr  = (*hl_itr)->begin() ; hp1_itr  != (*hl_itr)->end() ; hp1_itr++)
			{
				for(hp2_itr = (*hp1_itr)->historyPointsObj2.begin()  ; hp2_itr != (*hp1_itr)->historyPointsObj2.end() ; hp2_itr++)
				{
					if ((*hp2_itr)->hasBeenMeeting)
					{
						if(glbl_var.saveFramesToHard)
						{
							behaviourFile <<  "TRUE}" <<endl;
						}

						return true;
					}
					if ( (*hp2_itr)->meeting) 
						found = true;
				}
			}
			if (!found) 
			{
				if(glbl_var.saveFramesToHard)
				{
					behaviourFile <<  "FALSE: they haven't been meeting} " <<endl;
				}
				return false;
			}


			hl_itr++;

		}
		//if (glbl_var.BGmodel->get_t() - (*(*(*hl_itr)->begin())->historyPointsObj2.begin())->frameNumber > MEETING_DURATION_THRESHOLD)
		//if (glbl_var.sampledAbsoluteTime - (*(*(*hl_itr)->begin())->historyPointsObj2.begin())->frameNumber > MEETING_DURATION_THRESHOLD)
		//{
			if(glbl_var.saveFramesToHard)
			{
				behaviourFile <<  "FALSE: no enough history}" <<endl;
			}
			return false;
		//}




	}
	else
	{
		if(glbl_var.saveFramesToHard)
		{
			behaviourFile << "FALSE: they are not close}" << endl; 
		}

		return false;
	}

}

bool BehaviourMatrix::areFighting(TrackerObject* person1,TrackerObject* person2,list<twoHistoryPointsInteraction*> &matched_twoObjHistoryPoints)
{
	if(glbl_var.saveFramesToHard)
	{
		behaviourFile << "{Checking \"areFighting\" for objects " << person1->blob->ID << " and " << person2->blob->ID << endl; 
	}

	if(!person1->occlusion_list.empty() || !person2->occlusion_list.empty()) 
	{
		if(glbl_var.saveFramesToHard)
		{
			behaviourFile << "FALSE: dummy object}" << endl; 
		}

		return false; //the thief must be a person
	}

	if(person1->object_classification == UNKNOWN || person1->object_classification == OBJECT ) 
	{
		if(glbl_var.saveFramesToHard)
		{
			behaviourFile << "FALSE: object1 is not a person}" << endl; 
		}

		return false; //must be a person
	}
	if(person2->object_classification == UNKNOWN || person2->object_classification == OBJECT )
	{
		if(glbl_var.saveFramesToHard)
		{
			behaviourFile << "FALSE: object2 is not a person}" << endl; 
		}

		return false; //must be a person
	}


	//high frequency of split and merge
	list<twoHistoryPointsInteraction*>::iterator itr;
	list<twoHistoryPointsInteraction*>::iterator itr2;
	list<Object1twoObjectPossibility*>::iterator mp1_itr;
	list<twoPossibilitiesInteraction*>::iterator mp2_itr;


	//fill the list with all possible points
	list<object1twoObjectHistoryPoint*>::iterator itr_hp1;
	list<object1twoObjectHistoryPoint*>* current_hl = retrievecurrentHistoryLine(person1,person2);
	if(!current_hl) 
	{
		if(glbl_var.saveFramesToHard)
		{
			behaviourFile << "FALSE: no current history}" << endl; 
		}

		return false;
	}

	bool possiblyMerged = false;
	for(itr_hp1 = current_hl->begin() ; itr_hp1 != current_hl->end() ; itr_hp1++)
	{
		for(itr = (*itr_hp1)->historyPointsObj2.begin(); itr != (*itr_hp1)->historyPointsObj2.end() ; itr++)
		{
			matched_twoObjHistoryPoints.push_back(*itr);
			/*if((*itr)->merged)
				possiblyMerged = true;*/

		}
	}

	itr = matched_twoObjHistoryPoints.begin() ;
	int num_of_changes = 0;
	bool lastMerged;
	while( itr != matched_twoObjHistoryPoints.end() )
	{
		//(i)
		//int result= (*itr)->checkPairPrecedingMergeChanges();
		//if(result < SPLIT_MERGE_FREQUENCY)
		//{
		//	itr2 = itr;
		//	itr++;
		//	matched_twoObjHistoryPoints.erase(itr2);
		//}
		//else
		//{

		//	if(glbl_var.saveFramesToHard)
		//	{
		//		behaviourFile << "they have a split frequency of " << result << endl; 
		//	}

		//	itr++;
		//}

		//(ii)
		lastMerged = (*itr)->merged;
		int number_of_changes = 0;
		list<list<object1twoObjectHistoryPoint*>*>* hl = retrieveHistoryLine(person1,person2);
		list<list<object1twoObjectHistoryPoint*>*>::iterator hl_itr;
		if(hl)
		{
			hl_itr = hl->begin();
			hl_itr++;
		}
		else
		{
			if(glbl_var.saveFramesToHard)
			{
				behaviourFile <<  "FALSE: no enough history}" <<endl;
			}
			return false;
		}
		list<object1twoObjectHistoryPoint*>::iterator hp1_itr;
		list<twoHistoryPointsInteraction*>::iterator hp2_itr;

		while(hl_itr != hl->end() )
		{
			
			bool toBreak = false;
			for(hp1_itr  = (*hl_itr)->begin() ; hp1_itr  != (*hl_itr)->end() ; hp1_itr++)
			{
				for(hp2_itr = (*hp1_itr)->historyPointsObj2.begin()  ; hp2_itr != (*hp1_itr)->historyPointsObj2.end() ; hp2_itr++)
				{
					bool condition1 = (*hp2_itr)->merged != lastMerged;// the change of merge condition
					bool condition2 = false;//if split then must have high enough velocities at different directions
					double dummy = -1, dummySpeed1 = -1, dummySpeed2 = -1;
					if((*hp2_itr)->merged)
						condition2 = true;
					else
					{
						bool found= false;
						for(mp1_itr = (*hp2_itr)->possibilitiesObj1.begin() ; mp1_itr != (*hp2_itr)->possibilitiesObj1.end() ;mp1_itr++)
						{
							for(mp2_itr = (*mp1_itr)->possibilitiesObj2.begin() ; mp2_itr != (*mp1_itr)->possibilitiesObj2.end() ;mp2_itr++)
							{
								if (!sameDirection((*mp2_itr)->mp1,(*mp2_itr)->mp2,dummy) && (  (*mp2_itr)->mp1->speed > glbl_var.BGmodel->MOTION_TOLERANCE/TIME_MEASUREMENT_STEP   ||   (*mp2_itr)->mp2->speed > glbl_var.BGmodel->MOTION_TOLERANCE/TIME_MEASUREMENT_STEP  )   )
								{
									dummySpeed1 = (*mp2_itr)->mp1->speed;
									dummySpeed2 = (*mp2_itr)->mp2->speed;
									condition2 = true;
									found = true;
									break;
								}	
							}
							if(found) break;
						}
						//if(possiblyMerged) found=false; //to avoid too many false positives, we only consider persons if there is no way they are merged

					}
					if (condition1 && condition2 ) 
					{
						behaviourFile <<  "a change with high speed and different directions at f" << (*(*(*hl_itr)->begin())->historyPointsObj2.begin())->frameNumber <<"; speed1  = " << dummySpeed1 << "; speed2 = " << dummySpeed2 << "; angle = " << dummy*180/3.14 <<  " }" <<endl;

						lastMerged = !lastMerged;
						number_of_changes++;
						toBreak = true;
						break;
					}
				}
				if(toBreak) 
					break;
			}

			if(number_of_changes > SPLIT_MERGE_FREQUENCY)
				{
					if(glbl_var.saveFramesToHard)
					{
						behaviourFile <<  "TRUE! " << number_of_changes << " > " << SPLIT_MERGE_FREQUENCY << " }" <<endl;
					}

					return true;
				}

			//if (glbl_var.sampledAbsoluteTime - (*(*(*hl_itr)->begin())->historyPointsObj2.begin())->frameNumber > FIGHTING_TIME_SPAN && number_of_changes > SPLIT_MERGE_FREQUENCY)
			if (glbl_var.BGmodel->get_t() - (*(*(*hl_itr)->begin())->historyPointsObj2.begin())->frameNumber > FIGHTING_TIME_SPAN)
			{
				/*if(number_of_changes > SPLIT_MERGE_FREQUENCY)
				{
					if(glbl_var.saveFramesToHard)
					{
						behaviourFile <<  "TRUE! " << number_of_changes << " > " << SPLIT_MERGE_FREQUENCY << " }" <<endl;
					}

					return true;
				}
				else*/
				{
					if(glbl_var.saveFramesToHard)
					{
						behaviourFile <<  "FALSE: not enough frequency within time span!" << number_of_changes << " < " << SPLIT_MERGE_FREQUENCY << "}" <<endl;
					}

					return false;
				}
			}


			hl_itr++;
		}
		/*if (glbl_var.sampledAbsoluteTime - (*(*(*hl_itr)->begin())->historyPointsObj2.begin())->frameNumber > FIGHTING_TIME_SPAN)
		{*/
		if(glbl_var.saveFramesToHard)
		{
			behaviourFile <<  "FALSE: no enough history}" <<endl;
		}
		return false;
		//}


	}
	if(glbl_var.saveFramesToHard)
	{
		behaviourFile <<  "FALSE: no enough history}" <<endl;
	}
	return false;

	//(i)
	//if(glbl_var.saveFramesToHard)
	//{
	//	behaviourFile << ( matched_twoObjHistoryPoints.empty() ? "FALSE": "TRUE" ) << "}" << endl; 
	//}

	//if(matched_twoObjHistoryPoints.empty())
	//	return false;
	//else
	//	return true;
}

bool BehaviourMatrix::walkingTogether(TrackerObject* person1,TrackerObject* person2, list<twoHistoryPointsInteraction*>* matched_hps) //should be called after meeting
{

	if(glbl_var.saveFramesToHard)
	{
		behaviourFile << "{Checking \"walkingTogether\" for objects " << person1->blob->ID << " and " << person2->blob->ID << endl; 
	}

	if(!person1->occlusion_list.empty() || !person2->occlusion_list.empty()) 
	{
		if(glbl_var.saveFramesToHard)
		{
			behaviourFile << "FALSE: dummy object}" << endl; 
		}

		return false; //the thief must be a person
	}

	if(person1->object_classification == UNKNOWN || person1->object_classification == OBJECT )
	{
		if(glbl_var.saveFramesToHard)
		{
			behaviourFile << "FALSE: object1 is not a person}" << endl; 
		}

		return false; //must be a person
	}
	if(person2->object_classification == UNKNOWN || person2->object_classification == OBJECT ) 
	{
		if(glbl_var.saveFramesToHard)
		{
			behaviourFile << "FALSE: object2 is not a person}" << endl; 
		}

		return false; //must be a person
	}

	//choose the current hps with meeting
	list<object1twoObjectHistoryPoint*>* current_hl = retrievecurrentHistoryLine(person1,person2);
	if(!current_hl) 
	{

		if(glbl_var.saveFramesToHard)
		{
			behaviourFile << "FALSE: not current history}" << endl; 
		}

		return false;
	}
	list<object1twoObjectHistoryPoint*>::iterator itr_hpObj1;
	list<twoHistoryPointsInteraction*>::iterator itr_hpObj2;
	for( itr_hpObj1 = current_hl->begin() ; itr_hpObj1 != current_hl->end(); itr_hpObj1++)
	{
		for( itr_hpObj2 = (*itr_hpObj1)->historyPointsObj2.begin() ; itr_hpObj2 != (*itr_hpObj1)->historyPointsObj2.end(); itr_hpObj2++)
		{
			if ((*itr_hpObj2)->hasBeenMeeting)
			{
				if(glbl_var.saveFramesToHard)
				{
					behaviourFile << "they have been meeting at " << (*itr_hpObj2)->hp1->xBase << "," << (*itr_hpObj2)->hp1->yBase << "-" << (*itr_hpObj2)->hp2->xBase << "," << (*itr_hpObj2)->hp2->yBase << endl; 
				}


				matched_hps->push_front(*itr_hpObj2);
			}
		}
	}
	if (matched_hps->empty())
	{
		if(glbl_var.saveFramesToHard)
		{
			behaviourFile << "FALSE: not meeting}" << endl; 
		}
		return false;
	}


	//from the remaining ones, choose the ones with smae speed and direction, and moving
	list<Object1twoObjectPossibility*>::iterator itr_p1;
	list<twoPossibilitiesInteraction*>::iterator itr_p2,itr_p2_temp;
	bool result  = false;
	
	for(itr_hpObj2 = matched_hps->begin() ; itr_hpObj2 != matched_hps->end() ; itr_hpObj2++) //for all matched hps
	{	
		for( itr_p1 = (*itr_hpObj2)->possibilitiesObj1.begin() ; itr_p1 != (*itr_hpObj2)->possibilitiesObj1.end(); itr_p1++ )
		{

			itr_p2 = (*itr_p1)->possibilitiesObj2.begin() ;
			while( itr_p2 != (*itr_p1)->possibilitiesObj2.end() ) //for all sub mps
			{
				//1- make sure they are moving and 2- of same speed and direction
				if (!(*itr_p2)->moving || !(*itr_p2)->sameSpeedDirection)
				{
					itr_p2_temp = itr_p2;
					itr_p2++;
					//(*itr_p1)->possibilitiesObj2.erase(itr_p2_temp);
				}
				else 
				{

					if(glbl_var.saveFramesToHard)
					{
						behaviourFile << "they have been walking together at " << (*itr_p2)->mp1->vxBase << "," << (*itr_p2)->mp1->vyBase << "-" << (*itr_p2)->mp2->vxBase << "," << (*itr_p2)->mp2->vyBase << endl; 
					}

					result = true;
					itr_p2++;
				}
			}

		}




	}

	if(glbl_var.saveFramesToHard)
	{
		behaviourFile << ( result ? "TRUE": "FALSE" ) << "}" << endl; 
	}
	return result;




}


//
//bool checkAlignmentParentDirectionChange(twoHistoryPointsInteraction* oldHp, twoHistoryPointsInteraction* newHp)
//{
//
//	//check alignment change
//	int PI = 3.1417;
//	double newAngle = atan(calcAlignment(obj1,obj2) );
//	twoPossibilitiesInteraction* ptr = retrieveLastHistoryLine(obj1,obj2);
//	double oldAngle;
//	if(ptr)
//		oldAngle = atan(ptr->alignment );
//	else
//		oldAngle = newAngle;
//	if(abs(oldAngle -  newAngle) > DIFFERENT_ANGLES_THRESHOLD) return true;
//
//
//	//direction condition
//	TrackerObject* commonParent = NULL;
//
//	list<TrackerObject*>::iterator ptrObj1;
//	list<TrackerObject*>::iterator ptrObj2;
//
//	for (ptrObj1= obj1->parent.begin(); ptrObj1 != obj1->parent.end(); ptrObj1++)
//	{
//		for (ptrObj2= obj2->parent.begin(); ptrObj2 != obj2->parent.end(); ptrObj2++)
//		{
//
//			if((*ptrObj1)->blob->ID == (*ptrObj2)->blob->ID)
//			{
//				commonParent = (*ptrObj1);
//				goto label_done;
//			}
//		}
//	}
//
//label_done:
//
//	if(commonParent) 
//	{
//		double xDirectionOld = commonParent->lastHistoryMeasurement->xDirection;
//		double xDirectionNew = (*commonParent->history.begin())->xDirection;
//		double yDirectionOld = commonParent->lastHistoryMeasurement->yDirection;
//		double yDirectionNew = (*commonParent->history.begin())->yDirection;
//
//		if( (xDirectionOld < 0) != (xDirectionNew < 0) ) return true;
//		if( (yDirectionNew < 0) != (yDirectionOld < 0) ) return true;
//
//		int PI = 3.1417;
//		double angleOld = ( xDirectionOld == 0 ? ( yDirectionOld == 0 ? 0 : PI ) : atan(yDirectionOld/xDirectionOld ));
//		double angleNew = ( xDirectionNew == 0 ? ( yDirectionNew == 0 ? 0 : PI ) : atan(yDirectionNew/xDirectionNew ));
//
//		if(abs(angleOld -  angleNew) > DIFFERENT_ANGLES_THRESHOLD) return true;
//
//	}
//
//
//	return false;
//
//
//}
