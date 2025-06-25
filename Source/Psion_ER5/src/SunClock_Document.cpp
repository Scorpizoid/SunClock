// SunClock_CSunClockDocument.cpp
// -------------------------------
//

////////////////////////////////////////////////////////////////////////
//
// Source file for the implementation of the 
// document class - CSunClockDocument
//
////////////////////////////////////////////////////////////////////////

#include "SunClock.h"

//             The constructor of the document class just passes the
//             supplied reference to the constructor initialisation list.
//             The document has no real work to do in this application.
//
CSunClockDocument::CSunClockDocument(CEikApplication& aApp)
		: CEikDocument(aApp)
	{
	}


//             This is called by the UI framework as soon as the 
//             document has been created. It creates an instance
//             of the ApplicationUI. The Application UI class is
//             an instance of a CEikAppUi derived class.
//
CEikAppUi* CSunClockDocument::CreateAppUiL()
	{
    return new(ELeave) CSunClockAppUi;
	}



