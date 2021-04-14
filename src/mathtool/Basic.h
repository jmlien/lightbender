//------------------------------------------------------------------------------
//  Copyright 2007-2014 by Jyh-Ming Lien and George Mason University
//  See the file "LICENSE" for more information
//------------------------------------------------------------------------------


#ifndef _H_UTILITY
#define _H_UTILITY

#include <cmath>
#include <cassert>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <string>
#include <iostream>   // define C++ stream I/O routines
#include <iomanip>
using namespace std;

namespace mathtool
{

	// Miscellaneous Scalar Math 
	template<class T> T abs(T x)
	{
		return     (((x) < 0) ? (-(x)) : (x));
	}

	template<class T> T sqr(T x)
	{
		return ((x)* (x));
	}


	template<class T> int round(T x, int p)
	{
		return (long int)(((long int)((x)*std::pow(10.0, p) + ((x)<0 ? -0.5 : 0.5))) / std::pow(10.0, p));
	}

	template<class T> int round(T v)
	{
		int integer = (int)floor((long int)(v));
		T fraction = v - integer;

		if (v>0)
			return (fraction >= 0.5) ? integer + 1 : integer;
		else
			return (fraction >= -0.5) ? integer : integer + 1;
	}

	template<class T>  int sign(T x)
	{
		return ((x) >= 0 ? 1 : -1);
	}

	//#define swap(x1, x2)  {int tmp=x1; x1=x2; x2=tmp}
	template<class T, class S> T applysign(T x, S y)
	{
		return ((y) >= 0 ? abs(x) : -abs(x));
	}

	// Angle Conversions & Constants

    #ifndef PI
    #define PI 3.1415926535897f
    #endif

    #ifndef PI2
    #define PI2 6.2831853071794f
    #endif

    #define RAD2DEG (180/PI)
    #define DEG2RAD (PI/180)

    #define DegToRad(x) ((x)*DEG2RAD)
    #define RadToDeg(x) ((x)*RAD2DEG)

	/* range of real numbers */
	#ifndef SMALLNUMBER
	#define SMALLNUMBER 1.0e-10
	#endif

	#ifndef HUGENUMBER
	#define HUGENUMBER  1.0e10
	#endif


	//computes sqrt(a^2 + b^2) without destructive underflow or overflow
	template<class T> T pythag(T a, T b)
	{
		T absa, absb;

		absa = fabs(a);
		absb = fabs(b);

		if (absa > absb)
			return absa * sqrt(1.0 + sqr(absb / absa));
		else if (absb > 0)
			return absb * sqrt(1.0 + sqr(absa / absb));
		else
			return 0;
	}

    // Utility Error message routines
    // print s to stdout with trailing blank and no terminating end of line
	// Utility message routines
	inline void prompt(char *s)
	{
		cout << s << " ";
	}


    // print s1, s2, s3 to stdout as blank separated fields with terminating eol
	inline void message(char *s1, char *s2 = NULL, char *s3 = NULL)
	{
		cout << s1;
		if (s2 != NULL && strlen(s2) > 0)
			cout << " " << s2;
		if (s3 != NULL && strlen(s3) > 0)
			cout << " " << s3;
		cout << endl;
	}

	

    // print Status: to stdout followed by message(s1, s2, s3)
	inline void status(char *s1, char *s2=NULL, char *s3=NULL)
	{
		cout << "Status: ";
		message(s1, s2, s3);
	}



    // print Error: followed by s1, s2 and s3 to stderr as blank separated fields 
    // with terminating eol
	inline void error(char *s1, char *s2 = NULL, char *s3 = NULL)
	{
		cerr << "Error: ";
		cerr << s1;
		if (s2 != NULL && strlen(s2) > 0)
			cerr << " " << s2;
		if (s3 != NULL && strlen(s3) > 0)
			cerr << " " << s3;
		cerr << endl;
	}


    // print error(s1, s2, s3) and then exit program with code 1 
	inline void abort(char *s1, char *s2 = NULL, char *s3 = NULL)
	{
		error(s1, s2, s3);
		exit(1);
	}

    ///Added by Jyh-Ming Lien

    #ifdef WIN32

    ////////////////////////////////////////////////////////////////////////////////////////
    // Following functions define M_PI and drand48, which are not starndard c library and 
    // definitions. In addition, rint used to round off float points to int is also here.
    /////////////////////////////////////////////////////////////////////////////////////////

    #define M_PI 3.1415926 //reference PI above

    extern "C" {
        //Implementation of these functions are located in util.cpp
        double drand48();
        double erand48(register unsigned short *xsubi);
        long irand48(register unsigned short m);
        long krand48(register unsigned short *xsubi, unsigned short m);
        long lrand48();
        long mrand48();
        static void next();
        void srand48(long seedval);
        unsigned short * seed48(unsigned short seed16v[3]);
        void lcong48(unsigned short param[7]);
        long nrand48(register unsigned short *xsubi);
        long jrand48(register unsigned short *xsubi);

        /**Round to closest integer.
          *The rint() function rounds x to an integer value according
          *to the prevalent rounding mode.  The default rounding mode
          *is to round to the nearest integer.
          *@return The  rint() function returns the integer value as a float-
          *ing-point number.
          */
        double rint(double x);

    } //end extern "C"

    #endif //_WIN32

} //end of nprmlib

#endif
