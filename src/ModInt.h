// ModInt.h
// (c) Ingmar Bitter '96-'99 / Hanspeter Pfister '97
// $Id: ModInt.h,v 1.1 2004/02/06 21:50:57 vigmond Exp $

#ifndef _ModInt_h_		// prevent multiple includes
#define _ModInt_h_

#include "Global.h"
#include <limits.h>


class ModInt
{
  protected:
    int n;
    unsigned int m;
  public:
    static void	Demo(int , char **);

    // constructors & destructors
    inline ModInt (const int k=0, const unsigned int modulus=INT_MAX) : n(MyMod(k, modulus)), m(modulus) { }
    inline ModInt (const ModInt & k) : n(k.n), m(k.m) { }

    // casts to other data types
    inline operator int() const { return n; }

    // show/set data & data properties
    inline friend ostream & operator << (ostream & os, const ModInt k) { return k.Ostream(os); }
    inline ostream & Ostream (ostream & os) const { return (os << n <<":"<< m); }
    inline double Norm ( ) const { return double(n); }
  inline ModInt Abs ( ) const { return ModInt((n<0) ? -n : n); }

    inline ModInt & operator () (const int k) { n = MyMod(k, m); return *this; }
    inline ModInt & operator () (const int k, const unsigned int modulus)
    {
      n = MyMod(k, modulus); m = modulus; return *this;
    }

    // Unary operators
    inline ModInt   operator +  ( ) const { return ModInt(n);  }
    inline ModInt   operator -  ( ) const { return ModInt(-n); }
    inline ModInt & operator ++ ( )   { n = ++n % m; return *this; }
    inline ModInt & operator ++ (int) { n = ++n % m; return *this; }
    inline ModInt & operator -- ( )   { n = MyMod(--n, m); return *this; }
    inline ModInt & operator -- (int) { n = MyMod(--n, m); return *this; }

    // ModInt op ModInt
    inline ModInt & operator =  (const ModInt k) { n=k.n; m=k.m; return *this; }
    inline ModInt & operator += (const ModInt k) { n = MyMod((n+k.n), m); return *this; }
    inline ModInt & operator -= (const ModInt k) { n = MyMod((n-k.n), m); return *this; }
    inline ModInt & operator *= (const ModInt k) { n = MyMod((n*k.n), m); return *this; }
    inline ModInt & operator /= (const ModInt k) { n = MyMod((n/k.n), m); return *this; }
    inline ModInt   operator +  (const ModInt k)	const { return ModInt(n+k.n, m); }
    inline ModInt   operator -  (const ModInt k)	const { return ModInt(n-k.n, m); }
    inline ModInt   operator *  (const ModInt k)	const { return ModInt(n*k.n, m); }
    inline ModInt   operator /  (const ModInt k)	const { return ModInt(n/k.n, m); }

    // ModInt op double
    inline ModInt & operator =  (const double x) { n = MyMod(x, m); return *this; }
    inline ModInt & operator += (const double x) { n = MyMod((n+x), m); return *this; }
    inline ModInt & operator -= (const double x) { n = MyMod((n-x), m); return *this; }
    inline ModInt & operator *= (const double x) { n = MyMod((n*x), m); return *this; }
    inline ModInt & operator /= (const double x) { n = MyMod((n/x), m); return *this; }
    inline ModInt   operator +  (const double x)	const { return ModInt(n+x, m); }
    inline ModInt   operator -  (const double x)	const { return ModInt(n-x, m); }
    inline ModInt   operator *  (const double x)	const { return ModInt(n*x, m); }
    inline ModInt   operator /  (const double x)	const { return ModInt(n/x, m); }

    // double op ModInt
    inline friend double & operator +=  (double & x, const ModInt k) { return x+=k.n; }
    inline friend double & operator -=  (double & x, const ModInt k) { return x-=k.n; }
    inline friend double & operator *=  (double & x, const ModInt k) { return x*=k.n; }
    inline friend double & operator /=  (double & x, const ModInt k) { return x/=k.n; }
    inline friend double	 operator +   (const double x, const ModInt k) { return x+k.n; }

    inline friend double	 operator -   (const double x, const ModInt k) { return x-k.n; }
    inline friend double	 operator *   (const double x, const ModInt k) { return x*k.n; }
    inline friend double	 operator /   (const double x, const ModInt k) { return x/k.n; }

    // ModInt op int
    inline ModInt & operator =  (const int i) { n = MyMod(i, m); return *this; }
    inline ModInt & operator += (const int i) { n = MyMod((n+i), m); return *this; }
    inline ModInt & operator -= (const int i) { n = MyMod((n-i), m); return *this; }
    inline ModInt & operator *= (const int i) { n = MyMod((n*i), m); return *this; }
    inline ModInt & operator /= (const int i) { n = MyMod((n/i), m); return *this; }
    inline ModInt   operator +  (const int i) const { return ModInt(n+i, m); }
    inline ModInt   operator -  (const int i) const { return ModInt(n-i, m); }
    inline ModInt   operator *  (const int i) const { return ModInt(n*i, m); }
    inline ModInt   operator /  (const int i) const { return ModInt(n/i, m); }

    // int op ModInt
    inline friend int & operator +=  (int & i, const ModInt k) { return i+=k.n; }
    inline friend int & operator -=  (int & i, const ModInt k) { return i-=k.n; }
    inline friend int & operator *=  (int & i, const ModInt k) { return i*=k.n; }
    inline friend int & operator /=  (int & i, const ModInt k) { return i/=k.n; }
    inline friend int	operator + (const int i, const ModInt k) { return i+k.n;	}
    inline friend int operator - (const int i, const ModInt k) { return i-k.n; }
    inline friend int operator * (const int i, const ModInt k) { return i*k.n; }
    inline friend int operator / (const int i, const ModInt k) { return i/k.n; }

    // ModInt op long
    inline ModInt & operator =  (const long i) { n = MyMod(   i,  m); return *this; }
    inline ModInt & operator += (const long i) { n = MyMod((n+i), m); return *this; }
    inline ModInt & operator -= (const long i) { n = MyMod((n-i), m); return *this; }
    inline ModInt & operator *= (const long i) { n = MyMod((n*i), m); return *this; }
    inline ModInt & operator /= (const long i) { n = MyMod((n/i), m); return *this; }
    inline ModInt   operator +  (const long i)	const { return ModInt(n+i, m); }
    inline ModInt   operator -  (const long i)	const { return ModInt(n-i, m); }
    inline ModInt   operator *  (const long i)	const { return ModInt(n*i, m); }
    inline ModInt   operator /  (const long i)	const { return ModInt(n/i, m); }

    // long op ModInt
    inline friend long& operator += (long & i, const ModInt k) { return i+=k.n; }
    inline friend long& operator -= (long & i, const ModInt k) { return i-=k.n; }
    inline friend long& operator *= (long & i, const ModInt k) { return i*=k.n; }
    inline friend long& operator /= (long & i, const ModInt k) { return i/=k.n; }
    inline friend long operator + (const long i, const ModInt k) { return i+k.n; }
    inline friend long operator - (const long i, const ModInt k) { return i-k.n; }
    inline friend long operator * (const long i, const ModInt k) { return i*k.n; }
    inline friend long operator / (const long i, const ModInt k) { return i/k.n; }

  private:
    int MyMod(int n, unsigned int m) { while ( n < 0 ) { n += m; } return ( n % m ); }
}
; // class MODINT


#endif	// _ModInt_h_

