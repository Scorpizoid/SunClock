// SunClock_CSunClockApplication.cpp
// ----------------------------------
//

////////////////////////////////////////////////////////////////////////
//
// Source file for the implementation of the 
// application class - CSunClockApplication
//
////////////////////////////////////////////////////////////////////////

#include "SunClock.h"

const TUid KUidSunClock = { 0x2002AA23 };

//             The function is called by the UI framework to ask for the
//             application's UID. The returned value is defined by the
//             constant KUidSunClock and must match the second value
//             defined in the project definition file.
// 
TUid CSunClockApplication::AppDllUid() const
	{
	return KUidSunClock;
	}

//             This function is called by the UI framework at
//             application start-up. It creates an instance of the
//             document class.
//
CApaDocument* CSunClockApplication::CreateDocumentL()
	{
	return new (ELeave) CSunClockDocument(*this);
	}



