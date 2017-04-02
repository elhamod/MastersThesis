#include "stdafx.h"
//#include "BGmodel.h"
#include "median.h"
#include <cv.h>
#include <cvaux.h>
#include <highgui.h>


int find_n(int array_size,double* arr, int tmp_index)
{
	if (array_size == 1) return 0;
	
	//randomally chosen pivot
  CvRNG rnggstate = cvRNG(0xffffffff);
  


  double *leftmatrix = new double[array_size];
  double *rightmatrix = new double[array_size];
  double *wholematrix = new double[array_size];
  int wholematrixlength = 0;

  //copy values from attribs to wholematrix
  for (int i=0; i<array_size; i++)
  {
	wholematrix[wholematrixlength] =arr[i];
	wholematrixlength++;
  }
  
  int pivot;
  do {
	  if (wholematrixlength == 0)
		  pivot = 0;
	  else
		  pivot = cvRandInt(&rnggstate )%wholematrixlength;
		  // split into the matrices
	  int leftmatrixcounter=0;
	  int rightmatrixcounter=0;
	  for (int i=0; i < wholematrixlength; i++)
	  {

		  if (wholematrix[i] < wholematrix[pivot])
		  {
			  leftmatrix[leftmatrixcounter] = wholematrix[i];
			  leftmatrixcounter++;
		  }
		  else if(wholematrix[i] > wholematrix[pivot])
		  {
				rightmatrix[rightmatrixcounter] = wholematrix[i];
				rightmatrixcounter++;
		  }

	  }
	 
	  if (tmp_index <= leftmatrixcounter)
	  {
		delete[]  wholematrix;
		wholematrix = leftmatrix;
		wholematrixlength = leftmatrixcounter;
		delete[] rightmatrix;
		leftmatrix = 0;
		leftmatrix = new double[leftmatrixcounter];
		rightmatrix = new double[leftmatrixcounter];
	  }
	  else  if (tmp_index > wholematrixlength - rightmatrixcounter)
	  {
		tmp_index = tmp_index - (wholematrixlength - rightmatrixcounter);
		delete[]  wholematrix;
		wholematrix = rightmatrix;
		wholematrixlength = rightmatrixcounter;
		delete[] leftmatrix;
		rightmatrix = 0;
		rightmatrix = new double[rightmatrixcounter];
		leftmatrix = new double[rightmatrixcounter];
		
	  }
	  else
	  {
		  double answer = wholematrix[pivot];
		  delete[] leftmatrix;
		  delete[] rightmatrix;
		  delete[] wholematrix;
		  //cout << "split_point is called: attribut " << attribs[attrib_index].name << "[" << index << "]" << " = " << answer << endl;

		  //return the index in the original matrix
		  for (int i=0; i<array_size; i++)
			if  (answer == arr[i]) return i;

	  }
  }
  while(1);


}


double find_median(int array_size,double* arr)
{
	if (array_size == 1) return arr[0];
	
	//randomally chosen pivot
  CvRNG rnggstate = cvRNG(0xffffffff);
  int tmp_index = cvFloor(array_size/2);
  


  double *leftmatrix = new double[array_size];
  double *rightmatrix = new double[array_size];
  double *wholematrix = new double[array_size];
  int wholematrixlength = 0;

  //copy values from attribs to wholematrix
  for (int i=0; i<array_size; i++)
  {
	wholematrix[wholematrixlength] =arr[i];
	wholematrixlength++;
  }
  
  int pivot;
  do {
	  if (wholematrixlength == 0)
		  pivot = 0;
	  else
		  pivot = cvRandInt(&rnggstate )%wholematrixlength;
		  // split into the matrices
	  int leftmatrixcounter=0;
	  int rightmatrixcounter=0;
	  for (int i=0; i < wholematrixlength; i++)
	  {

		  if (wholematrix[i] < wholematrix[pivot])
		  {
			  leftmatrix[leftmatrixcounter] = wholematrix[i];
			  leftmatrixcounter++;
		  }
		  else if(wholematrix[i] > wholematrix[pivot])
		  {
				rightmatrix[rightmatrixcounter] = wholematrix[i];
				rightmatrixcounter++;
		  }

	  }
	 
	  if (tmp_index <= leftmatrixcounter)
	  {
		delete[]  wholematrix;
		wholematrix = leftmatrix;
		wholematrixlength = leftmatrixcounter;
		delete[] rightmatrix;
		leftmatrix = 0;
		leftmatrix = new double[leftmatrixcounter];
		rightmatrix = new double[leftmatrixcounter];
	  }
	  else  if (tmp_index > wholematrixlength - rightmatrixcounter)
	  {
		tmp_index = tmp_index - (wholematrixlength - rightmatrixcounter);
		delete[]  wholematrix;
		wholematrix = rightmatrix;
		wholematrixlength = rightmatrixcounter;
		delete[] leftmatrix;
		rightmatrix = 0;
		rightmatrix = new double[rightmatrixcounter];
		leftmatrix = new double[rightmatrixcounter];
		
	  }
	  else
	  {
		  double answer = wholematrix[pivot];
		  delete[] leftmatrix;
		  delete[] rightmatrix;
		  delete[] wholematrix;
		  //cout << "split_point is called: attribut " << attribs[attrib_index].name << "[" << index << "]" << " = " << answer << endl;
		  return answer;
	  }
  }
  while(1);
}
