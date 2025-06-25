/* standalone moon phase calculation */ 

#include "SunClock.h"


MoonPhase::MoonPhase()
{
	iPI = 3.1415926535897932384626433832795;
	iRAD = (iPI/180.0);
    iSMALL_FLOAT = (1e-12);
}

TReal MoonPhase::Julian(TInt year,TInt month,TReal day)
{
    /*
      Returns the number of julian days for the specified day.
      */
    
    TInt a, b, c, e;
    if (month < 3) {
		year--;
		month += 12;
    }
    if (year > 1582 || (year == 1582 && month>10) ||
	(year == 1582 && month==10 && day > 15)) {
		a=year/100;
		b=2-a+a/4;
    }
    c = 365.25 * year;
    e = 30.6001 * (month+1);
    return b+c+e+day+1720994.5;
}

TReal MoonPhase::sun_position(TReal j)
{
    TReal n,x,e,l,dl,v;
    TInt i;

    n=360/365.2422*j;
    i=n/360;
    n=n-i*360.0;
    x=n-3.762863;
    if (x<0)
	{
		x += 360;
	}
    x *= iRAD;
    e=x;
    do {
		dl=e-.016718*sin(e)-x;
		e=e-dl/(1-.016718*cos(e));
    } while (fabs(dl)>= iSMALL_FLOAT);
    v=360/iPI*atan(1.01686011182*tan(e/2));
    l=v+282.596403;
    i=l/360;
    l=l-i*360.0;
    return l;
}

TReal MoonPhase::moon_position(TReal j, TReal ls)
{
    
    TReal ms,l,mm,n,ev,sms,ae,ec;
    TInt i;
    
    /* ls = sun_position(j) */
    ms = 0.985647332099*j - 3.762863;
    if (ms < 0)
	{
		ms += 360.0;
	}
    l = 13.176396*j + 64.975464;
    i = l/360;
    l = l - i*360.0;
    if (l < 0)
	{
		l += 360.0;
	}
    mm = l-0.1114041*j-349.383063;
    i = mm/360;
    mm -= i*360.0;
    n = 151.950429 - 0.0529539*j;
    i = n/360;
    n -= i*360.0;
    ev = 1.2739*sin((2*(l-ls)-mm)*iRAD);
    sms = sin(ms*iRAD);
    ae = 0.1858*sms;
    mm += ev-ae- 0.37*sms;
    ec = 6.2886*sin(mm*iRAD);
    l += ev+ec-ae+ 0.214*sin(2*mm*iRAD);
    l= 0.6583*sin(2*(l-ls)*iRAD)+l;
    return l;
}

TReal MoonPhase::moon_phase(TInt year,TInt month,TInt day, TReal hour)
{
    /*
      Calculates more accurately than Moon_phase , the phase of the moon at
      the given epoch.
      returns the moon phase as a real number (0-1)
      */

    TReal j= Julian(year,month,(TReal)day+hour/24.0) - 2444238.5;
    TReal ls = sun_position(j);
    TReal lm = moon_position(j, ls);

    TReal t = lm - ls;
    if (t < 0) t += 360;
    return (1.0 - cos((lm - ls)*iRAD))/2;
}
