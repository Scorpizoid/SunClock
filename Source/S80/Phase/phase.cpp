/* standalone moon phase calculation */ 

#include <stdio.h>
#include <math.h>

#define		PI	3.1415926535897932384626433832795
#define		RAD	(PI/180.0)
#define     SMALL_FLOAT	(1e-12)




double 
Julian(int year,int month,double day)
{
    /*
      Returns the number of julian days for the specified day.
      */
    
    int a,b,c,e;
    if (month < 3) {
	year--;
	month += 12;
    }
    if (year > 1582 || (year == 1582 && month>10) ||
	(year == 1582 && month==10 && day > 15)) {
	a=year/100;
	b=2-a+a/4;
    }
    c = 365.25*year;
    e = 30.6001*(month+1);
    return b+c+e+day+1720994.5;
}

double sun_position(double j)
{
    double n,x,e,l,dl,v;
    double m2;
    int i;

    n=360/365.2422*j;
    i=n/360;
    n=n-i*360.0;
    x=n-3.762863;
    if (x<0) x += 360;
    x *= RAD;
    e=x;
    do {
	dl=e-.016718*sin(e)-x;
	e=e-dl/(1-.016718*cos(e));
    } while (fabs(dl)>=SMALL_FLOAT);
    v=360/PI*atan(1.01686011182*tan(e/2));
    l=v+282.596403;
    i=l/360;
    l=l-i*360.0;
    return l;
}

double moon_position(double j, double ls)
{
    
    double ms,l,mm,n,ev,sms,z,x,lm,bm,ae,ec;
    double d;
    double ds, as, dm;
    int i;
    
    /* ls = sun_position(j) */
    ms = 0.985647332099*j - 3.762863;
    if (ms < 0) ms += 360.0;
    l = 13.176396*j + 64.975464;
    i = l/360;
    l = l - i*360.0;
    if (l < 0) l += 360.0;
    mm = l-0.1114041*j-349.383063;
    i = mm/360;
    mm -= i*360.0;
    n = 151.950429 - 0.0529539*j;
    i = n/360;
    n -= i*360.0;
    ev = 1.2739*sin((2*(l-ls)-mm)*RAD);
    sms = sin(ms*RAD);
    ae = 0.1858*sms;
    mm += ev-ae- 0.37*sms;
    ec = 6.2886*sin(mm*RAD);
    l += ev+ec-ae+ 0.214*sin(2*mm*RAD);
    l= 0.6583*sin(2*(l-ls)*RAD)+l;
    return l;
}

double moon_phase(int year,int month,int day, double hour, int* ip)
{
    /*
      Calculates more accurately than Moon_phase , the phase of the moon at
      the given epoch.
      returns the moon phase as a real number (0-1)
      */

    double j= Julian(year,month,(double)day+hour/24.0)-2444238.5;
    double ls = sun_position(j);
    double lm = moon_position(j, ls);

    double t = lm - ls;
    if (t < 0) t += 360;
    *ip = (int)((t + 22.5)/45) & 0x7;
    return (1.0 - cos((lm - ls)*RAD))/2;
}

main()
{
    int y, m, d;
    int m0;
    int h;

    d = 1;
    m0 = m;

    printf("\nPhase\n");
    double p;
    int ip;
    p = moon_phase(2009, 11, 1, 20, &ip);//y, m, d, h, &ip);
	double results = floor(p*1000+0.5)/10;
    printf("%3.1f%%\n",results);//floor(p*1000+0.5)/10);
    return 0;
}
