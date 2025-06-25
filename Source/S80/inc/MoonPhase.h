// MoonPhase.h
//
////////////////////////////////////////////////////////////////////
// MoonPhase 
// ----------
//
//
////////////////////////////////////////////////////////////////////
#ifndef __MOONPHASE_H
#define __MOONPHASE_H


////////////////////////////////////////////////////////////////////////
//
// CRegionDlg eli nykyisen alueen asetusdialogi
//
////////////////////////////////////////////////////////////////////////

class MoonPhase {
public: 
	MoonPhase();
	TReal moon_phase(TInt year,TInt month,TInt day, TReal hour);

private:
	TReal Julian(TInt year,TInt month,TReal day);
	TReal sun_position(TReal j);
	TReal moon_position(TReal j, TReal ls);
	TReal iPI;
	TReal iRAD;
	TReal iSMALL_FLOAT;
};

#endif



