// SunClock_CSunClockAppUi.cpp
// ----------------------------

////////////////////////////////////////////////////////////////////////
//
// Source file for the implementation of the 
// application UI class - CSunClockAppUi
//
////////////////////////////////////////////////////////////////////////

#include "SunClock.h"

//             The second phase constructor of the application UI class.
//             The application UI creates and owns the one and only view.
// 
void CSunClockAppUi::ConstructL()
{
	           // BaseConstructL() completes the UI framework's
	           // construction of the App UI.
    BaseConstructL();
	           // Create the single application view in which to
	           // draw the text "Hello World!", passing into it
	           // the rectangle available to it.
	iAppView = CSunClockAppView::NewL(ClientRect());

				// v1.01
	iAppView->iVersionNumber = 1;

				// INI-tiedoston muuttujat
	_LIT(KSunClockIniFileName,"c:\\system\\data\\sunclock.ini");
    TFileName iniFileName=static_cast<TFileName>(KSunClockIniFileName);
    RFs fs;

				// Alustetaan muuttuja. T‰t‰ muuttujaa muutetaan vain silloin,
				// kun DST-dialogissa painetaan Close-nappia. Jos k‰ytt‰j‰ painaa
				// Esc-nappia, muuttujaa ei muuteta, vaan se pysyy nollana, 
				// ja t‰m‰ vaikuttaa HandleForegroundEventL():n ehtolauseissa.
	iAppView->iEscWasPressed = 0;

				// K‰ynnistett‰ess‰ ohjelmaa, avataan INI-tiedosto, tai jos sit‰
				// ei ole, luodaan se (OpenL()-funktio)
	/////////////////////////////////////////////////////////////////////////
    fs.Connect();
    CFileStore* store=NULL;
    TRAPD(err,store=CDirectFileStore::OpenL(fs,iniFileName,EFileShareAny));
 	if (err==KErrNone)
    {
        CleanupStack::PushL(store);
        RStoreReadStream inStream;
        inStream.OpenLC(*store,store->Root());
        iAppView->InternalizeL(inStream);
        CleanupStack::PopAndDestroy(2); // inStream + store
    }
    fs.Close();
	/////////////////////////////////////////////////////////////////////////

				// Lis‰t‰‰n AppView pinoon
	AddToStackL(iAppView);
}


//             The app Ui owns the two views and is,
//             therefore, responsible for destroying them
//
CSunClockAppUi::~CSunClockAppUi()
	{
	delete iAppView;
	}


//             Called by the UI framework when a command has been issued.
//             In this app, a command can originate through a 
//             hot-key press or by selection of a menu item or by 
//             pressing a CBA button.
//             The command Ids are defined in the .hrh file
//             and are 'connected' to the hot-key and menu item in the
//             resource file.
//             Note that the EEikCmdExit is defined by the UI
//             framework and is pulled in by including eikon.hrh
//
void CSunClockAppUi::HandleCommandL(TInt aCommand)
	{
	          // UI environment
	CEikonEnv* eikonEnv=CEikonEnv::Static();


			// Oletusarvoisesti Esci‰ ei ole painettu, joten
			// alustetaan muuttuja siten
	iAppView->iEscWasPressed = 0;


			// About-dialogin otsikko...
	_LIT(KAboutDialogTitle,"SunClock v1.00"); 

			// ja teksti
	_LIT(KAboutDialogText,"An EPOC freeware app\nby Sami Vuori.\nhttp://www.symbioosi.net\n"); 

			// Informaatiodialogin alustus
	CEikDialog* infoDialog = new (ELeave) CEikInfoDialog(KAboutDialogTitle,KAboutDialogText);

			// T‰ll‰ muuttujalla tiedet‰‰n, onko dialogi toteutettu vai ei.
			// Tarvitaan, koska ExecuteLD tuhoaa infoDialog-muuttujan, ja
			// jos ko. funktiota ei k‰ytet‰, meid‰n t‰ytyy tuhota infoDialog
			// itse
	TInt dialogRun = 0;
			// N‰m‰ ovat silt‰ varalta, jos k‰ytt‰j‰ haluaa m‰‰ritt‰‰
			// kes‰ajan myˆhemmin Menun kautta.
	_LIT(KSunClockIniFileName,"c:\\system\\data\\sunclock.ini");
	TFileName iniFileName=static_cast<TFileName>(KSunClockIniFileName);
	RFs fs;
	CFileStore* store=NULL;

	switch (aCommand)
		{
		// Menu -> Tools -> Set summer time
	case ESunClockSetSummerTime:

			// M‰‰ritet‰‰n Menun kautta kes‰aika.
		/////////////////////////////////////////////////////////////////////////////
		fs.Connect();
		User::LeaveIfError(fs.Delete(KSunClockIniFileName));
				// Poistetaan vanha INI-tiedosto
		TRAPD(err,store=CDirectFileStore::OpenL(fs,iniFileName,EFileShareAny));
 		if (err==KErrNone)
		{
			CleanupStack::PushL(store);
			RStoreReadStream inStream;
			inStream.OpenLC(*store,store->Root());
			iAppView->InternalizeL(inStream);
			CleanupStack::PopAndDestroy(2); // inStream + store
		}
		fs.Close();
		/////////////////////////////////////////////////////////////////////////////

				// Ajetaan dialogi
		LaunchDateDialogL();

				// Tallennetaan dialogin p‰iv‰m‰‰r‰t INI-tiedostoon.
		iAppView->SaveL();

				// Jos k‰ytt‰j‰ painoi Esci‰, n‰ytet‰‰n sanoma...
		if (iAppView->iEscWasPressed == 0)
		{
			eikonEnv->InfoMsg(R_DST_NOT_DEFINED);
		}
				// ...ja jos valinta tapahtui,
				// n‰ytet‰‰n toinen sanoma.
		else
		{
			if (iAppView->iChoice == 0)
			{
				eikonEnv->InfoMsg(R_DST_DEFINED);
			}
			if (iAppView->iChoice == 1)
			{
				eikonEnv->InfoMsg(R_NO_DST);
			}
		}		
		break;

		// Menu -> Help -> About
	case ESunClockAbout:
			// Create and launch Info dialog using static function. 
			// The function returns when the OK button is pressed. 
		infoDialog->ExecuteLD(R_EIK_DIALOG_INFO);
		dialogRun = 1;
		break;
		// Jos k‰ytt‰j‰ valitsee Menusta haluamansa horisontaalisen viivan,
		// n‰ytet‰‰n se ja sen lis‰ksi tallennetaan arvo INI-tiedostoon, jolloin
		// se ladataan kun ohjelma k‰ynnistet‰‰n seuraavan kerran.
	case ECbaButton0:
	case ESunClockViewArcticCircle:
		iAppView->iShowArcticCircle = iAppView->iShowArcticCircle ? 0 : 1;
		iAppView->SaveL();
		iAppView->DrawNow();
		break;
	case ECbaButton1:
	case ESunClockViewTropicOfCancer:
		iAppView->iShowTropicOfCancer = iAppView->iShowTropicOfCancer ? 0 : 1;
		iAppView->SaveL();
		iAppView->DrawNow();
		break;
	case ECbaButton2:
	case ESunClockViewEquator:
		iAppView->iShowEquator = iAppView->iShowEquator ? 0 : 1;
		iAppView->SaveL();
		iAppView->DrawNow();
		break;
	case ECbaButton3:
	case ESunClockViewTropicOfCapricorn:
		iAppView->iShowTropicOfCapricorn = iAppView->iShowTropicOfCapricorn ? 0 : 1;
		iAppView->SaveL();
		iAppView->DrawNow();
		break;
	case ECbaButton4:
	case ESunClockViewAntarcticCircle:
		iAppView->iShowAntarcticCircle = iAppView->iShowAntarcticCircle ? 0 : 1;
		iAppView->SaveL();
		iAppView->DrawNow();
		break;

      // Exit the application. The call is
      // implemented by the UI framework.
	case ECbaButton5:
	case EEikCmdExit:
		iAppView->SaveL();
		delete infoDialog;
		Exit();
		break;
		}
		if (dialogRun == 0)
		{
			delete infoDialog;
		}
	}


// The dialog initialisator
void CSunClockAppUi::LaunchDateDialogL()
{
	// We construct a new dialog which takes the iAppView
	// (the application's view control) as its parameter
	CDateDlg* dialog = new (ELeave) CDateDlg(iAppView);
	dialog->ExecuteLD(R_DATE_EDITOR_TEST_DIALOG);
}




		// Dialogin esit‰yttˆ
void CDateDlg::PreLayoutDynInitL() 
{ 
    //Get a pointer to the control within the dialog. 
    CCoeControl* myControlPtr = this->Control(0); 
    CCoeControl* myControlPtr2 = this->Control(1); 
//    CCoeControl* myControlPtr3 = this->Control(2); 

    //Cast the returned CCoeControl* pointer to the correct type. 
    CEikDateEditor* DSTStart = static_cast<CEikDateEditor*>(myControlPtr);
    CEikDateEditor* DSTEnd = static_cast<CEikDateEditor*>(myControlPtr2);
//    CEikChoiceList* Choices = static_cast<CEikChoiceList*>(myControlPtr3);

				// Asetetaan kalenteridialogin aloitusp‰iv‰m‰‰r‰ksi
				// nykyisen vuoden ensimm‰inen p‰iv‰.
	////////////////////////////////////////////////////////////
	TTime homeTime;
	homeTime.HomeTime();

	TDateTime dateTime = homeTime.DateTime();
	TInt year = dateTime.Year();
	TBuf<30> presetDate;
	_LIT(KYear,"%d0000:000000.000000");
	presetDate.Format(KYear, year);

	homeTime.Set(presetDate);
	
	DSTStart->SetDate(homeTime);
	DSTEnd->SetDate(homeTime);
	////////////////////////////////////////////////////////////

}

		// T‰‰ on viel‰kin aika kryptinen joukko merkkej‰ - t‰ytyy
		// tsekata jos joskus tajuaisi t‰st‰ jotain
CDateDlg::CDateDlg(CSunClockAppView* aAppView)
	: iAppView(aAppView)
{
}


		//Check if OK to exit the dialog 
TBool CDateDlg::OkToExitL(TInt aControlId) 
{ 
		// Control 0 = kes‰ajan alku -kalenteri
		// Control 1 = kes‰ajan loppu -kalenteri
		// Control 2 = "K‰ytet‰‰nkˆ kes‰aikaa"-valinta
	CCoeControl* control = this->Control(0); 
	CCoeControl* control2 = this->Control(1);
	CCoeControl* control3 = this->Control(2);

	CEikDateEditor* dateEditor = static_cast <CEikDateEditor*>(control);
	CEikDateEditor* dateEditor2 = static_cast <CEikDateEditor*>(control2);
	CEikChoiceList* iChoiceList = static_cast <CEikChoiceList*>(control3);


		// Sis‰lt‰‰ valinnan - 0 = kyll‰, 1 = ei
	TInt useSummerTime = iChoiceList->CurrentItem();

		// Otetaan p‰iv‰m‰‰r‰arvot dialogista...
	TTime day;
    TTime day2;
	day = dateEditor->Date(); 
	day2 = dateEditor2->Date();

		// ...ja vied‰‰n ne AppView:n funktiolle, joka k‰sittelee
		// ne ja tallentaa INI-tiedostoon.
	iAppView->SetDay(day, day2, useSummerTime);

		// Jos k‰ytt‰j‰ on painanut Esc-nappia, t‰m‰ muuttuja ei
		// muutu, jolloin tiedet‰‰n, ett‰ k‰ytt‰j‰lle n‰ytet‰‰n
		// oikea InfoMsg()
	iAppView->iEscWasPressed = 1;
		// Dialogin voi sulkea
	return ETrue;
}


		// K‰sittelee ohjelman joutumista etu- tai taka-alalle.
		// T‰m‰ funktio ajetaan heti k‰ynnistyksess‰ ConstructL-
		// funktion j‰lkeen, joten t‰ss‰ kannattaa olla kaikki
		// p‰iv‰m‰‰r‰nvaihtotarkistukset yms.
void CSunClockAppUi::HandleForegroundEventL(TBool aForeground)
{
		// Call Base class method
//	CEikAppUi::HandleForegroundEventL(aForeground);

		// Tarvitaan InfoMsg:n n‰ytt‰miseen
	CEikonEnv* eikonEnv=CEikonEnv::Static();

		// Alustetaan muuttuja sen vuoksi, ett‰ jos ohjelma on
		// p‰‰ll‰ vuosia putkeen, voidaan olla varmoja siit‰, ett‰
		// menun kautta tapahtuneen kes‰aikojen vaihdon yhteydess‰
		// n‰ytet‰‰n oikea InfoMsg, jos k‰ytt‰j‰ on painanut Esci‰.
	iAppView->iEscWasPressed = 0;

		// Katsotaan, mik‰ vuosi t‰ll‰ hetkell‰ on
	TTime homeTime;
	homeTime.HomeTime();

	TDateTime dateTime = homeTime.DateTime();
	TInt year = dateTime.Year();

		// Kun ohjelma tulee etualalle
	if(aForeground)
	{
		iAppView->	RSunlight();
			// Piirret‰‰n AppView
		iAppView->DrawNow();

	
			// Jos INI-tiedoston vuosiluku ei t‰sm‰‰ t‰m‰nhetkisen vuoden
			// kanssa, tai sit‰ ei ole ollenkaan (eli INI-tiedostoa ei lˆydy)
		if ((iAppView->iYearNo != year) || (!iAppView->iYearNo))
		{
						// Tehd‰‰n INI-tiedosto
			////////////////////////////////////////////////////////////////////////
			_LIT(KSunClockIniFileName,"c:\\system\\data\\sunclock.ini");
			TFileName iniFileName=static_cast<TFileName>(KSunClockIniFileName);

			RFs fs;
			fs.Connect();
			TRAPD(err,User::LeaveIfError(fs.Delete(KSunClockIniFileName)));
			CFileStore* store=NULL;
			TRAP(err,store=CDirectFileStore::OpenL(fs,iniFileName,EFileShareAny));
 			if (err==KErrNone)
			{
				CleanupStack::PushL(store);
				RStoreReadStream inStream;
				inStream.OpenLC(*store,store->Root());
				iAppView->InternalizeL(inStream);
				CleanupStack::PopAndDestroy(2); // inStream + store
			}
			fs.Close();
			////////////////////////////////////////////////////////////////////////

						// Ja ajetaan dialogi
			LaunchDateDialogL();
						// Asetetaan uusi vuosi ja tallennetaan
						// (tallentaa myˆs kes‰aikap‰iv‰m‰‰r‰t)
			iAppView->SetYearL(year);
						// Jos k‰ytt‰j‰ painoi Esci‰, 
						// heitet‰‰n ruudulle sanoma
			if (iAppView->iEscWasPressed == 0)
			{
				eikonEnv->InfoMsg(R_DST_NOT_UPDATED);
			}
						// Jos ei, heitet‰‰n ruudulle toinen
						// sanoma, riippuen valinnasta.
			else
			{
				if (iAppView->iChoice == 0)
				{
					eikonEnv->InfoMsg(R_DST_UPDATED);
				}
				if (iAppView->iChoice == 1)
				{
					eikonEnv->InfoMsg(R_NO_DST);
				}
			}
		}
	}
	else
	{
		// T‰ll‰ er‰‰ ei tehd‰ mit‰‰n
	}
}


		// T‰m‰ funktio k‰sittelee menua. Se n‰ytt‰‰ View-valikossa pieni‰
		// rukseja riippuen siit‰, mitk‰ valinnat k‰ytt‰j‰ on valinnut.
void CSunClockAppUi::DynInitMenuPaneL(TInt aResourceId,CEikMenuPane* aMenuPane)
{
	          // This is the View menu pane. See also the resource file.
			  // Here, we adjust the text to reflect the state of the
	          // menu items in the Tools pane.
	if (aResourceId == R_SUNCLOCK_VIEW_MENU)
	{
				// Jos k‰ytt‰j‰ haluaa, ett‰ pohjoinen napapiiri n‰ytet‰‰n,
				// ruksataan kyseinen menun rivi.
		if (iAppView->iShowArcticCircle == 1)
		{
			aMenuPane->SetItemButtonState(ESunClockViewArcticCircle,EEikMenuItemSymbolOn);
		}
		if (iAppView->iShowArcticCircle == 0)
		{
			aMenuPane->SetItemButtonState(ESunClockViewArcticCircle,EEikMenuItemSymbolIndeterminate);
		}

				// Kravun k‰‰ntˆpiiri
		if (iAppView->iShowTropicOfCancer == 1)
		{
			aMenuPane->SetItemButtonState(ESunClockViewTropicOfCancer,EEikMenuItemSymbolOn);
		}
		if (iAppView->iShowTropicOfCancer == 0)
		{
			aMenuPane->SetItemButtonState(ESunClockViewTropicOfCancer,EEikMenuItemSymbolIndeterminate);
		}

				// Ekvaattori
		if (iAppView->iShowEquator == 1)
		{
			aMenuPane->SetItemButtonState(ESunClockViewEquator,EEikMenuItemSymbolOn);
		}
		if (iAppView->iShowEquator == 0)
		{
			aMenuPane->SetItemButtonState(ESunClockViewEquator,EEikMenuItemSymbolIndeterminate);
		}

				// Kauriin k‰‰ntˆpiiri
		if (iAppView->iShowTropicOfCapricorn == 1)
		{
			aMenuPane->SetItemButtonState(ESunClockViewTropicOfCapricorn,EEikMenuItemSymbolOn);
		}
		if (iAppView->iShowTropicOfCapricorn == 0)
		{
			aMenuPane->SetItemButtonState(ESunClockViewTropicOfCapricorn,EEikMenuItemSymbolIndeterminate);
		}

				// Etel‰inen napapiiri
		if (iAppView->iShowAntarcticCircle == 1)
		{
			aMenuPane->SetItemButtonState(ESunClockViewAntarcticCircle,EEikMenuItemSymbolOn);
		}
		if (iAppView->iShowAntarcticCircle == 0)
		{
			aMenuPane->SetItemButtonState(ESunClockViewAntarcticCircle,EEikMenuItemSymbolIndeterminate);
		}

	}
}
