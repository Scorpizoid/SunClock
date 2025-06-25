// SunClock_Main.cpp
// -------------------
//

////////////////////////////////////////////////////////////////////////
//
// SunClock 
// ----------
//
//
//
// This source file contains the single exported function required by 
// all UI applications and the E32Dll function which is also required
// but is not used here.
//
////////////////////////////////////////////////////////////////////////


#include "SunClock.h"

//             The entry point for the application code. It creates
//             an instance of the CApaApplication derived
//             class, CSunClockApplication.
//
EXPORT_C CApaApplication* NewApplication()
	{
	return new CSunClockApplication;
	}

//             This function is required by all EPOC32 DLLs. In this 
//             app, it does nothing.
//
GLDEF_C TInt E32Dll(TDllReason)
	{
	return KErrNone;
	}

