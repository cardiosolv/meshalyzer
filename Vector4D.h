// $Id: Vector4D.h,v 1.1 2004/02/06 21:50:57 vigmond Exp $
// (c) Ingmar Bitter '99

#ifndef _Vector4D_h_	// prevent multiple includes
#define _Vector4D_h_

#include "Global.h"
#include <sstream>
#include <iostream>

template <class T> class Vector4D
{
  public:
    T e[4];

  public:
    static void	Demo (int, char**);

    // constructors & destructors
    inline Vector4D<T> () { e[0] = e[1] = e[2] = e[3] = 0; }
    inline Vector4D<T> (const Vector4D<T> & v)
    {
      e[0]= static_cast<T>(v.e[0]);
      e[1]= static_cast<T>(v.e[1]);
      e[2]= static_cast<T>(v.e[2]);
      e[3]= static_cast<T>(v.e[3]);
    }
    inline Vector4D<T> (const T & x, const T & y, const T & z, const T & w) {  e[0]=x; e[1]=y; e[2]=z; e[3]=w; }
    template <class C> inline Vector4D<T> (const Vector4D<C> & v)
    {
      e[0]= static_cast<T>(v.e[0]);
      e[1]= static_cast<T>(v.e[1]);
      e[2]= static_cast<T>(v.e[2]);
      e[3]= static_cast<T>(v.e[3]);
    }

    inline friend istream & operator >> (istream & is, Vector4D<T> & v)
    {
      is >> v.X() >> v.Y() >> v.Z() >> v.W();  return is;
    }
    inline Vector4D<T> & operator << (const char * str);
    inline friend ostream & operator << (ostream & os, const Vector4D<T> & v) { return v.asOstream(os); }
    inline ostream & asOstream (ostream & os) const;

    inline double Norm  ( )       const { return sqrt(X()*X() + Y()*Y() + Z()*Z() + W()*W()); }
    inline double SquareLength( ) const { return     (X()*X() + Y()*Y() + Z()*Z() + W()*W()); }
    inline double Length( )       const { return sqrt(X()*X() + Y()*Y() + Z()*Z() + W()*W()); }
    inline Vector4D<T> & Normalize ( );

    inline T X() const { return e[0]; }
    inline T Y() const { return e[1]; }
    inline T Z() const { return e[2]; }
    inline T W() const { return e[3]; }

    inline T R() const { return e[0]; }
    inline T G() const { return e[1]; }
    inline T B() const { return e[2]; }
    inline T A() const { return e[3]; }

    inline T & X() { return e[0]; }
    inline T & Y() { return e[1]; }
    inline T & Z() { return e[2]; }
    inline T & W() { return e[3]; }

    inline T & R() { return e[0]; }
    inline T & G() { return e[1]; }
    inline T & B() { return e[2]; }
    inline T & A() { return e[3]; }

    inline T * ptr() { return e; }
    inline T * ptr(int index) { return &e[index]; }
    inline T & operator [] (const int index)       { return e[index]; }
    inline T   operator [] (const int index) const { return e[index]; }

    inline Vector4D<T> & operator()(T x, T y, T z, T w) { e[0]=x; e[1]=y; e[2]=z; e[3]=w;  return *this; }

    // computation functions
    inline bool operator == (const Vector4D<T> &v) const { return ( X()==v.X() && Y()==v.Y() && Z()==v.Z() && W()==v.W() ); }
    inline bool operator != (const Vector4D<T> &v) const { return ( X()!=v.X() || Y()!=v.Y() || Z()!=v.Z() || W()!=v.W() ); }

    inline Vector4D<T> operator + () const { return *this; }
    inline Vector4D<T> operator - () const { return Vector4D<T>(-X(), -Y(), -Z(), -W()); }

    inline Vector4D<T>   operator +  (const Vector4D<T> & v) const { return Vector4D<T>(X()+v.X(), Y()+v.Y(), Z()+v.Z(), W()+v.W()); }
    inline Vector4D<T>   operator -  (const Vector4D<T> & v) const { return Vector4D<T>(X()-v.X(), Y()-v.Y(), Z()-v.Z(), W()-v.W()); }
    inline Vector4D<T> & operator += (const Vector4D<T> & v) { X()+=v.X(); Y()+=v.Y(); Z()+=v.Z(); W()+=v.W(); return *this;  }
    inline Vector4D<T> & operator -= (const Vector4D<T> & v) { X()-=v.X(); Y()-=v.Y(); Z()-=v.Z(); W()-=v.W(); return *this;  }

    inline friend Vector4D<T> operator * (const T & a, const Vector4D<T> & v) { return v*a; }
    inline Vector4D<T>   operator *  (const T & a) const { return Vector4D<T>(X()*a, Y()*a, Z()*a, W()*a); }
    inline Vector4D<T>   operator /  (const T & a) const { return Vector4D<T>(X()/a, Y()/a, Z()/a, W()/a); }
    inline Vector4D<T> & operator *= (const T & a) { X()*=a; Y()*=a; Z()*=a; W()*=a; return *this; }
    inline Vector4D<T> & operator /= (const T & a) { X()/=a; Y()/=a; Z()/=a; W()/=a; return *this; }

    inline T Dot        (const Vector4D<T> & v) const { return DotProduct(v); }
    inline T DotProduct (const Vector4D<T> & v) const { return (X()*v.X() + Y()*v.Y() + Z()*v.Z() + W()*v.W()); }

	operator float(){return Norm();}
}
; // class Vector4D

typedef Vector4D<char>   V4c;
typedef Vector4D<short>  V4s;
typedef Vector4D<int>    V4i;
typedef Vector4D<long>   V4l;
typedef Vector4D<float>  V4f;
typedef Vector4D<double> V4d;

template <class T> Vector4D<T> & Vector4D<T>::operator << (const char * str)
{
  // read vector from string e.g. "2 3 -1 1"
  stringstream data;
  if (str[0] == '"') { // string has double quotes -> stripp before copy
    char buf[256]; strcpy(buf, &str[1]); buf[strcspn(buf,"\"")] = 0; data << buf;
  } else { data << str; } // str has no double quotes -> just copy
  double tmp; for (int k=0; k<4; ++k) { data>>tmp; e[k]=T(tmp); }
  return *this;
} // operator <<

template <class T> ostream & Vector4D<T>::asOstream (ostream & o) const
{
  o<<"(";
  o.width(2); o<<X()<<",";
  o.width(2); o<<Y()<<",";
  o.width(2); o<<Z()<<")";
  o.width(2); o<<W()<<")";
  return o;
}

template <class T> Vector4D<T> & Vector4D<T>::Normalize ( )
{
  double length = sqrt(X()*X() + Y()*Y() + Z()*Z() + W()*W());
  if (length > 0) *this /= length; return *this;
} // Normalize

#endif // $Id: Vector4D.h,v 1.1 2004/02/06 21:50:57 vigmond Exp $
