// SunClock_CSunClockAppView.cpp
// ------------------------------
//

////////////////////////////////////////////////////////////////////////
//
// Source file for the implementation of the 
// application view class - CSunClockAppView
//
////////////////////////////////////////////////////////////////////////

#include "SunClock.h"

//
//             Constructor for the view.
//
CSunClockAppView::CSunClockAppView()
{
}


//             Static NewL() function to start the standard two
//             phase construction.
CSunClockAppView* CSunClockAppView::NewL(const TRect& aRect)
{
	CSunClockAppView* self = new(ELeave) CSunClockAppView();
	CleanupStack::PushL(self);
	self->ConstructL(aRect);
	CleanupStack::Pop();
	return self;
}


//
//             Destructor for the view.
//
CSunClockAppView::~CSunClockAppView()
{
	// Tuhotaan  sekunnin v‰lein toistuva ajastin
    if(iPeriodicTimer)
    {
        // Calling Cancel without checking if the timer is active is safe
        iPeriodicTimer->Cancel();
    }
    delete iPeriodicTimer;

	// Tuhotaan 3,6 minuutin v‰lein toistuva ajastin
    if(iPeriodicTimer2)
    {
        // Calling Cancel without checking if the timer is active is safe
        iPeriodicTimer2->Cancel();
    }
    delete iPeriodicTimer2;

	// Tuhotaan 2 tunnin v‰lein toistuva ajastin
    if(iPeriodicTimer3)
    {
        // Calling Cancel without checking if the timer is active is safe
        iPeriodicTimer3->Cancel();
    }
    delete iPeriodicTimer3;
}


//             Second phase construction.
//
void CSunClockAppView::ConstructL(const TRect& aRect)
{

			// Karttakuvan sijainti; k‰ytet‰‰n k‰ytt‰j‰n valitseman
			// alueen keskitt‰miseen.
//	iMapPosition = 0;

	// Ajetaan INI-tiedoston lataaja, josta saamme
	// CSunClockAppViewin k‰yttˆˆn iDayDSTStarts- ja
	// iDayDSTEnds-arvot

	//////////////////////////////////////////////////////////////////////

	_LIT(KSunClockIniFileName,"c:\\system\\data\\sunclock.ini");
    TFileName iniFileName=static_cast<TFileName>(KSunClockIniFileName);

    RFs fs;
    fs.Connect();
    CFileStore* store=NULL;
    TRAPD(err,store=CDirectFileStore::OpenL(fs,iniFileName,EFileShareAny));
 	if (err==KErrNone)
	{
        CleanupStack::PushL(store);
        RStoreReadStream inStream;
        inStream.OpenLC(*store,store->Root());
        InternalizeL(inStream);
        CleanupStack::PopAndDestroy(2); // inStream + store
     }
     fs.Close();

	//////////////////////////////////////////////////////////////////////

	           // Control is a window owning control
	CreateWindowL();
	           // Extent of the control. This is
	           // the whole rectangle available to application.
	           // The rectangle is passed to us from the application UI.
	SetRect(aRect);
			   // At this stage, the control is ready to draw so
	           // we tell the UI framework by activating it.

				// Ajastimen p‰ivitysv‰liksi laitetaan sekunti, mutta piirto
				// tapahtuu vasta sitten, kun jokin muuttuu n‰ytˆll‰. T‰t‰
				// k‰ytet‰‰n vain vasemmalla puolella sijaitsevaan sivupalkkiin.
	TInt KPeriodicTimerInterval1Sec(1000000);

				// T‰m‰ ajastin piirt‰‰ karttaa 3,6 minuutin v‰lein.
	TInt KPeriodicTimerInterval216Sec(216000000);

				// T‰m‰ ajastin piirt‰‰ sivupalkkia 35 minuutin v‰lein (maksimiarvo).
	TInt KPeriodicTimerInterval35Min(2147483646);

				// Initialize the periodic timer.
    iPeriodicTimer = CPeriodic::NewL(CActive::EPriorityIdle);
    iPeriodicTimer2 = CPeriodic::NewL(CActive::EPriorityIdle);
    iPeriodicTimer3 = CPeriodic::NewL(CActive::EPriorityIdle);

				// Start the periodic timer, when ever the time elapsed
				// the PeriodicTimerCallBack() will get called.
				// Notice: The timer will periodically hit the PeriodicTimerCallBack
				// until you cancel the timer by calling iPeriodicTimer->Cancel().
    iPeriodicTimer->Start(
        KPeriodicTimerInterval1Sec, KPeriodicTimerInterval1Sec, 
        TCallBack(PeriodicTimerCallBack, this));

				// 3,6 minuutin v‰lein tapahtuva ajastin
    iPeriodicTimer2->Start(
        KPeriodicTimerInterval216Sec, KPeriodicTimerInterval216Sec, 
        TCallBack(PeriodicTimerCallBack2, this));

				// 2 tunnin v‰lein tapahtuva ajastin
    iPeriodicTimer3->Start(
        KPeriodicTimerInterval35Min, KPeriodicTimerInterval35Min, 
        TCallBack(PeriodicTimerCallBack3, this));

			// Ohjelman k‰ynnistyess‰ lasketaan auringonvalon
			// kesto
	RSunlight();


			// Aktivoidaan AppView
	ActivateL();
}



			// Piirtofunktio - ohjelman t‰rkein osa
void CSunClockAppView::Draw(const TRect& /*aRect*/) const
{
               // Window graphics context
	CWindowGc& gc = SystemGc();
	           // Area in which we shall draw
	TRect      sideBarRect(0,0,63,200);// = Rect();
			   // Font used for drawing text
	const CFont*     fontUsed;
	           // UI environment
	CEikonEnv* eikonEnv = CEikonEnv::Static();


			
				// Yhdistet‰‰n muuttujaan iTime nykyinen aika.
	TTime currentTime;
	currentTime.HomeTime();
				// Selvitet‰‰n, monesko vuoden p‰iv‰ on menossa.
				// Luku alkaa yhdest‰.
	TInt dayNoInYear = currentTime.DayNoInYear();

				// Selvitet‰‰n, monesko kuukauden p‰iv‰ on menossa.
				// Luku alkaa nollasta, joten t‰ytyy lis‰t‰ yksi p‰iv‰.
	TInt dayNoInMonth = currentTime.DayNoInMonth() + 1;

				// Nyt laskemme maskitiedoston sijainnin x-akselilla.
				// Minuutteja on vuorokaudessa 1440, joten otetaan
				// nykyajasta tunnit ja muutetaan ne minuuteiksi.
	TDateTime dateTime = currentTime.DateTime();
	TInt hours = dateTime.Hour();

				// Muutetaan minuuteiksi
	TInt minutes = dateTime.Minute()+hours*60;

				// Tarkistetaan, mik‰ vuosi on menossa
	TInt year = dateTime.Year();

				// TRAP-haaveja varten
	TInt err;

				// Teksti, joka ilmoittaa vuoden
	TBuf<5> iYearText;
	_LIT(KYear,"%d");
	iYearText.Format(KYear, year);

				// T‰ytet‰‰n sivupalkki mustalla v‰rill‰.
	gc.SetBrushColor(KRgbBlack); // change the brush color 
	gc.Clear(sideBarRect); // clear to brush color



				// Asetetaan kyn‰n v‰riksi valkoinen
	gc.SetPenColor(KRgbWhite); // change the pen color 

				// Viivojen pisteet
	TPoint whiteLineStartPoint[10];
	TPoint whiteLineEndPoint[10];


				// Asetetaan pisteiden arvot
	whiteLineStartPoint[0].SetXY(0,0);
	whiteLineStartPoint[1].SetXY(62,0);
	whiteLineStartPoint[2].SetXY(0,199);
	whiteLineStartPoint[3].SetXY(0,1);
	whiteLineStartPoint[4].SetXY(2,2);
	whiteLineStartPoint[5].SetXY(2,3);
	whiteLineStartPoint[6].SetXY(3,46);
	whiteLineStartPoint[7].SetXY(1,157);
	whiteLineStartPoint[8].SetXY(1,158);
	whiteLineStartPoint[9].SetXY(2,91);

	whiteLineEndPoint[0].SetXY(62,0);
	whiteLineEndPoint[1].SetXY(62,199);
	whiteLineEndPoint[2].SetXY(63,199);
	whiteLineEndPoint[3].SetXY(0,199);
	whiteLineEndPoint[4].SetXY(59,2);
	whiteLineEndPoint[5].SetXY(2,155);
	whiteLineEndPoint[6].SetXY(59,46);
	whiteLineEndPoint[7].SetXY(62,157);
	whiteLineEndPoint[8].SetXY(1,199);
	whiteLineEndPoint[9].SetXY(59,91);


			// Ja piirret‰‰n n‰ytˆlle
	TInt i;
	for(i=0;i<=9;i++)
	{
		gc.DrawLine(whiteLineStartPoint[i],whiteLineEndPoint[i]); 
	}


			// Harmaat viivat
	TPoint greyLineStartPoint[6];
	TPoint greyLineEndPoint[6];

	greyLineStartPoint[0].SetXY(3,47);
	greyLineStartPoint[1].SetXY(59,2);
	greyLineStartPoint[2].SetXY(3,92);
	greyLineStartPoint[3].SetXY(61,158);
	greyLineStartPoint[4].SetXY(2,198);
	greyLineStartPoint[5].SetXY(2,155);

	greyLineEndPoint[0].SetXY(59,47);
	greyLineEndPoint[1].SetXY(59,156);
	greyLineEndPoint[2].SetXY(59,92);
	greyLineEndPoint[3].SetXY(61,198);
	greyLineEndPoint[4].SetXY(62,198);
	greyLineEndPoint[5].SetXY(59,155);



	gc.SetPenColor(TRgb(142,142,142)); // change the pen color 

			// Piirret‰‰n
	for(i=0;i<=5;i++)
	{
		gc.DrawLine(greyLineStartPoint[i],greyLineEndPoint[i]); 
	}


				// Alustetaan tekstijonot
	_LIT(KJan,"Jan %d");
	_LIT(KFeb,"Feb %d");
	_LIT(KMar,"Mar %d");
	_LIT(KApr,"Apr %d");
	_LIT(KMay,"May %d");
	_LIT(KJun,"Jun %d");
	_LIT(KJul,"Jul %d");
	_LIT(KAug,"Aug %d");
	_LIT(KSep,"Sep %d");
	_LIT(KOct,"Oct %d");
	_LIT(KNov,"Nov %d");
	_LIT(KDec,"Dec %d");

				// T‰m‰ muuttuja pit‰‰ sis‰ll‰‰n teksti-
				// jonoa, jossa lukee esim. "Oct 28"
	TBuf<7> monthAndDay;


				// Katsotaan, mik‰ kuukausi on menossa
	if (dateTime.Month() == EJanuary) {
		monthAndDay.Format(KJan, dayNoInMonth);
	}

	if (dateTime.Month() == EFebruary) {
		monthAndDay.Format(KFeb, dayNoInMonth);
	}

	if (dateTime.Month() == EMarch) {
		monthAndDay.Format(KMar, dayNoInMonth);
	}

	if (dateTime.Month() == EApril) {
		monthAndDay.Format(KApr, dayNoInMonth);
	}

	if (dateTime.Month() == EMay) {
		monthAndDay.Format(KMay, dayNoInMonth);
	}

	if (dateTime.Month() == EJune) {
		monthAndDay.Format(KJun, dayNoInMonth);
	}

	if (dateTime.Month() == EJuly) {
		monthAndDay.Format(KJul, dayNoInMonth);
	}

	if (dateTime.Month() == EAugust) {
		monthAndDay.Format(KAug, dayNoInMonth);
	}

	if (dateTime.Month() == ESeptember) {
		monthAndDay.Format(KSep, dayNoInMonth);
	}

	if (dateTime.Month() == EOctober) {
		monthAndDay.Format(KOct, dayNoInMonth);
	}

	if (dateTime.Month() == ENovember) {
		monthAndDay.Format(KNov, dayNoInMonth);
	}

	if (dateTime.Month() == EDecember) {
		monthAndDay.Format(KDec, dayNoInMonth);
	}


				// Lasketaan, montako pikseli‰ maskikuvaa t‰ytyy siirt‰‰,
				// jotta se on oikeassa kohdassa oikeaan aikaan.
	TReal calculation = minutes/3.6;
	TReal result;
	Math::Round(result,calculation,0);
	TInt maskPosition_X = result + iMapPosition;


				// Seuraavassa tehd‰‰n offscreen-kuva, johon piirret‰‰n
				// senhetkinen maskipolygoni.

				// create a bitmap to be used off-screen 
	CFbsBitmap* offScreenBitmap = new (ELeave) CFbsBitmap(); 
	User::LeaveIfError(offScreenBitmap->Create(TSize(400,200),EColor256)); 
	CleanupStack::PushL(offScreenBitmap); 

				// create an off-screen device and context 
	CGraphicsContext* bitmapContext=NULL; 
	CFbsBitmapDevice* bitmapDevice = CFbsBitmapDevice::NewL(offScreenBitmap); 
	CleanupStack::PushL(bitmapDevice); 
	User::LeaveIfError(bitmapDevice->CreateContext(bitmapContext)); 
	CleanupStack::PushL(bitmapContext); 





					// T‰m‰ ehtolause toteutetaan, jos k‰ytt‰j‰ on painanut
					// DST-valikosta valinnan "Kyll‰". Jos ei, niin DST-aikaa
					// ei k‰ytet‰ ohjelmassa.
	if (iChoice == 0)
	{
					// T‰ss‰ tarkistetaan, k‰ytet‰‰nkˆ t‰ll‰ hetkell‰ kes‰aikaa
					// Jos k‰ytˆss‰ on kes‰aika, siirret‰‰n maskia tunnilla taaksep‰in
		if((dayNoInYear >= iDayDSTStarts) && (dayNoInYear <= iDayDSTEnds)) {
			maskPosition_X = maskPosition_X - 17;
		}
	}


				// Alustetaan pisteet, joilla rakennetaan maskikuvat.
				// N‰m‰ olisivat voineet olla myˆs taulukon muodossa,
				// mutta menkˆˆt nyt n‰in
	TPoint point1;
	TPoint point2;
	TPoint point3;
	TPoint point4;
	TPoint point5;
	TPoint point6;
	TPoint point7;
	TPoint point8;
	TPoint point9;
	TPoint point10;
	TPoint point11;
	TPoint point12;
	TPoint point13;
	TPoint point14;
	TPoint point15;
	TPoint point16;
	TPoint point17;
	TPoint point18;
	TPoint point19;
	TPoint point20;
	TPoint point21;
	TPoint point22;
	TPoint point23;
	TPoint point24;
	TPoint point25;
	TPoint point26;
	TPoint point27;
	TPoint point28;
	TPoint point29;
	TPoint point30;
	TPoint point31;
	TPoint point32;
	TPoint point33;
	TPoint point34;
	TPoint point35;
	TPoint point36;

	TPoint pointa;
	TPoint pointb;
	TPoint pointc;
	TPoint pointd;
	TPoint pointe;
	TPoint pointf;
	TPoint pointg;
	TPoint pointh;
	TPoint pointi;
	TPoint pointj;
	TPoint pointk;
	TPoint pointl;
	TPoint pointm;
	TPoint pointn;
	TPoint pointo;
	TPoint pointp;
	TPoint pointq;
	TPoint pointr;
	TPoint points;
	TPoint pointt;
	TPoint pointu;
	TPoint pointv;
	TPoint pointw;
	TPoint pointx;
	TPoint pointy;
	TPoint pointz;
	TPoint pointaa;
	TPoint pointab;
	TPoint pointac;
	TPoint pointad;
	TPoint pointae;
	TPoint pointaf;
	TPoint pointag;
	TPoint pointah;
	TPoint pointai;
	TPoint pointaj;


				// Asetetaan ensimm‰iset arvot, ettei tule erroria
	point1.SetXY(0,0);
	point2.SetXY(0,0);
	point3.SetXY(0,0);
	point4.SetXY(0,0);
	point5.SetXY(0,0);
	point6.SetXY(0,0);
	point7.SetXY(0,0);
	point8.SetXY(0,0);
	point9.SetXY(0,0);
	point10.SetXY(0,0);
	point11.SetXY(0,0);
	point12.SetXY(0,0);
	point13.SetXY(0,0);
	point14.SetXY(0,0);
	point15.SetXY(0,0);
	point16.SetXY(0,0);
	point17.SetXY(0,0);
	point18.SetXY(0,0);
	point19.SetXY(0,0);
	point20.SetXY(0,0);
	point21.SetXY(0,0);
	point22.SetXY(0,0);
	point23.SetXY(0,0);
	point24.SetXY(0,0);
	point25.SetXY(0,0);
	point26.SetXY(0,0);
	point27.SetXY(0,0);
	point28.SetXY(0,0);
	point29.SetXY(0,0);
	point30.SetXY(0,0);
	point31.SetXY(0,0);
	point32.SetXY(0,0);
	point33.SetXY(0,0);
	point34.SetXY(0,0);
	point35.SetXY(0,0);
	point36.SetXY(0,0);


	// January

	if ((dayNoInYear >= 1) && (dayNoInYear <= 31)) {

	 // Kun halutaan siirt‰‰ varjoa oikealle, v‰hennet‰‰n maskPosition_X:‰‰.
		maskPosition_X = maskPosition_X - 201;//235;
		point1.SetXY(0-maskPosition_X,181);
		point2.SetXY(22-maskPosition_X,179);
		point3.SetXY(39-maskPosition_X,174);
		point4.SetXY(59-maskPosition_X,163);
		point5.SetXY(72-maskPosition_X,154);
		point6.SetXY(80-maskPosition_X,142);
		point7.SetXY(90-maskPosition_X,122);
		point8.SetXY(95-maskPosition_X,107);
		point9.SetXY(101-maskPosition_X,91);
		point10.SetXY(106-maskPosition_X,78);
		point11.SetXY(113-maskPosition_X,64);
		point12.SetXY(122-maskPosition_X,53);
		point13.SetXY(137-maskPosition_X,40);
		point14.SetXY(148-maskPosition_X,33);
		point15.SetXY(161-maskPosition_X,28);
		point16.SetXY(180-maskPosition_X,25);
		point17.SetXY(200-maskPosition_X,24);
		point18.SetXY(220-maskPosition_X,25);
		point19.SetXY(239-maskPosition_X,28);
		point20.SetXY(252-maskPosition_X,33);
		point21.SetXY(263-maskPosition_X,40);
		point22.SetXY(278-maskPosition_X,53);
		point23.SetXY(287-maskPosition_X,64);
		point24.SetXY(294-maskPosition_X,78);
		point25.SetXY(299-maskPosition_X,91);
		point26.SetXY(305-maskPosition_X,107);
		point27.SetXY(310-maskPosition_X,122);
		point28.SetXY(320-maskPosition_X,142);
		point29.SetXY(328-maskPosition_X,154);
		point30.SetXY(341-maskPosition_X,163);
		point31.SetXY(361-maskPosition_X,174);
		point32.SetXY(378-maskPosition_X,179);
		point33.SetXY(399-maskPosition_X,181);
		point34.SetXY(399-maskPosition_X,199);
		point35.SetXY(0-maskPosition_X,199);
		point36.SetXY(0-maskPosition_X,181);
	}


	// February

	if ((dayNoInYear >= 32) && (dayNoInYear <= 59)) {
		maskPosition_X = maskPosition_X - 204; // 239
		point1.SetXY(0-maskPosition_X,184);
		point2.SetXY(30-maskPosition_X,183);
		point3.SetXY(48-maskPosition_X,180);
		point4.SetXY(65-maskPosition_X,173);
		point5.SetXY(78-maskPosition_X,160);
		point6.SetXY(88-maskPosition_X,140);
		point7.SetXY(94-maskPosition_X,119);
		point8.SetXY(98-maskPosition_X,101);
		point9.SetXY(102-maskPosition_X,83);
		point10.SetXY(107-maskPosition_X,67);
		point11.SetXY(112-maskPosition_X,54);
		point12.SetXY(118-maskPosition_X,41);
		point13.SetXY(125-maskPosition_X,31);
		point14.SetXY(133-maskPosition_X,25);
		point15.SetXY(149-maskPosition_X,19);
		point16.SetXY(169-maskPosition_X,15);
		point17.SetXY(199-maskPosition_X,13);
		point18.SetXY(229-maskPosition_X,15);
		point19.SetXY(249-maskPosition_X,19);
		point20.SetXY(265-maskPosition_X,25);
		point21.SetXY(273-maskPosition_X,31);
		point22.SetXY(280-maskPosition_X,41);
		point23.SetXY(286-maskPosition_X,54);
		point24.SetXY(291-maskPosition_X,67);
		point25.SetXY(296-maskPosition_X,83);
		point26.SetXY(300-maskPosition_X,101);
		point27.SetXY(304-maskPosition_X,119);
		point28.SetXY(310-maskPosition_X,140);
		point29.SetXY(320-maskPosition_X,160);
		point30.SetXY(333-maskPosition_X,173);
		point31.SetXY(350-maskPosition_X,180);
		point32.SetXY(368-maskPosition_X,183);
		point33.SetXY(399-maskPosition_X,184);
		point34.SetXY(399-maskPosition_X,199);
		point35.SetXY(0-maskPosition_X,199);
		point36.SetXY(0-maskPosition_X,184);
	}


	// March

	if ((dayNoInYear >= 60) && (dayNoInYear <= 90)) {
		maskPosition_X = maskPosition_X - 207;//244;
		point1.SetXY(0-maskPosition_X,198);
		point2.SetXY(46-maskPosition_X,196);
		point3.SetXY(72-maskPosition_X,194);
		point4.SetXY(83-maskPosition_X,189);
		point5.SetXY(89-maskPosition_X,183);
		point6.SetXY(91-maskPosition_X,169);
		point7.SetXY(98-maskPosition_X,29);
		point8.SetXY(102-maskPosition_X,16);
		point9.SetXY(107-maskPosition_X,10);
		point10.SetXY(116-maskPosition_X,6);
		point11.SetXY(141-maskPosition_X,3);
		point12.SetXY(248-maskPosition_X,3);
		point13.SetXY(273-maskPosition_X,6);
		point14.SetXY(282-maskPosition_X,10);
		point15.SetXY(287-maskPosition_X,16);
		point16.SetXY(291-maskPosition_X,29);
		point17.SetXY(298-maskPosition_X,169);
		point18.SetXY(300-maskPosition_X,183);
		point19.SetXY(306-maskPosition_X,189);
		point20.SetXY(317-maskPosition_X,194);
		point21.SetXY(343-maskPosition_X,196);
		point22.SetXY(389-maskPosition_X,198);
		point23.SetXY(399-maskPosition_X,198);
		point24.SetXY(399-maskPosition_X,198);
		point25.SetXY(399-maskPosition_X,198);
		point26.SetXY(399-maskPosition_X,198);
		point27.SetXY(399-maskPosition_X,198);
		point28.SetXY(399-maskPosition_X,198);
		point29.SetXY(399-maskPosition_X,198);
		point30.SetXY(399-maskPosition_X,198);
		point31.SetXY(399-maskPosition_X,198);
		point32.SetXY(399-maskPosition_X,198);
		point33.SetXY(399-maskPosition_X,198);
		point34.SetXY(399-maskPosition_X,199);
		point35.SetXY(0-maskPosition_X,199);
		point36.SetXY(0-maskPosition_X,198);
	}



	// April

	if ((dayNoInYear >= 91) && (dayNoInYear <= 120)) {
		maskPosition_X = maskPosition_X - 199;//239;
		point1.SetXY(0-maskPosition_X,11);
		point2.SetXY(39-maskPosition_X,14);
		point3.SetXY(56-maskPosition_X,18);
		point4.SetXY(70-maskPosition_X,24);
		point5.SetXY(79-maskPosition_X,32);
		point6.SetXY(86-maskPosition_X,42);
		point7.SetXY(92-maskPosition_X,56);
		point8.SetXY(108-maskPosition_X,140);
		point9.SetXY(116-maskPosition_X,157);
		point10.SetXY(126-maskPosition_X,172);
		point11.SetXY(139-maskPosition_X,181);
		point12.SetXY(152-maskPosition_X,186);
		point13.SetXY(172-maskPosition_X,188);
		point14.SetXY(229-maskPosition_X,188);
		point15.SetXY(249-maskPosition_X,186);
		point16.SetXY(262-maskPosition_X,181);
		point17.SetXY(275-maskPosition_X,172);
		point18.SetXY(285-maskPosition_X,157);
		point19.SetXY(293-maskPosition_X,140);
		point20.SetXY(309-maskPosition_X,56);
		point21.SetXY(315-maskPosition_X,42);
		point22.SetXY(322-maskPosition_X,32);
		point23.SetXY(331-maskPosition_X,24);
		point24.SetXY(345-maskPosition_X,18);
		point25.SetXY(362-maskPosition_X,14);
		point26.SetXY(399-maskPosition_X,11);
		point27.SetXY(399-maskPosition_X,11);
		point28.SetXY(399-maskPosition_X,11);
		point29.SetXY(399-maskPosition_X,11);
		point30.SetXY(399-maskPosition_X,11);
		point31.SetXY(399-maskPosition_X,11);
		point32.SetXY(399-maskPosition_X,11);
		point33.SetXY(399-maskPosition_X,11);
		point34.SetXY(399-maskPosition_X,0);
		point35.SetXY(0-maskPosition_X,0);
		point36.SetXY(0-maskPosition_X,11);
	}



	// May

	if ((dayNoInYear >= 121) && (dayNoInYear <= 150)) {
		maskPosition_X = maskPosition_X - 201;//240;
		point1.SetXY(0-maskPosition_X,20);
		point2.SetXY(30-maskPosition_X,23);
		point3.SetXY(51-maskPosition_X,30);
		point4.SetXY(68-maskPosition_X,42);
		point5.SetXY(79-maskPosition_X,60);
		point6.SetXY(90-maskPosition_X,82);
		point7.SetXY(97-maskPosition_X,100);
		point8.SetXY(102-maskPosition_X,116);
		point9.SetXY(109-maskPosition_X,132);
		point10.SetXY(117-maskPosition_X,145);
		point11.SetXY(126-maskPosition_X,156);
		point12.SetXY(136-maskPosition_X,165);
		point13.SetXY(146-maskPosition_X,170);
		point14.SetXY(160-maskPosition_X,175);
		point15.SetXY(178-maskPosition_X,178);
		point16.SetXY(216-maskPosition_X,178);
		point17.SetXY(234-maskPosition_X,175);
		point18.SetXY(248-maskPosition_X,170);
		point19.SetXY(258-maskPosition_X,165);
		point20.SetXY(268-maskPosition_X,156);
		point21.SetXY(277-maskPosition_X,145);
		point22.SetXY(285-maskPosition_X,132);
		point23.SetXY(292-maskPosition_X,116);
		point24.SetXY(297-maskPosition_X,100);
		point25.SetXY(304-maskPosition_X,82);
		point26.SetXY(315-maskPosition_X,60);
		point27.SetXY(326-maskPosition_X,42);
		point28.SetXY(343-maskPosition_X,30);
		point29.SetXY(364-maskPosition_X,23);
		point30.SetXY(394-maskPosition_X,20);
		point31.SetXY(399-maskPosition_X,20);
		point32.SetXY(399-maskPosition_X,20);
		point33.SetXY(399-maskPosition_X,20);
		point34.SetXY(399-maskPosition_X,0);
		point35.SetXY(0-maskPosition_X,0);
		point36.SetXY(0-maskPosition_X,20);
	}




	// June

	if ((dayNoInYear >= 151) && (dayNoInYear <= 180)) {
		maskPosition_X = maskPosition_X - 198;// 234;
		point1.SetXY(0-maskPosition_X,25);
		point2.SetXY(30-maskPosition_X,28);
		point3.SetXY(46-maskPosition_X,32);
		point4.SetXY(64-maskPosition_X,40);
		point5.SetXY(77-maskPosition_X,50);
		point6.SetXY(87-maskPosition_X,68);
		point7.SetXY(112-maskPosition_X,122);
		point8.SetXY(121-maskPosition_X,137);
		point9.SetXY(132-maskPosition_X,151);
		point10.SetXY(141-maskPosition_X,159);
		point11.SetXY(156-maskPosition_X,167);
		point12.SetXY(172-maskPosition_X,172);
		point13.SetXY(187-maskPosition_X,174);
		point14.SetXY(216-maskPosition_X,174);
		point15.SetXY(231-maskPosition_X,172);
		point16.SetXY(247-maskPosition_X,167);
		point17.SetXY(262-maskPosition_X,159);
		point18.SetXY(271-maskPosition_X,151);
		point19.SetXY(282-maskPosition_X,137);
		point20.SetXY(291-maskPosition_X,122);
		point21.SetXY(316-maskPosition_X,68);
		point22.SetXY(326-maskPosition_X,50);
		point23.SetXY(339-maskPosition_X,40);
		point24.SetXY(357-maskPosition_X,32);
		point25.SetXY(373-maskPosition_X,28);
		point26.SetXY(399-maskPosition_X,25);
		point27.SetXY(399-maskPosition_X,25);
		point28.SetXY(399-maskPosition_X,25);
		point29.SetXY(399-maskPosition_X,25);
		point30.SetXY(399-maskPosition_X,25);
		point31.SetXY(399-maskPosition_X,25);
		point32.SetXY(399-maskPosition_X,25);
		point33.SetXY(399-maskPosition_X,20);
		point34.SetXY(399-maskPosition_X,0);
		point35.SetXY(0-maskPosition_X,0);
		point36.SetXY(0-maskPosition_X,25);
	}



	// July

	if ((dayNoInYear >= 181) && (dayNoInYear <= 210)) {
		maskPosition_X = maskPosition_X - 202;//241;
		point1.SetXY(0-maskPosition_X,24);
		point2.SetXY(26-maskPosition_X,25);
		point3.SetXY(45-maskPosition_X,30);
		point4.SetXY(61-maskPosition_X,38);
		point5.SetXY(69-maskPosition_X,44);
		point6.SetXY(78-maskPosition_X,55);
		point7.SetXY(87-maskPosition_X,72);
		point8.SetXY(100-maskPosition_X,102);
		point9.SetXY(114-maskPosition_X,133);
		point10.SetXY(119-maskPosition_X,143);
		point11.SetXY(124-maskPosition_X,149);
		point12.SetXY(133-maskPosition_X,158);
		point13.SetXY(141-maskPosition_X,164);
		point14.SetXY(155-maskPosition_X,170);
		point15.SetXY(175-maskPosition_X,175);
		point16.SetXY(195-maskPosition_X,177);
		point17.SetXY(202-maskPosition_X,177);
		point18.SetXY(222-maskPosition_X,175);
		point19.SetXY(242-maskPosition_X,170);
		point20.SetXY(256-maskPosition_X,164);
		point21.SetXY(264-maskPosition_X,158);
		point22.SetXY(273-maskPosition_X,149);
		point23.SetXY(278-maskPosition_X,143);
		point24.SetXY(283-maskPosition_X,133);
		point25.SetXY(297-maskPosition_X,102);
		point26.SetXY(310-maskPosition_X,72);
		point27.SetXY(319-maskPosition_X,55);
		point28.SetXY(328-maskPosition_X,44);
		point29.SetXY(336-maskPosition_X,38);
		point30.SetXY(352-maskPosition_X,30);
		point31.SetXY(371-maskPosition_X,25);
		point32.SetXY(397-maskPosition_X,24);
		point33.SetXY(399-maskPosition_X,24);
		point34.SetXY(399-maskPosition_X,0);
		point35.SetXY(0-maskPosition_X,0);
		point36.SetXY(0-maskPosition_X,24);
	}


	// August

	if ((dayNoInYear >= 211) && (dayNoInYear <= 240)) {
		maskPosition_X = maskPosition_X - 203;//240;
		point1.SetXY(0-maskPosition_X,14);
		point2.SetXY(31-maskPosition_X,16);
		point3.SetXY(49-maskPosition_X,20);
		point4.SetXY(63-maskPosition_X,27);
		point5.SetXY(72-maskPosition_X,34);
		point6.SetXY(81-maskPosition_X,46);
		point7.SetXY(86-maskPosition_X,60);
		point8.SetXY(105-maskPosition_X,130);
		point9.SetXY(113-maskPosition_X,149);
		point10.SetXY(119-maskPosition_X,160);
		point11.SetXY(124-maskPosition_X,166);
		point12.SetXY(134-maskPosition_X,173);
		point13.SetXY(149-maskPosition_X,180);
		point14.SetXY(168-maskPosition_X,184);
		point15.SetXY(181-maskPosition_X,185);
		point16.SetXY(215-maskPosition_X,185);
		point17.SetXY(228-maskPosition_X,184);
		point18.SetXY(247-maskPosition_X,180);
		point19.SetXY(262-maskPosition_X,173);
		point20.SetXY(272-maskPosition_X,166);
		point21.SetXY(277-maskPosition_X,160);
		point22.SetXY(283-maskPosition_X,149);
		point23.SetXY(291-maskPosition_X,130);
		point24.SetXY(310-maskPosition_X,60);
		point25.SetXY(315-maskPosition_X,46);
		point26.SetXY(324-maskPosition_X,34);
		point27.SetXY(333-maskPosition_X,27);
		point28.SetXY(347-maskPosition_X,20);
		point29.SetXY(365-maskPosition_X,16);
		point30.SetXY(396-maskPosition_X,14);
		point31.SetXY(399-maskPosition_X,14);
		point32.SetXY(399-maskPosition_X,14);
		point33.SetXY(399-maskPosition_X,14);
		point34.SetXY(399-maskPosition_X,0);
		point35.SetXY(0-maskPosition_X,0);
		point36.SetXY(0-maskPosition_X,14);
	}


	// September

	if ((dayNoInYear >= 241) && (dayNoInYear <= 270)) {
		maskPosition_X = maskPosition_X - 196;//237;
		point1.SetXY(0-maskPosition_X,2);
		point2.SetXY(66-maskPosition_X,5);
		point3.SetXY(81-maskPosition_X,8);
		point4.SetXY(90-maskPosition_X,12);
		point5.SetXY(95-maskPosition_X,21);
		point6.SetXY(98-maskPosition_X,31);
		point7.SetXY(106-maskPosition_X,164);
		point8.SetXY(110-maskPosition_X,180);
		point9.SetXY(116-maskPosition_X,187);
		point10.SetXY(125-maskPosition_X,192);
		point11.SetXY(140-maskPosition_X,195);
		point12.SetXY(148-maskPosition_X,196);
		point13.SetXY(256-maskPosition_X,196);
		point14.SetXY(264-maskPosition_X,195);
		point15.SetXY(279-maskPosition_X,192);
		point16.SetXY(288-maskPosition_X,187);
		point17.SetXY(294-maskPosition_X,180);
		point18.SetXY(298-maskPosition_X,164);
		point19.SetXY(306-maskPosition_X,31);
		point20.SetXY(309-maskPosition_X,21);
		point21.SetXY(314-maskPosition_X,12);
		point22.SetXY(323-maskPosition_X,8);
		point23.SetXY(338-maskPosition_X,5);
		point24.SetXY(399-maskPosition_X,2);
		point25.SetXY(399-maskPosition_X,2);
		point26.SetXY(399-maskPosition_X,2);
		point27.SetXY(399-maskPosition_X,2);
		point28.SetXY(399-maskPosition_X,2);
		point29.SetXY(399-maskPosition_X,2);
		point30.SetXY(399-maskPosition_X,2);
		point31.SetXY(399-maskPosition_X,2);
		point32.SetXY(399-maskPosition_X,2);
		point33.SetXY(399-maskPosition_X,2);
		point34.SetXY(399-maskPosition_X,0);
		point35.SetXY(0-maskPosition_X,0);
		point36.SetXY(0-maskPosition_X,2);
	}




	// October

	if ((dayNoInYear >= 271) && (dayNoInYear <= 300)) {
		maskPosition_X = maskPosition_X - 199;//241;
		point1.SetXY(0-maskPosition_X,190);
		point2.SetXY(30-maskPosition_X,190);
		point3.SetXY(61-maskPosition_X,182);
		point4.SetXY(75-maskPosition_X,173);
		point5.SetXY(84-maskPosition_X,159);
		point6.SetXY(95-maskPosition_X,117);
		point7.SetXY(104-maskPosition_X,61);
		point8.SetXY(110-maskPosition_X,39);
		point9.SetXY(119-maskPosition_X,27);
		point10.SetXY(133-maskPosition_X,19);
		point11.SetXY(155-maskPosition_X,12);
		point12.SetXY(176-maskPosition_X,11);
		point13.SetXY(218-maskPosition_X,11);
		point14.SetXY(239-maskPosition_X,12);
		point15.SetXY(261-maskPosition_X,19);
		point16.SetXY(275-maskPosition_X,27);
		point17.SetXY(284-maskPosition_X,39);
		point18.SetXY(290-maskPosition_X,61);
		point19.SetXY(299-maskPosition_X,117);
		point20.SetXY(310-maskPosition_X,159);
		point21.SetXY(319-maskPosition_X,173);
		point22.SetXY(333-maskPosition_X,182);
		point23.SetXY(364-maskPosition_X,190);
		point24.SetXY(399-maskPosition_X,190);
		point25.SetXY(399-maskPosition_X,190);
		point26.SetXY(399-maskPosition_X,190);
		point27.SetXY(399-maskPosition_X,190);
		point28.SetXY(399-maskPosition_X,190);
		point29.SetXY(399-maskPosition_X,190);
		point30.SetXY(399-maskPosition_X,190);
		point31.SetXY(399-maskPosition_X,190);
		point32.SetXY(399-maskPosition_X,190);
		point33.SetXY(399-maskPosition_X,190);
		point34.SetXY(399-maskPosition_X,199);
		point35.SetXY(0-maskPosition_X,199);
		point36.SetXY(0-maskPosition_X,190);
	}



	// November

	if ((dayNoInYear >= 301) && (dayNoInYear <= 330)) {
		maskPosition_X = maskPosition_X - 205;//208;
		point1.SetXY(0-maskPosition_X,179);
		point2.SetXY(28-maskPosition_X,176);
		point3.SetXY(45-maskPosition_X,169);
		point4.SetXY(58-maskPosition_X,162);
		point5.SetXY(68-maskPosition_X,151);
		point6.SetXY(75-maskPosition_X,142);
		point7.SetXY(102-maskPosition_X,72);
		point8.SetXY(112-maskPosition_X,52);
		point9.SetXY(120-maskPosition_X,42);
		point10.SetXY(132-maskPosition_X,33);
		point11.SetXY(142-maskPosition_X,28);
		point12.SetXY(160-maskPosition_X,23);
		point13.SetXY(183-maskPosition_X,21);
		point14.SetXY(200-maskPosition_X,21);
		point15.SetXY(223-maskPosition_X,23);
		point16.SetXY(241-maskPosition_X,28);
		point17.SetXY(251-maskPosition_X,33);
		point18.SetXY(263-maskPosition_X,42);
		point19.SetXY(271-maskPosition_X,52);
		point20.SetXY(281-maskPosition_X,72);
		point21.SetXY(308-maskPosition_X,142);
		point22.SetXY(315-maskPosition_X,151);
		point23.SetXY(325-maskPosition_X,162);
		point24.SetXY(338-maskPosition_X,169);
		point25.SetXY(355-maskPosition_X,176);
		point26.SetXY(383-maskPosition_X,179);
		point27.SetXY(399-maskPosition_X,179);
		point28.SetXY(399-maskPosition_X,179);
		point29.SetXY(399-maskPosition_X,179);
		point30.SetXY(399-maskPosition_X,179);
		point31.SetXY(399-maskPosition_X,179);
		point32.SetXY(399-maskPosition_X,179);
		point33.SetXY(399-maskPosition_X,179);
		point34.SetXY(399-maskPosition_X,199);
		point35.SetXY(0-maskPosition_X,199);
		point36.SetXY(0-maskPosition_X,179);
	}


	// December

	if ((dayNoInYear >= 331) && (dayNoInYear <= 366)) {
		maskPosition_X = maskPosition_X - 202;//240;
		point1.SetXY(0-maskPosition_X,174);
		point2.SetXY(23-maskPosition_X,172);
		point3.SetXY(43-maskPosition_X,167);
		point4.SetXY(66-maskPosition_X,154);
		point5.SetXY(84-maskPosition_X,130);
		point6.SetXY(116-maskPosition_X,62);
		point7.SetXY(126-maskPosition_X,48);
		point8.SetXY(139-maskPosition_X,39);
		point9.SetXY(156-maskPosition_X,31);
		point10.SetXY(172-maskPosition_X,27);
		point11.SetXY(184-maskPosition_X,26);
		point12.SetXY(210-maskPosition_X,26);
		point13.SetXY(222-maskPosition_X,27);
		point14.SetXY(238-maskPosition_X,31);
		point15.SetXY(255-maskPosition_X,39);
		point16.SetXY(268-maskPosition_X,48);
		point17.SetXY(278-maskPosition_X,62);
		point18.SetXY(310-maskPosition_X,130);
		point19.SetXY(328-maskPosition_X,154);
		point20.SetXY(351-maskPosition_X,167);
		point21.SetXY(371-maskPosition_X,172);
		point22.SetXY(394-maskPosition_X,174);
		point23.SetXY(399-maskPosition_X,174);
		point24.SetXY(399-maskPosition_X,174);
		point25.SetXY(399-maskPosition_X,174);
		point26.SetXY(399-maskPosition_X,174);
		point27.SetXY(399-maskPosition_X,174);
		point28.SetXY(399-maskPosition_X,174);
		point29.SetXY(399-maskPosition_X,174);
		point30.SetXY(399-maskPosition_X,174);
		point31.SetXY(399-maskPosition_X,174);
		point32.SetXY(399-maskPosition_X,174);
		point33.SetXY(399-maskPosition_X,174);
		point34.SetXY(399-maskPosition_X,199);
		point35.SetXY(0-maskPosition_X,199);
		point36.SetXY(0-maskPosition_X,174);
	}


		// Sijoitetaan myˆs toinen maski, joka kulkee ruudulla
	// ensimm‰isen maskin per‰ss‰

	// Jos ensimm‰inen piirtopiste on ylitt‰nyt rajan, joka kulkee
	// ruudulla kohdassa -400x, aloitetaan piirto toiselta puolelta
	// eli maskin oikealta puolelta
	if (point1.iX < -400) {
		point1.iX+=400;
		point2.iX+=400;
		point3.iX+=400;
		point4.iX+=400;
		point5.iX+=400;
		point6.iX+=400;
		point7.iX+=400;
		point8.iX+=400;
		point9.iX+=400;
		point10.iX+=400;
		point11.iX+=400;
		point12.iX+=400;
		point13.iX+=400;
		point14.iX+=400;
		point15.iX+=400;
		point16.iX+=400;
		point17.iX+=400;
		point18.iX+=400;
		point19.iX+=400;
		point20.iX+=400;
		point21.iX+=400;
		point22.iX+=400;
		point23.iX+=400;
		point24.iX+=400;
		point25.iX+=400;
		point26.iX+=400;
		point27.iX+=400;
		point28.iX+=400;
		point29.iX+=400;
		point30.iX+=400;
		point31.iX+=400;
		point32.iX+=400;
		point33.iX+=400;
		point34.iX+=400;
		point35.iX+=400;
		point36.iX+=400;
	}

	

	// Sijoitetaan myˆs toinen maski, joka kulkee ruudulla
	// ensimm‰isen maskin per‰ss‰

	// Jos ensimm‰inen piirtopiste on ylitt‰nyt rajan, joka kulkee
	// ruudulla kohdassa -400x, aloitetaan piirto toiselta puolelta
	// eli maskin oikealta puolelta
	if ((point1.iX-400) < -400) {
		pointa.SetXY(point1.iX+400,point1.iY);
		pointb.SetXY(point2.iX+400,point2.iY);
		pointc.SetXY(point3.iX+400,point3.iY);
		pointd.SetXY(point4.iX+400,point4.iY);
		pointe.SetXY(point5.iX+400,point5.iY);
		pointf.SetXY(point6.iX+400,point6.iY);
		pointg.SetXY(point7.iX+400,point7.iY);
		pointh.SetXY(point8.iX+400,point8.iY);
		pointi.SetXY(point9.iX+400,point9.iY);
		pointj.SetXY(point10.iX+400,point10.iY);
		pointk.SetXY(point11.iX+400,point11.iY);
		pointl.SetXY(point12.iX+400,point12.iY);
		pointm.SetXY(point13.iX+400,point13.iY);
		pointn.SetXY(point14.iX+400,point14.iY);
		pointo.SetXY(point15.iX+400,point15.iY);
		pointp.SetXY(point16.iX+400,point16.iY);
		pointq.SetXY(point17.iX+400,point17.iY);
		pointr.SetXY(point18.iX+400,point18.iY);
		points.SetXY(point19.iX+400,point19.iY);
		pointt.SetXY(point20.iX+400,point20.iY);
		pointu.SetXY(point21.iX+400,point21.iY);
		pointv.SetXY(point22.iX+400,point22.iY);
		pointw.SetXY(point23.iX+400,point23.iY);
		pointx.SetXY(point24.iX+400,point24.iY);
		pointy.SetXY(point25.iX+400,point25.iY);
		pointz.SetXY(point26.iX+400,point26.iY);
		pointaa.SetXY(point27.iX+400,point27.iY);
		pointab.SetXY(point28.iX+400,point28.iY);
		pointac.SetXY(point29.iX+400,point29.iY);
		pointad.SetXY(point30.iX+400,point30.iY);
		pointae.SetXY(point31.iX+400,point31.iY);
		pointaf.SetXY(point32.iX+400,point32.iY);
		pointag.SetXY(point33.iX+400,point33.iY);
		pointah.SetXY(point34.iX+400,point34.iY);
		pointai.SetXY(point35.iX+400,point35.iY);
		pointaj.SetXY(point36.iX+400,point36.iY);
	}

	// Jos piirtokohta alkaa ennen kuin se saavuttaa -400:n,
	// rajan, piirret‰‰n maskia toisen vasemmalle puolelle.
	else {
		pointa.SetXY(point1.iX-400,point1.iY);
		pointb.SetXY(point2.iX-400,point2.iY);
		pointc.SetXY(point3.iX-400,point3.iY);
		pointd.SetXY(point4.iX-400,point4.iY);
		pointe.SetXY(point5.iX-400,point5.iY);
		pointf.SetXY(point6.iX-400,point6.iY);
		pointg.SetXY(point7.iX-400,point7.iY);
		pointh.SetXY(point8.iX-400,point8.iY);
		pointi.SetXY(point9.iX-400,point9.iY);
		pointj.SetXY(point10.iX-400,point10.iY);
		pointk.SetXY(point11.iX-400,point11.iY);
		pointl.SetXY(point12.iX-400,point12.iY);
		pointm.SetXY(point13.iX-400,point13.iY);
		pointn.SetXY(point14.iX-400,point14.iY);
		pointo.SetXY(point15.iX-400,point15.iY);
		pointp.SetXY(point16.iX-400,point16.iY);
		pointq.SetXY(point17.iX-400,point17.iY);
		pointr.SetXY(point18.iX-400,point18.iY);
		points.SetXY(point19.iX-400,point19.iY);
		pointt.SetXY(point20.iX-400,point20.iY);
		pointu.SetXY(point21.iX-400,point21.iY);
		pointv.SetXY(point22.iX-400,point22.iY);
		pointw.SetXY(point23.iX-400,point23.iY);
		pointx.SetXY(point24.iX-400,point24.iY);
		pointy.SetXY(point25.iX-400,point25.iY);
		pointz.SetXY(point26.iX-400,point26.iY);
		pointaa.SetXY(point27.iX-400,point27.iY);
		pointab.SetXY(point28.iX-400,point28.iY);
		pointac.SetXY(point29.iX-400,point29.iY);
		pointad.SetXY(point30.iX-400,point30.iY);
		pointae.SetXY(point31.iX-400,point31.iY);
		pointaf.SetXY(point32.iX-400,point32.iY);
		pointag.SetXY(point33.iX-400,point33.iY);
		pointah.SetXY(point34.iX-400,point34.iY);
		pointai.SetXY(point35.iX-400,point35.iY);
		pointaj.SetXY(point36.iX-400,point36.iY);
	}


	/*
			N‰m‰ ovat vain editointitarkoituksiin,
			ja ovat tarkoituksella j‰tetty t‰h‰n.
	point1.SetXY();
	point2.SetXY();
	point3.SetXY();
	point4.SetXY();
	point5.SetXY();
	point6.SetXY();
	point7.SetXY();
	point8.SetXY();
	point9.SetXY();
	point10.SetXY();
	point11.SetXY();
	point12.SetXY();
	point13.SetXY();
	point14.SetXY();
	point15.SetXY();
	point16.SetXY();
	point17.SetXY();
	point18.SetXY();
	point19.SetXY();
	point20.SetXY();
	point21.SetXY();
	point22.SetXY();
	point23.SetXY();
	point24.SetXY();
	point25.SetXY();
	point26.SetXY();
	point27.SetXY();
	point28.SetXY();
	point29.SetXY();
	point30.SetXY();
	point31.SetXY();
	point32.SetXY();
	point33.SetXY();

	*/



	// Sijoitetaan pisteet n‰ytˆlle

	CArrayFix<TPoint>* mypoints = new CArrayFixFlat<TPoint>(73); 
	CleanupStack::PushL(mypoints); 
	mypoints->AppendL(point1);
	mypoints->AppendL(point2);
	mypoints->AppendL(point3);
	mypoints->AppendL(point4);
	mypoints->AppendL(point5);
	mypoints->AppendL(point6);
	mypoints->AppendL(point7);
	mypoints->AppendL(point8);
	mypoints->AppendL(point9);
	mypoints->AppendL(point10);
	mypoints->AppendL(point11);
	mypoints->AppendL(point12);
	mypoints->AppendL(point13);
	mypoints->AppendL(point14);
	mypoints->AppendL(point15);
	mypoints->AppendL(point16);
	mypoints->AppendL(point17);
	mypoints->AppendL(point18);
	mypoints->AppendL(point19);
	mypoints->AppendL(point20);
	mypoints->AppendL(point21);
	mypoints->AppendL(point22);
	mypoints->AppendL(point23);
	mypoints->AppendL(point24);
	mypoints->AppendL(point25);
	mypoints->AppendL(point26);
	mypoints->AppendL(point27);
	mypoints->AppendL(point28);
	mypoints->AppendL(point29);
	mypoints->AppendL(point30);
	mypoints->AppendL(point31);
	mypoints->AppendL(point32);
	mypoints->AppendL(point33);
	mypoints->AppendL(point34);
	mypoints->AppendL(point35);
	mypoints->AppendL(point36);

	mypoints->AppendL(pointa);
	mypoints->AppendL(pointb);
	mypoints->AppendL(pointc);
	mypoints->AppendL(pointd);
	mypoints->AppendL(pointe);
	mypoints->AppendL(pointf);
	mypoints->AppendL(pointg);
	mypoints->AppendL(pointh);
	mypoints->AppendL(pointi);
	mypoints->AppendL(pointj);
	mypoints->AppendL(pointk);
	mypoints->AppendL(pointl);
	mypoints->AppendL(pointm);
	mypoints->AppendL(pointn);
	mypoints->AppendL(pointo);
	mypoints->AppendL(pointp);
	mypoints->AppendL(pointq);
	mypoints->AppendL(pointr);
	mypoints->AppendL(points);
	mypoints->AppendL(pointt);
	mypoints->AppendL(pointu);
	mypoints->AppendL(pointv);
	mypoints->AppendL(pointw);
	mypoints->AppendL(pointx);
	mypoints->AppendL(pointy);
	mypoints->AppendL(pointz);
	mypoints->AppendL(pointaa);
	mypoints->AppendL(pointab);
	mypoints->AppendL(pointac);
	mypoints->AppendL(pointad);
	mypoints->AppendL(pointae);
	mypoints->AppendL(pointaf);
	mypoints->AppendL(pointag);
	mypoints->AppendL(pointah);
	mypoints->AppendL(pointai);
	mypoints->AppendL(pointaj);


					// T‰ytet‰‰n maskipolygoni mustalla v‰rill‰
	bitmapContext->SetBrushStyle(CGraphicsContext::ESolidBrush); 
	bitmapContext->SetBrushColor(TRgb(0,0,0)); 
	bitmapContext->DrawPolygon(mypoints,CGraphicsContext::EWinding); 


					// switch statement with example cases 
	bitmapContext->DrawPolyLine(mypoints); 
	CleanupStack::PopAndDestroy(); // mypoints


				// Yˆkuva
		
	CFbsBitmap* bitmapNight;
	bitmapNight = new (ELeave) CFbsBitmap();
	CleanupStack::PushL(bitmapNight); 
				// Ladataan t‰m‰n kuun kuva
	TRAP(err, User::LeaveIfError(bitmapNight->Load(_L("c:\\system\\apps\\sunclock\\night.mbm"), 0)));
	if (err)
	{
		TRAP(err, User::LeaveIfError(bitmapNight->Load(_L("d:\\system\\apps\\sunclock\\night.mbm"), 0)));
		if (err)
		{
			User::LeaveIfError(bitmapNight->Load(_L("z:\\system\\apps\\sunclock\\night.mbm"), 0));
		}
	}
	CleanupStack::Pop();

				// Blitataan kuva sivupalkin viereen.
	TPoint nightMapPosition(63+iMapPosition,0); 
	TRect nightMapRect(0,0,400-iMapPosition,200);
	gc.BitBlt(nightMapPosition, bitmapNight, nightMapRect);

	TPoint nightMapPosition2(63,0); 
	TRect nightMapRect2(400-iMapPosition,0,400,200);
	gc.BitBlt(nightMapPosition2, bitmapNight, nightMapRect2);

		
			// P‰iv‰kuva
	
	CFbsBitmap* bitmapSeason;
	bitmapSeason = new (ELeave) CFbsBitmap();
	CleanupStack::PushL(bitmapSeason);

			// Seasons.mbm-kuvaan tallennetun kuvan numero.
			// Tiedostossa on 24 kuvaa; 2 jokaiselle kuukaudelle.
			// Katsotaan, mik‰ kuva osuu t‰lle ajankohdalle.
	TInt imageNumber = 0;

	if ((dayNoInYear >= 0) && (dayNoInYear <= 14)) {
		imageNumber = 0;
	}
	if ((dayNoInYear >= 15) && (dayNoInYear <= 30)) {
		imageNumber = 1;
	}
	if ((dayNoInYear >= 31) && (dayNoInYear <= 44)) {
		imageNumber = 2;
	}
	if ((dayNoInYear >= 45) && (dayNoInYear <= 60)) {
		imageNumber = 3;
	}
	if ((dayNoInYear >= 61) && (dayNoInYear <= 74)) {
		imageNumber = 4;
	}
	if ((dayNoInYear >= 75) && (dayNoInYear <= 90)) {
		imageNumber = 5;
	}
	if ((dayNoInYear >= 91) && (dayNoInYear <= 104)) {
		imageNumber = 6;
	}
	if ((dayNoInYear >= 105) && (dayNoInYear <= 120)) {
		imageNumber = 7;
	}
	if ((dayNoInYear >= 121) && (dayNoInYear <= 134)) {
		imageNumber = 8;
	}
	if ((dayNoInYear >= 135) && (dayNoInYear <= 150)) {
		imageNumber = 9;
	}
	if ((dayNoInYear >= 151) && (dayNoInYear <= 164)) {
		imageNumber = 10;
	}
	if ((dayNoInYear >= 165) && (dayNoInYear <= 180)) {
		imageNumber = 11;
	}
	if ((dayNoInYear >= 181) && (dayNoInYear <= 194)) {
		imageNumber = 12;
	}
	if ((dayNoInYear >= 195) && (dayNoInYear <= 210)) {
		imageNumber = 13;
	}
	if ((dayNoInYear >= 211) && (dayNoInYear <= 224)) {
		imageNumber = 14;
	}
	if ((dayNoInYear >= 225) && (dayNoInYear <= 240)) {
		imageNumber = 15;
	}
	if ((dayNoInYear >= 241) && (dayNoInYear <= 254)) {
		imageNumber = 16;
	}
	if ((dayNoInYear >= 255) && (dayNoInYear <= 270)) {
		imageNumber = 17;
	}
	if ((dayNoInYear >= 271) && (dayNoInYear <= 284)) {
		imageNumber = 18;
	}
	if ((dayNoInYear >= 285) && (dayNoInYear <= 300)) {
		imageNumber = 19;
	}
	if ((dayNoInYear >= 301) && (dayNoInYear <= 314)) {
		imageNumber = 20;
	}
	if ((dayNoInYear >= 315) && (dayNoInYear <= 330)) {
		imageNumber = 21;
	}
	if ((dayNoInYear >= 331) && (dayNoInYear <= 344)) {
		imageNumber = 22;
	}
	if ((dayNoInYear >= 345) && (dayNoInYear <= 366)) {
		imageNumber = 23;
	}

			// Ladataan t‰m‰n kauden kuva
	TRAP(err, User::LeaveIfError(bitmapSeason->Load(_L("c:\\system\\apps\\sunclock\\seasons.mbm"), imageNumber)));
	if (err)
	{
		TRAP(err, User::LeaveIfError(bitmapSeason->Load(_L("d:\\system\\apps\\sunclock\\seasons.mbm"), imageNumber)));
		if (err)
		{
			User::LeaveIfError(bitmapSeason->Load(_L("z:\\system\\apps\\sunclock\\seasons.mbm"), imageNumber));
		}
	}



	CleanupStack::Pop(); // bitmapSeason
	TPoint posSeasonBitmap(63+iMapPosition,0); 
	TPoint posSeasonBitmap2(63,0); //modulo

	TRect bmpPieceRect(0,0,400-iMapPosition,200);
	gc.BitBltMasked(posSeasonBitmap,bitmapSeason,bmpPieceRect,offScreenBitmap,ETrue);

	TRect bmpPieceRect2(400-iMapPosition,0,400,200); // modulo
	gc.BitBltMasked(posSeasonBitmap2,bitmapSeason,bmpPieceRect2,offScreenBitmap,ETrue); //modulo


			// Muuttujat, jotka pit‰v‰t sis‰ll‰‰n nykyisen auringonnousuajan
			// iSunrise m‰‰ritet‰‰n RSunlight-funktiossa
	TDateTime sunriseDateTime = iSunrise.DateTime();
	TInt sunriseHours = sunriseDateTime.Hour();
	TInt sunriseMinutes = sunriseDateTime.Minute();

			// Muuttujat, jotka pit‰v‰t sis‰ll‰‰n nykyisen auringonlaskuajan
	TDateTime sunsetDateTime = iSunset.DateTime();
	TInt sunsetHours = sunsetDateTime.Hour();
	TInt sunsetMinutes = sunsetDateTime.Minute();

	
			// Jos kes‰aika on k‰ytˆss‰, siirret‰‰n auringonlasku- ja -nousuaikoja
			// tunnilla eteen p‰in
	if((dayNoInYear >= iDayDSTStarts) && (dayNoInYear <= iDayDSTEnds)) {
		sunriseHours = sunriseHours + 1;
		sunsetHours = sunsetHours + 1;
	}


	_LIT(KSunlightTimeNormal,"%d:%d");
			// T‰m‰ muuttuja est‰‰ sen, ett‰ auringonlasku- ja nousuajat
			// eiv‰t saa muotoa "11:2", vaan minuutin eteen lis‰t‰‰n 0
	_LIT(KTSunlightTimeWithLessThan10Minutes,"%d:0%d");

			// Sijoitetaan auringonnousu- ja -laskuajat tekstimuuttujiin
	TBuf<7> sunriseTime;
	if (sunriseMinutes < 10)
	{
		sunriseTime.Format(KTSunlightTimeWithLessThan10Minutes, sunriseHours, sunriseMinutes);
	}
	else
	{
		sunriseTime.Format(KSunlightTimeNormal, sunriseHours, sunriseMinutes);
	}


	TBuf<7> sunsetTime;
	if (sunsetMinutes < 10)
	{
		sunsetTime.Format(KTSunlightTimeWithLessThan10Minutes, sunsetHours, sunsetMinutes);
	}
	else
	{
		sunsetTime.Format(KSunlightTimeNormal, sunsetHours, sunsetMinutes);
	}
	
	
			// Auringonnousukuva
	CFbsBitmap* bitmapSunrise;
	bitmapSunrise = new (ELeave) CFbsBitmap();
	CleanupStack::PushL(bitmapSunrise); 
			// Ladataan kuva
	TRAP(err, User::LeaveIfError(bitmapSunrise->Load(_L("c:\\system\\apps\\sunclock\\sunlight.mbm"), 0)));
	if (err)
	{
		TRAP(err, User::LeaveIfError(bitmapSunrise->Load(_L("d:\\system\\apps\\sunclock\\sunlight.mbm"), 0)));
		if (err)
		{
			User::LeaveIfError(bitmapSunrise->Load(_L("z:\\system\\apps\\sunclock\\sunlight.mbm"), 0));
		}
	}
	CleanupStack::Pop(); // bitmapSunrise
	TPoint posSunrise(17,4); 
	gc.BitBlt(posSunrise, bitmapSunrise);



			// Auringonlaskukuva
	CFbsBitmap* bitmapSunset;
	bitmapSunset = new (ELeave) CFbsBitmap();
	CleanupStack::PushL(bitmapSunset); 
			// Ladataan kuva
	TRAP(err, User::LeaveIfError(bitmapSunset->Load(_L("c:\\system\\apps\\sunclock\\sunlight.mbm"), 1)));
	if (err)
	{
		TRAP(err, User::LeaveIfError(bitmapSunset->Load(_L("d:\\system\\apps\\sunclock\\sunlight.mbm"), 1)));
		if (err)
		{
			User::LeaveIfError(bitmapSunset->Load(_L("z:\\system\\apps\\sunclock\\sunlight.mbm"), 1));
		}
	}
	CleanupStack::Pop(); // bitmapSunset
	TPoint posSunset(17,49); 
	gc.BitBlt(posSunset, bitmapSunset);



				// Jos k‰ytt‰j‰ on m‰‰ritt‰nyt, ett‰ ekvaattori piirret‰‰n,
				// niin sitten se myˆs piirret‰‰n.
	TPoint equatorStartPoint(63,99);
	TPoint equatorEndPoint(463,99);

	if (iShowEquator == 1)
	{
		gc.SetPenColor(TRgb(255,0,0)); // change the pen color
		gc.DrawLine(equatorStartPoint,equatorEndPoint); 
	}

				// Jos k‰ytt‰j‰ on m‰‰ritt‰nyt, ett‰ Kravun k‰‰ntˆpiiri piirret‰‰n,
				// niin sitten se myˆs piirret‰‰n.
	TPoint tropicOfCancerStartPoint(63,73);
	TPoint tropicOfCancerEndPoint(463,73);

	if (iShowTropicOfCancer == 1)
	{
		gc.SetPenColor(TRgb(255,255,0)); // change the pen color
		gc.DrawLine(tropicOfCancerStartPoint,tropicOfCancerEndPoint); 
	}

				// Jos k‰ytt‰j‰ on m‰‰ritt‰nyt, ett‰ Kauriin k‰‰ntˆpiiri piirret‰‰n,
				// niin sitten se myˆs piirret‰‰n.
	TPoint tropicOfCapricornStartPoint(63,125);
	TPoint tropicOfCapricornEndPoint(463,125);

	if (iShowTropicOfCapricorn == 1)
	{
		gc.SetPenColor(TRgb(255,255,0)); // change the pen color
		gc.DrawLine(tropicOfCapricornStartPoint,tropicOfCapricornEndPoint); 
	}

				// Jos k‰ytt‰j‰ on m‰‰ritt‰nyt, ett‰ pohjoinen napapiiri piirret‰‰n,
				// niin sitten se myˆs piirret‰‰n.
	TPoint arcticCircleStartPoint(63,25);
	TPoint arcticCircleEndPoint(463,25);

	if (iShowArcticCircle == 1)
	{
		gc.SetPenColor(TRgb(0,0,255)); // change the pen color
		gc.DrawLine(arcticCircleStartPoint,arcticCircleEndPoint); 
	}

				// Jos k‰ytt‰j‰ on m‰‰ritt‰nyt, ett‰ etel‰inen napapiiri piirret‰‰n,
				// niin sitten se myˆs piirret‰‰n.
	TPoint antarcticCircleStartPoint(63,173);
	TPoint antarcticCircleEndPoint(463,173);

	if (iShowAntarcticCircle == 1)
	{
		gc.SetPenColor(TRgb(0,0,255)); // change the pen color
		gc.DrawLine(antarcticCircleStartPoint,antarcticCircleEndPoint); 
	}





				// Muuttujat, joita k‰yt‰mme kuun faasin laskemiseen
	TReal lunarMonthHours = 708.734139;
	TTime newMoonDateAndTime;
	TTimeIntervalHours hoursSinceFirstNewMoon;
	TReal roundedNumber;
	TReal fraction;
	TInt moonImage;

				// Sijoitetaan valitsemamme uuden kuun ajankohta
				// t‰h‰n muuttujaan, ja tallennetaan se lopuksi
				// TTime-muuttujaan. Ajankohta olisi voinut olla
				// mik‰ vain uuden kuun ajankohta, mutta valitsin
				// ohjelman tekohetkell‰ viimeisimm‰n uuden kuun.
	TBuf<25> newMoonTime;
	_LIT(KLastNewMoonTime,"20090917:053300.000000");//Jan 19 04:01//20090917:053300.000000");
	newMoonTime.Format(KLastNewMoonTime);
	newMoonDateAndTime.Set(newMoonTime);

				// Lasketaan, montako tuntia on kulunut m‰‰ritt‰m‰st‰mme
				// uudesta kuusta
	currentTime.HoursFrom(newMoonDateAndTime, hoursSinceFirstNewMoon);

				// Muutetaan TTimeIntervalHours kokonaisluvuksi; t‰m‰ toimii
				// viimeisen kerran jossain kohtaa vuonna 2017, koska 32-
				// bittinen TUint-kokonaisluku voi sis‰lt‰‰ luvun v‰lill‰
				// 0 - 65 535, ja vuoteen 2017 on matkaa tuon maksimim‰‰r‰n
				// verran tunteja.
	TUint hoursSinceFirstNewMoonUInt = hoursSinceFirstNewMoon.Int();


				// Kuunkierojen suhdeluku; esim. jos m‰‰ritt‰m‰st‰mme kuun-
				// kierrosta on kulunut vaikkapa 918 tuntia, suhdeluvuksi
				// tulee 1,2953.
	TReal lunarMonthRatio = hoursSinceFirstNewMoonUInt / lunarMonthHours;

				// Pyˆristet‰‰n suhdeluku kahden desimaalin tarkkuudelle, esim.
				// 1,2953 -> 1,30
	Math::Round(roundedNumber,lunarMonthRatio, 2);

				// Ja otetaan luvusta pelk‰t desimaalit, esim. 1,30 -> 0,30
	Math::Frac(fraction,roundedNumber);





			// Seuraava koodinp‰tk‰ laskee, montako prosenttia Kuusta
			// n‰kyy t‰ll‰ hetkell‰.
	MoonPhase moonPhase;

    TInt y = dateTime.Year();
			// Lis‰t‰‰n kuukauteen ja p‰iv‰‰n yksi, koska ne alkavat
			// laskea nollasta (kuukauden ensimm‰inen p‰iv‰ on 0.)
	TInt m = dateTime.Month() + 1;
	TInt d = dateTime.Day() + 1;
    TInt h = dateTime.Hour();

    TReal moment;
    TReal moment2;

				// Jotta Kuun faasi n‰kyisi kaikkialla oikein,
				// t‰ytyy laskea, montako tuntia paikka on j‰ljess‰
				// Greenwichin ajasta
	TInt minutesLong = cityData.iLatLong/*latLong*/.MinutesLong();
	// EI TOIMI
	TInt hourOffset = 12 * minutesLong / 10800;
//	TInt hourOffset = off * 12;

			// 0 = ei, 1 = kyll‰
    TInt headingTowardsFullMoon = 0;

    moment = moonPhase.moon_phase(y, m, d, h+hourOffset);
    moment2 = moonPhase.moon_phase(y, m, d, h+hourOffset+1);

				// T‰ll‰ tarkistetaan, kasvaako Kuu t‰ll‰ hetkell‰
				// vai v‰heneekˆ se -> tarkistetaan, paljonko prosentti-
				// arvo on tunnin p‰‰st‰ ja p‰‰tell‰‰n siit‰.
	if (moment > moment2)
	{
		headingTowardsFullMoon = 0;
	}
	if (moment <= moment2)
	{
		headingTowardsFullMoon = 1;
	}
	TReal moonPercentageLit = floor(moment*1000+0.5)/10;


			// Tekstimuuttujaan tallennetaan t‰m‰n hetkinen Kuun
			// n‰kyvyys prosentteina.
	TBuf<5> moonPercentage;
	_LIT(KMoonPercentage,"%3.0f%%");
	moonPercentage.Format(KMoonPercentage, moonPercentageLit);

	

		
			// moon_tfm.mbm-kuvassa on 12 kuvaa (jos uusikuu- ja t‰ysikuukuvia
			// ei oteta huomioon), 100 prosenttia t‰ytyy jakaa 12:lla
	TReal hundredDividedBy12 = 8.333333333333333333333333333;
	
	
			// Kuun kuva sidebariin
	CFbsBitmap* bitmapMoon;
	bitmapMoon = new (ELeave) CFbsBitmap();
	CleanupStack::PushL(bitmapMoon); 
	
	if (headingTowardsFullMoon == 1)
	{
					// Sijoitetaan moonImage-muuttujaan oikea kuvan numero:
					// 13 = uusi kuu ja 12 = uusi kuu, 0-11 ovat silt‰ v‰lilt‰
		if (moonPercentageLit <= hundredDividedBy12/2) { moonImage = 13;}
		if ((moonPercentageLit > hundredDividedBy12/2) && (moonPercentageLit <= hundredDividedBy12)) { moonImage = 0;}
		if ((moonPercentageLit > hundredDividedBy12) && (moonPercentageLit <= hundredDividedBy12*2)) { moonImage = 1;}
		if ((moonPercentageLit > hundredDividedBy12*2) && (moonPercentageLit <= hundredDividedBy12*3)) { moonImage = 2;} 
		if ((moonPercentageLit > hundredDividedBy12*3) && (moonPercentageLit <= hundredDividedBy12*4)) { moonImage = 3;} 
		if ((moonPercentageLit > hundredDividedBy12*4) && (moonPercentageLit <= hundredDividedBy12*5)) { moonImage = 4;} 
		if ((moonPercentageLit > hundredDividedBy12*5) && (moonPercentageLit <= hundredDividedBy12*6)) { moonImage = 5;} 
		if ((moonPercentageLit > hundredDividedBy12*6) && (moonPercentageLit <= hundredDividedBy12*7)) { moonImage = 6;} 
		if ((moonPercentageLit > hundredDividedBy12*7) && (moonPercentageLit <= hundredDividedBy12*8)) { moonImage = 7;} 
		if ((moonPercentageLit > hundredDividedBy12*8) && (moonPercentageLit <= hundredDividedBy12*9)) { moonImage = 8;} 
		if ((moonPercentageLit > hundredDividedBy12*9) && (moonPercentageLit <= hundredDividedBy12*10)) { moonImage = 9;} 
		if ((moonPercentageLit > hundredDividedBy12*10) && (moonPercentageLit <= hundredDividedBy12*11)) { moonImage = 10;} 
		if ((moonPercentageLit > hundredDividedBy12*11) && (moonPercentageLit <= hundredDividedBy12*11+hundredDividedBy12/2)) { moonImage = 11;} 
		if (moonPercentageLit > hundredDividedBy12*11+hundredDividedBy12/2) { moonImage = 12;}

				// Ladataan kuva
		TRAP(err, User::LeaveIfError(bitmapMoon->Load(_L("c:\\system\\apps\\sunclock\\moon_tfm.mbm"), moonImage)));
		if (err)
		{
			TRAP(err, User::LeaveIfError(bitmapMoon->Load(_L("d:\\system\\apps\\sunclock\\moon_tfm.mbm"), moonImage)));
			if (err)
			{
				User::LeaveIfError(bitmapMoon->Load(_L("z:\\system\\apps\\sunclock\\moon_tfm.mbm"), moonImage));
			}
		}
	}



	TInt hundredDividedBy10 = 10;

	if (headingTowardsFullMoon == 0)
	{
					// Sijoitetaan moonImage-muuttujaan oikea kuvan numero:
					// 11 = uusi kuu ja 10 = uusi kuu, 0-9 ovat silt‰ v‰lilt‰
		if (moonPercentageLit <= hundredDividedBy10/2) { moonImage = 11;}
		if ((moonPercentageLit > hundredDividedBy10/2) && (moonPercentageLit <= hundredDividedBy10)) { moonImage = 9;}
		if ((moonPercentageLit > hundredDividedBy10) && (moonPercentageLit <= hundredDividedBy10*2)) { moonImage = 8;}
		if ((moonPercentageLit > hundredDividedBy10*2) && (moonPercentageLit <= hundredDividedBy10*3)) { moonImage = 7;} 
		if ((moonPercentageLit > hundredDividedBy10*3) && (moonPercentageLit <= hundredDividedBy10*4)) { moonImage = 6;} 
		if ((moonPercentageLit > hundredDividedBy10*4) && (moonPercentageLit <= hundredDividedBy10*5)) { moonImage = 5;} 
		if ((moonPercentageLit > hundredDividedBy10*5) && (moonPercentageLit <= hundredDividedBy10*6)) { moonImage = 4;} 
		if ((moonPercentageLit > hundredDividedBy10*6) && (moonPercentageLit <= hundredDividedBy10*7)) { moonImage = 3;} 
		if ((moonPercentageLit > hundredDividedBy10*7) && (moonPercentageLit <= hundredDividedBy10*8)) { moonImage = 2;} 
		if ((moonPercentageLit > hundredDividedBy10*8) && (moonPercentageLit <= hundredDividedBy10*9)) { moonImage = 1;} 
		if ((moonPercentageLit > hundredDividedBy10*9) && (moonPercentageLit <= hundredDividedBy10*10-hundredDividedBy10/2)) { moonImage = 0;} 
		if (moonPercentageLit > hundredDividedBy10*10-hundredDividedBy10/2) { moonImage = 10;}

				// Ladataan kuva
		TRAP(err, User::LeaveIfError(bitmapMoon->Load(_L("c:\\system\\apps\\sunclock\\moon_tnm.mbm"), moonImage)));
		if (err)
		{
			TRAP(err, User::LeaveIfError(bitmapMoon->Load(_L("d:\\system\\apps\\sunclock\\moon_tnm.mbm"), moonImage)));
			if (err)
			{
				User::LeaveIfError(bitmapMoon->Load(_L("z:\\system\\apps\\sunclock\\moon_tnm.mbm"), moonImage));
			}
		}
	}

	CleanupStack::Pop(); // moonImage
	TPoint posMoon(11,94);
	gc.BitBlt(posMoon, bitmapMoon);



				// Fontti
	fontUsed = eikonEnv->DenseFont();
	gc.UseFont(fontUsed);

				// Draw the text in the middle of the rectangle.
				// "Oct 28"
	TInt   baselineOffset=(175);

				// "2009"
	TInt   baselineOffset2=(194);

				// Auringonnousuaika
	TInt   baselineOffset3=(41);

				// Auringonlaskuaika
	TInt   baselineOffset4=(86);

				// Kuun prosentti
	TInt   baselineOffset5=(150);

				// Tekstin v‰ri
	gc.SetPenColor(KRgbWhite); 

				// Piirret‰‰n teksti
	gc.DrawText(monthAndDay,sideBarRect,baselineOffset,CGraphicsContext::ECenter, 0);
	gc.DrawText(iYearText,sideBarRect,baselineOffset2,CGraphicsContext::ECenter, 0);
	gc.DrawText(sunriseTime,sideBarRect,baselineOffset3,CGraphicsContext::ECenter, 0);
	gc.DrawText(sunsetTime,sideBarRect,baselineOffset4,CGraphicsContext::ECenter, 0);
	gc.DrawText(moonPercentage,sideBarRect,baselineOffset5,CGraphicsContext::ECenter, 0);

				// Finished using the font
	gc.DiscardFont();

	CleanupStack::PopAndDestroy(3); //mypoints, offScreenBitmap, bitmapDevice
	delete bitmapNight;
	delete bitmapSeason;
	delete bitmapSunrise;
	delete bitmapSunset;
	delete bitmapMoon;

}


				// Asetetaan p‰iv‰n numero vuodessa, joka on saatu
				// AppUi:n kalenteridialogin kautta.
void CSunClockAppView::SetDay(TTime aDate, TTime aDate2, TInt aChoice)
{
	iDate = aDate;
	iDate2 = aDate2;
	iDayDSTStarts = iDate.DayNoInYear();
	iDayDSTEnds = iDate2.DayNoInYear();
	iChoice = aChoice;
	DrawNow();
}



				// INI-tiedoston ID
TStreamId CSunClockAppView::StoreL(CStreamStore& aStore) const
{
    RStoreWriteStream outStream;
    TStreamId id=outStream.CreateLC(aStore);
    ExternalizeL(outStream);
    outStream.CommitL();
    CleanupStack::PopAndDestroy();  // outStream
    return id;
}


				// Avataan INI-tiedosto?
void CSunClockAppView::RestoreL(const CStreamStore& aStore, TStreamId aId)
{
    RStoreReadStream inStream;
    inStream.OpenLC(aStore,aId);
    InternalizeL(inStream);
    CleanupStack::PopAndDestroy(); // inStream
}



				// Kirjoitetaan arvot INI-tiedostoon
void CSunClockAppView::ExternalizeL(RWriteStream& aOut) const
{
    aOut.WriteInt32L(static_cast<TInt>(iDayDSTStarts));
    aOut.WriteInt32L(static_cast<TInt>(iDayDSTEnds));
    aOut.WriteInt32L(static_cast<TInt>(iYearNo));
    aOut.WriteInt32L(static_cast<TInt>(iChoice));
    aOut.WriteInt32L(static_cast<TInt>(iMapPosition));
    aOut.WriteInt32L(static_cast<TInt>(iShowArcticCircle));
    aOut.WriteInt32L(static_cast<TInt>(iShowTropicOfCancer));
    aOut.WriteInt32L(static_cast<TInt>(iShowEquator));
    aOut.WriteInt32L(static_cast<TInt>(iShowTropicOfCapricorn));
    aOut.WriteInt32L(static_cast<TInt>(iShowAntarcticCircle));
    aOut.WriteInt32L(static_cast<TInt>(iVersionNumber));
}


				// Ladataan arvot INI-tiedostosta
void CSunClockAppView::InternalizeL(RReadStream& aIn)
{
    iDayDSTStarts=static_cast<TInt>(aIn.ReadInt32L());
    iDayDSTEnds=static_cast<TInt>(aIn.ReadInt32L());
    iYearNo=static_cast<TInt>(aIn.ReadInt32L());
    iChoice=static_cast<TInt>(aIn.ReadInt32L());
    iMapPosition=static_cast<TInt>(aIn.ReadInt32L());
    iShowArcticCircle=static_cast<TInt>(aIn.ReadInt32L());
    iShowTropicOfCancer=static_cast<TInt>(aIn.ReadInt32L());
    iShowEquator=static_cast<TInt>(aIn.ReadInt32L());
    iShowTropicOfCapricorn=static_cast<TInt>(aIn.ReadInt32L());
    iShowAntarcticCircle=static_cast<TInt>(aIn.ReadInt32L());
    iVersionNumber=static_cast<TInt>(aIn.ReadInt32L());
	DrawNow();
}


				// Tallennetaan INI-tiedosto tiedostoj‰rjestelm‰‰n
void CSunClockAppView::SaveL()
{
	_LIT(KRulerIniFileName,"c:\\system\\data\\sunclock.ini");
    RFs fs;
    fs.Connect();

    CFileStore* store=NULL;;

    TParse parse;
    parse.Set(KRulerIniFileName,NULL,NULL);

    TRAPD(replaceErrorCode,store=CDirectFileStore::ReplaceL(fs,KRulerIniFileName,EFileShareAny|EFileWrite));

	if (replaceErrorCode!=KErrNone)
    {
        store=CDirectFileStore::CreateL(fs,KRulerIniFileName,EFileShareAny|EFileWrite);
    }

    CleanupStack::PushL(store);
    store->SetTypeL(KDirectFileStoreLayoutUid);
    RStoreWriteStream outStream;
    TStreamId id=outStream.CreateLC(*store);
    ExternalizeL(outStream);
    outStream.CommitL();
    store->SetRootL(id);
    store->CommitL();
    CleanupStack::PopAndDestroy(2); // outStream + store
    fs.Close();
}




				// Asetetaan vuosiluku ja tallennetaan INI-tiedostoon
				// Funktio kutsutaan AppUi:sta
void CSunClockAppView::SetYearL(TInt aYear)
{
	iYearNo = aYear;
	SaveL();
}




		// T‰m‰ funktio palauttaa auringonlasku- ja
		// nousuajan, ja se p‰ivittyy silloin kun jokin
		// tiedoista muuttuu
void CSunClockAppView::RSunlight()
{
	iCurrentTime.HomeTime();   
	iWS.Connect();
    iErr2 = iWS.Home(iWorldID);   
    if( iErr2 == KErrNone )   
    {   
        iErr2 = iWS.CalculateSunlight(iSunrise, iSunset,
            iWorldID, iCurrentTime);   
		iWS.CityData(cityData, iWorldID);   
    }
	iWS.Close();

			// Selvitet‰‰n, montako asteminuuttia kaukana nykyinen
			// kotikaupunki on Greenwichista, Lontoosta
	TLatLong latLong;
	latLong = cityData.iLatLong;
	TInt minutesLong = latLong.MinutesLong();

			// Muutetaan iMapPositionin arvoa niin, ett‰ miinustetaan
			// Greenwichin pikseliarvosta (200) offset
	TInt minutesPerPixel = 10800 / 200; // = 54
	TReal pixelOffset = minutesLong / minutesPerPixel;
	Math::Int(pixelOffsetInt,pixelOffset);
	if (pixelOffsetInt <= 0)
	{
		iMapPosition = 200 + pixelOffsetInt + 200;
	}
	if (pixelOffsetInt > 0)
	{
		iMapPosition = pixelOffsetInt;
	}


}



				// Ajastimen funktio (1 sekunnin p‰ivitysv‰li)
TInt CSunClockAppView::PeriodicTimerCallBack(TAny* aAny)
{
	CSunClockAppView* self = static_cast<CSunClockAppView*>( aAny );
 
				// Tarvitsemme nykyist‰ aikaa t‰h‰n funktioon myˆs
	TTime now;
	now.HomeTime();
	TDateTime dateTime = now.DateTime();
	TInt hours = dateTime.Hour();

				// Muutetaan minuuteiksi
	TInt minutes = dateTime.Minute();
	TInt seconds = dateTime.Second();

				// Kun p‰iv‰ vaihtuu (annetaan kaksi sekuntia k‰sittelylle)
	if ((hours == 00) && (minutes == 00) && (seconds <= 2))
	{
			// P‰ivitet‰‰n auringonvaloajat
		self->RSunlight();
			// Piirret‰‰n view
		self->DrawNow();
	}
        // Cancel the timer when the callback should not be called again.
        // Call: self->iPeriodicTimer->Cancel();
 
	return KErrNone; // Return value ignored by CPeriodic
}


				// Ajastimen funktio (216 sekunnin eli 3,6 minuutin p‰ivitysv‰li)
TInt CSunClockAppView::PeriodicTimerCallBack2(TAny* aAny)
{
	CSunClockAppView* self = static_cast<CSunClockAppView*>( aAny );
 
        // Piirret‰‰n view
	self->DrawNow();
 
        // Cancel the timer when the callback should not be called again.
        // Call: self->iPeriodicTimer->Cancel();
 
	return KErrNone; // Return value ignored by CPeriodic
}

				// Ajastimen funktio (35 minuutin p‰ivitysv‰li)
TInt CSunClockAppView::PeriodicTimerCallBack3(TAny* aAny)
{
	CSunClockAppView* self = static_cast<CSunClockAppView*>( aAny );
 
        // Piirret‰‰n view
	self->DrawNow();
 
        // Cancel the timer when the callback should not be called again.
        // Call: self->iPeriodicTimer->Cancel();
 
	return KErrNone; // Return value ignored by CPeriodic
}