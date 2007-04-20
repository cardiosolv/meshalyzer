// $Id: Global.h,v 1.2 2005/06/11 16:17:53 vigmond Exp $
// (c) Ingmar Bitter '96-'99

#ifndef _Global_h_		// prevent multiple includes
#define _Global_h_

// include most common libraries
// (no ".h" to ensure use of new C++ standard)
#include <string>   // string

#if defined(__sgi) && !defined(__GNUC__)
#include <mstring.h>
#include <assert.h>  // assert
#include <stdlib.h>  // abort
#include <iostream.h> // cout, cerr, ostream
#else
#include <cassert>  // assert
#include <cstdlib>  // abort
#include <iostream> // cout, cerr, ostream
#endif
#include <stdio.h> // sprintf

using namespace std;

inline string & operator << (string & s, const string & add) { return s += add; }
inline string & operator << (string & s, const char * add) { string tmp(add); return s<<tmp; }
inline string & operator << (string & s, const int & k) { static char num[30]; sprintf(num,"%i", k); return s+=num; }
inline string & operator << (string & s, const long & k) { static char num[30]; sprintf(num,"%li", k); return s+=num; }
inline string & operator << (string & s, const float  & f) { static char num[30]; sprintf(num,"%f", f); return s+=num; }
inline string & operator << (string & s, const double & d) { static char num[30]; sprintf(num,"%f",d); return s+=num; }
//template <class T> inline string & operator << (string & s, const T & t) { s += t; return s; }
//inline char * c_str(const double & d) { static char num[100]; sprintf(num,"%f",d); return num; } // needed to fix above template when using doubles

static const char * const BoolStr[] = {"Oh No ! :-( ",":-) "};
static void Dummy() { cout<<BoolStr[0]; Dummy(); }

typedef unsigned char uchar;

template<class T> inline const T  Abs (const T& a) { return (a<0) ? a : -a; }
template<class T> inline const T& Max (const T& a, const T& b) { return (a>b) ? a : b; }
template<class T> inline const T& Min (const T& a, const T& b) { return (a<b) ? a : b; }
template<class T> inline const T& Max (const T& a, const T& b, const T& c) { return (a>b) ? Max(a,c) : Max(b,c); }
template<class T> inline const T& Min (const T& a, const T& b, const T& c) { return (a<b) ? Min(a,c) : Min(b,c); }
template<class T> inline const T& Mid (const T& a, const T& b, const T& c) { return (a<b) ? ((b<c)?b:Max(a,c))
                                                                               :            ((b>c)?b:Min(a,c)); }
template<class T> inline const T& Bound(const T&a, const T& b, const T& c) { return (a<b) ? ((b<c)?b:c)
                                                                               :            ((b>c)?b:c); }
#define DB(var,msg) { cerr <<" "<<var<<"="<<msg; }
#define ERROR(msg) { cerr << endl << "###Error### in file " << __FILE__ << " at line " << __LINE__ << endl; cerr << (msg) << endl; cerr << "Exiting ..." << endl; abort(); }

#ifdef sgi
// avoid following warning
// "/usr/include/CC/iostream.h", line 675: remark(1174):
// variable "iostream_init" was declared but never referenced
//  } iostream_init ;	
#endif

#endif // $Id: Global.h,v 1.2 2005/06/11 16:17:53 vigmond Exp $
