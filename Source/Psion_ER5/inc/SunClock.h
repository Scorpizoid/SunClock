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

		// Näihin tallennetaan DST:n alku- ja
		// loppupäivät
	TTime iDate;
	TTime iDate2;
	void SetDay(TTime aDate, TTime aDate2, TInt aChoice);

		// Vuosiluvun tallentaminen INI-tiedostoon
	TInt iYearNo;
	void SetYearL(TInt aYear);

		// Päivien numerot (vuoden alusta),
		// jotka aloittavat ja lopettavat
		// DST:n
	TInt iDayDSTStarts;
	TInt iDayDSTEnds;

		// Kartan paikka, joka voi muuttua kun käyttäjä
		// valitsee toisen maanosan
	TInt iMapPosition;

		// Kumman käyttäjä valitsee - käytetäänkö kesäaikaa vai ei
	TInt iChoice;

		// Painettiinko Esc-nappia DST-dialogin aikana?
	TInt iEscWasPressed;

		// Tämä tarkistaa, onko ohjelma käytössä ensimmäistä kertaa,
		// ja jos on, se poistaa vanhan INI-tiedoston, joka aiheuttaa
		// KERN-EXEC 3 -virheen, koska siinä on eri määrä tietueita
		// kuin uudessa INI-tiedostossa.
	TInt iVersionNumber;

		// INI-tiedoston käsittelyyn liittyvät
		// funktiot
    void ExternalizeL(RWriteStream& aOut) const;
    void InternalizeL(RReadStream& aIn);
    void SaveL();

		// Palauttaa INI-tiedoston ID:n
    TStreamId StoreL( CStreamStore& aStore) const;
		// ?
	void RestoreL(const CStreamStore& aStore, TStreamId aId);

		// Auringonvaloaikaan liittyvät muuttujat
	RWorldServer    iWS; 
    TWorldId		iWorldID;   
    TTime			iCurrentTime;   
    TInt			iErr2;   
	TTime			iSunrise;
	TTime			iSunset;
	TCityData		cityData;

		// ja funktio
	void RSunlight();


		// Nämä muuttujat koskevat napapiirien
		// ja kääntöpiirien/ ekvaattorin näyttämistä.
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
				// Käsittelee ohjelman joutumista taka- tai etualalle
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
// CDateDlg eli DST-päivämäärien asetusdialogi
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



