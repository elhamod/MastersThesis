// Version3withInterface.cpp : main project file. (just loads the main form)

#include "stdafx.h"
#include "Form1.h"
#include <cv.h>
#include <cvaux.h>
#include <highgui.h>
#include "iostream"             
#include <time.h>
#include <windows.h>
#include "g_variables.h"

using namespace std;
using namespace Version3withInterface;

[STAThreadAttribute]
int main(array<System::String ^> ^args) {

	// Enabling Windows XP visual effects before any controls are created
	Application::EnableVisualStyles();
	Application::SetCompatibleTextRenderingDefault(false); 

	//Loads Form1
	Application::Run(gcnew Form1());

  return 0;
}
