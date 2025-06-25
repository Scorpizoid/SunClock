// SunClock.h
//
////////////////////////////////////////////////////////////////////
// SunClock 
// ----------
//
//
// The class definitions are:
//
// CSunClockApplication 
// CSunClockAppUi
// CSunClockAppView
// CSunClockDocument
//
//
////////////////////////////////////////////////////////////////////
#ifndef __SUNCLOCK_H
#define __SUNCLOCK_H

#include <coeccntx.h>

#include <eikenv.h>
#include <eikappui.h>
#include <eikapp.h>
#include <eikdoc.h>
#include <eikmenup.h>
#include <e32std.h>
#include <e32math.h>

#include <eikdialg.h> // Dialogit
#include <eikmfne.h> // CEikDateEditor

#include <coemain.h> // CCoeEnv
#include <s32stor.h> // CDictionaryStore
#include <f32file.h> // RFs
#include <s32file.h> // CFileStore

#include <eikchlst.h> // CEikChoiceList
#include <t32wld.h> // RWorldServer
#include <eikinfo.h> //CCknInfoDialog

#include "MoonPhase.h"
#include <math.h>
#include <eikmenu.hrh>


#include <eikon.rsg>
#include <eikcmds.hrh>

#include <SunClock.rsg>
#include "SunClock.hrh"



////////////////////////////////////////////////////////////////////////
//
// CSunClockApplication
//
////////////////////////////////////////////////////////////////////////

class CSunClockApplication : public CEikApplication
	{
private: 
	           // Inherited from class CApaApplication
	CApaDocument* CreateDocumentL();
	TUid AppDllUid() const;
	};

////////////////////////////////////////////////////////////////////////
//
// CSunClockAppView
//
////////////////////////////////////////////////////////////////////////
class CSunClockAppView : public CCoeControl
    {
public:
	static CSunClockAppView* NewL(const TRect& aRect);
	CSunClockAppView();
	~CSunClockAppView();
    void ConstructL(const TRect& aRect);

		// N�ihin tallennetaan DST:n alku- ja
		// loppup�iv�t
	TTime iDate;
	TTime iDate2;
	void SetDay(TTime aDate, TTime aDate2, TInt aChoice);

		// Vuosiluvun tallentaminen INI-tiedostoon
	TInt iYearNo;
	void SetYearL(TInt aYear);

		// P�ivien numerot (vuoden alusta),
		// jotka aloittavat ja lopettavat
		// DST:n
	TInt iDayDSTStarts;
	TInt iDayDSTEnds;

		// Kartan paikka, joka voi muuttua kun k�ytt�j�
		// valitsee toisen maanosan
	TInt iMapPosition;

		// Kumman k�ytt�j� valitsee - k�ytet��nk� kes�aikaa vai ei
	TInt iChoice;

		// Painettiinko Esc-nappia DST-dialogin aikana?
	TInt iEscWasPressed;

		// T�m� tarkistaa, onko ohjelma k�yt�ss� ensimm�ist� kertaa,
		// ja jos on, se poistaa vanhan INI-tiedoston, joka aiheuttaa
		// KERN-EXEC 3 -virheen, koska siin� on eri m��r� tietueita
		// kuin uudessa INI-tiedostossa.
	TInt iVersionNumber;

		// INI-tiedoston k�sittelyyn liittyv�t
		// funktiot
    void ExternalizeL(RWriteStream& aOut) const;
    void InternalizeL(RReadStream& aIn);
    void SaveL();

		// Palauttaa INI-tiedoston ID:n
    TStreamId StoreL( CStreamStore& aStore) const;
		// ?
	void RestoreL(const CStreamStore& aStore, TStreamId aId);

		// Auringonvaloaikaan liittyv�t muuttujat
	RWorldServer    iWS; 
    TWorldId		iWorldID;   
    TTime			iCurrentTime;   
    TInt			iErr2;   
	TTime			iSunrise;
	TTime			iSunset;
	TCityData		cityData;

		// ja funktio
	void RSunlight();


		// N�m� muuttujat koskevat napapiirien
		// ja k��nt�piirien/ ekvaattorin n�ytt�mist�.
	TInt iShowEquator;
	TInt iShowTropicOfCancer;
	TInt iShowTropicOfCapricorn;
	TInt iShowArcticCircle;
	TInt iShowAntarcticCircle;
	TInt16 pixelOffsetInt;


private:
		// Ajastimen funktio ja muuttuja
	static TInt PeriodicTimerCallBack(TAny* aAny);
	CPeriodic* iPeriodicTimer;
	static TInt PeriodicTimerCallBack2(TAny* aAny);
	CPeriodic* iPeriodicTimer2;
	static TInt PeriodicTimerCallBack3(TAny* aAny);
	CPeriodic* iPeriodicTimer3;

	           // Inherited from CCoeControl
	void Draw(const TRect& /*aRect*/) const;
    };


////////////////////////////////////////////////////////////////////////
//
// CSunClockAppUi
//
////////////////////////////////////////////////////////////////////////
class CSunClockAppUi : public CEikAppUi
    {
public:
    void ConstructL();
	~CSunClockAppUi();
	void LaunchDateDialogL();

private:
              // Inherited from class CEikAppUi
	void HandleCommandL(TInt aCommand);
				// K�sittelee ohjelman joutumista taka- tai etualalle
	void HandleForegroundEventL(TBool aForeground);

	void DynInitMenuPaneL(TInt aResourceId,CEikMenuPane* aMenuPane);

private:
	CSunClockAppView* iAppView;
	};


////////////////////////////////////////////////////////////////////////
//
// CSunClockDocument
//
////////////////////////////////////////////////////////////////////////
class CSunClockDocument : public CEikDocument
	{
public:
	static CSunClockDocument* NewL(CEikApplication& aApp);
	CSunClockDocument(CEikApplication& aApp);
	void ConstructL();
private: 
	           // Inherited from CEikDocument
	CEikAppUi* CreateAppUiL();
	};



////////////////////////////////////////////////////////////////////////
//
// CDateDlg eli DST-p�iv�m��rien asetusdialogi
//
////////////////////////////////////////////////////////////////////////

class CDateDlg : public CEikDialog {
public: 
	 CDateDlg(CSunClockAppView* aModel); 
     void PreLayoutDynInitL(); //Combo box initialisation. 
     TBool OkToExitL(TInt aControlId); //Adding items and retrieving the value. 

private:
	 CSunClockAppView* iAppView;
};

#endif



