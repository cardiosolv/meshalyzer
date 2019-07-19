// Matrix4x4.h
// (c) Ingmar Bitter '96-'99
// $Id: Matrix4x4.h,v 1.2 2005/09/21 20:08:05 vigmond Exp $

#ifndef _Matrix4x4_h_	// prevent multiple includes
#define _Matrix4x4_h_

#include "Global.h"
#include "Vector3D.h"

// Matrix stored Type matrix[colums][rows]
// second element is second entry of first row denoted by e[1][0]
// indices are viewed as x/y-coords of the positions in the matrix
// thus the first entry is colum number, second is row number

template <class T> class Matrix4x4
{
  public:
    union {
      T e[4][4];
      T m[16];
    };
  public:
    static void Demo (int, char **);

    // constructors & destructors
    inline Matrix4x4 ()
    {
      e[0][0] = 1;  e[1][0] = 0;  e[2][0] = 0;  e[3][0] = 0;
      e[0][1] = 0;  e[1][1] = 1;  e[2][1] = 0;  e[3][1] = 0;
      e[0][2] = 0;  e[1][2] = 0;  e[2][2] = 1;  e[3][2] = 0;
      e[0][3] = 0;  e[1][3] = 0;  e[2][3] = 0;  e[3][3] = 1;
    }
    inline Matrix4x4 (const T ax, const T bx, const T cx, const T dx,
                      const T ay, const T by, const T cy, const T dy,
                      const T az, const T bz, const T cz, const T dz,
                      const T aw, const T bw, const T cw, const T dw)
    {
      e[0][0] = ax;  e[1][0] = bx;  e[2][0] = cx;  e[3][0] = dx;
      e[0][1] = ay;  e[1][1] = by;  e[2][1] = cy;  e[3][1] = dy;
      e[0][2] = az;  e[1][2] = bz;  e[2][2] = cz;  e[3][2] = dz;
      e[0][3] = aw;  e[1][3] = bw;  e[2][3] = cw;  e[3][3] = dw;
    }
    inline Matrix4x4 (const Matrix4x4<T> & m)
    {
      e[0][0] = m.e[0][0]; e[1][0] = m.e[1][0]; e[2][0] = m.e[2][0]; e[3][0] = m.e[3][0];
      e[0][1] = m.e[0][1]; e[1][1] = m.e[1][1]; e[2][1] = m.e[2][1]; e[3][1] = m.e[3][1];
      e[0][2] = m.e[0][2]; e[1][2] = m.e[1][2]; e[2][2] = m.e[2][2]; e[3][2] = m.e[3][2];
      e[0][3] = m.e[0][3]; e[1][3] = m.e[1][3]; e[2][3] = m.e[2][3]; e[3][3] = m.e[3][3];
    }

    template <class C> inline Matrix4x4 (const Matrix4x4<C> & m)
    {
      e[0][0] = static_cast<T>(m.e[0][0]);
      e[1][0] = static_cast<T>(m.e[1][0]);
      e[2][0] = static_cast<T>(m.e[2][0]);
      e[3][0] = static_cast<T>(m.e[3][0]);
      e[0][1] = static_cast<T>(m.e[0][1]);
      e[1][1] = static_cast<T>(m.e[1][1]);
      e[2][1] = static_cast<T>(m.e[2][1]);
      e[3][1] = static_cast<T>(m.e[3][1]);
      e[0][2] = static_cast<T>(m.e[0][2]);
      e[1][2] = static_cast<T>(m.e[1][2]);
      e[2][2] = static_cast<T>(m.e[2][2]);
      e[3][2] = static_cast<T>(m.e[3][2]);
      e[0][3] = static_cast<T>(m.e[0][3]);
      e[1][3] = static_cast<T>(m.e[1][3]);
      e[2][3] = static_cast<T>(m.e[2][3]);
      e[3][3] = static_cast<T>(m.e[3][3]);
    }

//   template <class C> inline operator const Matrix4x4<C> () {
//     return  Matrix4x4<C>(C(e[0][0]), C(e[1][0]), C(e[2][0]), C(e[3][0]),
//                          C(e[0][1]), C(e[1][1]), C(e[2][1]), C(e[3][1]),
//                          C(e[0][2]), C(e[1][2]), C(e[2][2]), C(e[3][2]),
//                          C(e[0][3]), C(e[1][3]), C(e[2][3]), C(e[3][3]));
//   }

    // show/set data & data properties
    friend ostream & operator << (ostream & os, const Matrix4x4<T> & m) { return m.Ostream(os); }
    ostream & Ostream (ostream & os) const
    {
      int w=4; os.precision(2);
      os<<"{(";os.width(w); os<<e[0][0]<<","; os.width(w); os<<e[1][0]<<","; os.width(w); os<<e[2][0]<<","; os.width(w); os<<e[3][0]<<"), ";
      os<<"("; os.width(w); os<<e[0][1]<<","; os.width(w); os<<e[1][1]<<","; os.width(w); os<<e[2][1]<<","; os.width(w); os<<e[3][1]<<"), ";
      os<<"("; os.width(w); os<<e[0][2]<<","; os.width(w); os<<e[1][2]<<","; os.width(w); os<<e[2][2]<<","; os.width(w); os<<e[3][2]<<"), ";
      os<<"("; os.width(w); os<<e[0][3]<<","; os.width(w); os<<e[1][3]<<","; os.width(w); os<<e[2][3]<<","; os.width(w); os<<e[3][3]<<")}";
      return os;
    }

    inline Matrix4x4<T> &	Identity()
    {
      e[0][0] = 1;  e[1][0] = 0;  e[2][0] = 0;  e[3][0] = 0;
      e[0][1] = 0;  e[1][1] = 1;  e[2][1] = 0;  e[3][1] = 0;
      e[0][2] = 0;  e[1][2] = 0;  e[2][2] = 1;  e[3][2] = 0;
      e[0][3] = 0;  e[1][3] = 0;  e[2][3] = 0;  e[3][3] = 1;
      return *this;
    }

    inline void	SetOglMatrix(T m[16])
    {
      m[0] = e[0][0];  m[4] = e[1][0];  m[ 8] = e[2][0];  m[12] = e[3][0];
      m[1] = e[0][1];  m[5] = e[1][1];  m[ 9] = e[2][1];  m[13] = e[3][1];
      m[2] = e[0][2];  m[6] = e[1][2];  m[10] = e[2][2];  m[14] = e[3][2];
      m[3] = e[0][3];  m[7] = e[1][3];  m[11] = e[2][3];  m[15] = e[3][3];
    }

    inline Matrix4x4<T> &	operator ()
    (const T ax, const T bx, const T cx, const T dx,
     const T ay, const T by, const T cy, const T dy,
     const T az, const T bz, const T cz, const T dz,
     const T aw, const T bw, const T cw, const T dw)
    {
      e[0][0] = ax;  e[1][0] = bx;  e[2][0] = cx;  e[3][0] = dx;
      e[0][1] = ay;  e[1][1] = by;  e[2][1] = cy;  e[3][1] = dy;
      e[0][2] = az;  e[1][2] = bz;  e[2][2] = cz;  e[3][2] = dz;
      e[0][3] = aw;  e[1][3] = bw;  e[2][3] = cw;  e[3][3] = dw;
      return *this;
    }

    inline bool operator == (const Matrix4x4<T> & m) const
    {
      return
        e[0][0] == m.e[0][0] && e[1][0] == m.e[1][0] && e[2][0] == m.e[2][0] && e[3][0] == m.e[3][0] &&
        e[0][1] == m.e[0][1] && e[1][1] == m.e[1][1] && e[2][1] == m.e[2][1] && e[3][1] == m.e[3][1] &&
        e[0][2] == m.e[0][2] && e[1][2] == m.e[1][2] && e[2][2] == m.e[2][2] && e[3][2] == m.e[3][2] &&
        e[0][3] == m.e[0][3] && e[1][3] == m.e[1][3] && e[2][3] == m.e[2][3] && e[3][3] == m.e[3][3] ;
    }
    inline bool operator != (const Matrix4x4<T> & m) const
    {
      return
        e[0][0] != m.e[0][0] || e[1][0] != m.e[1][0] || e[2][0] != m.e[2][0] || e[3][0] != m.e[3][0] ||
        e[0][1] != m.e[0][1] || e[1][1] != m.e[1][1] || e[2][1] != m.e[2][1] || e[3][1] != m.e[3][1] ||
        e[0][2] != m.e[0][2] || e[1][2] != m.e[1][2] || e[2][2] != m.e[2][2] || e[3][2] != m.e[3][2] ||
        e[0][3] != m.e[0][3] || e[1][3] != m.e[1][3] || e[2][3] != m.e[2][3] || e[3][3] != m.e[3][3] ;
    }

    inline Matrix4x4<T>   operator +  ( ) const { return Matrix4x4<T>(*this); }
    inline Matrix4x4<T>   operator -  ( ) const
    {
      return Matrix4x4<T>(	-e[0][0], -e[1][0], -e[2][0], -e[3][0],
                           -e[0][1], -e[1][1], -e[2][1], -e[3][1],
                           -e[0][2], -e[1][2], -e[2][2], -e[3][2],
                           -e[0][3], -e[1][3], -e[2][3], -e[3][3]);
    }

    inline Matrix4x4<T>   operator +  (const Matrix4x4<T> & m) const
    {
      return
        (e[0][0] + m.e[0][0], e[1][0] + m.e[1][0], e[2][0] + m.e[2][0], e[3][0] + m.e[3][0],
         e[0][1] + m.e[0][1], e[1][1] + m.e[1][1], e[2][1] + m.e[2][1], e[3][1] + m.e[3][1],
         e[0][2] + m.e[0][2], e[1][2] + m.e[1][2], e[2][2] + m.e[2][2], e[3][2] + m.e[3][2],
         e[0][3] + m.e[0][3], e[1][3] + m.e[1][3], e[2][3] + m.e[2][3], e[3][3] + m.e[3][3]);
    }
    inline Matrix4x4<T>   operator -  (const Matrix4x4<T> & m) const
    {
      return
        (e[0][0] - m.e[0][0], e[1][0] - m.e[1][0], e[2][0] - m.e[2][0], e[3][0] - m.e[3][0],
         e[0][1] - m.e[0][1], e[1][1] - m.e[1][1], e[2][1] - m.e[2][1], e[3][1] - m.e[3][1],
         e[0][2] - m.e[0][2], e[1][2] - m.e[1][2], e[2][2] - m.e[2][2], e[3][2] - m.e[3][2],
         e[0][3] - m.e[0][3], e[1][3] - m.e[1][3], e[2][3] - m.e[2][3], e[3][3] - m.e[3][3]);
    }
    inline Matrix4x4<T> & operator += (const Matrix4x4<T> & m)
    {
      e[0][0] += m.e[0][0]; e[1][0] += m.e[1][0]; e[2][0] += m.e[2][0]; e[3][0] += m.e[3][0];
      e[0][1] += m.e[0][1]; e[1][1] += m.e[1][1]; e[2][1] += m.e[2][1]; e[3][1] += m.e[3][1];
      e[0][2] += m.e[0][2]; e[1][2] += m.e[1][2]; e[2][2] += m.e[2][2]; e[3][2] += m.e[3][2];
      e[0][3] += m.e[0][3]; e[1][3] += m.e[1][3]; e[2][3] += m.e[2][3]; e[3][3] += m.e[3][3];
      return *this;
    }
    inline Matrix4x4<T> & operator -= (const Matrix4x4<T> & m)
    {
      e[0][0] -= m.e[0][0]; e[1][0] -= m.e[1][0]; e[2][0] -= m.e[2][0]; e[3][0] -= m.e[3][0];
      e[0][1] -= m.e[0][1]; e[1][1] -= m.e[1][1]; e[2][1] -= m.e[2][1]; e[3][1] -= m.e[3][1];
      e[0][2] -= m.e[0][2]; e[1][2] -= m.e[1][2]; e[2][2] -= m.e[2][2]; e[3][2] -= m.e[3][2];
      e[0][3] -= m.e[0][3]; e[1][3] -= m.e[1][3]; e[2][3] -= m.e[2][3]; e[3][3] -= m.e[3][3];
      return *this;
    }

    inline friend Matrix4x4<T> operator * (const T & a, const Matrix4x4<T> &m) { return m*a; }
    inline Matrix4x4<T> operator * (const T & a) const
    {
      return Matrix4x4<T>
             (e[0][0] * a,	e[1][0] * a,	e[2][0] * a,	e[3][0] * a,
              e[0][1] * a,	e[1][1] * a,	e[2][1] * a,	e[3][1] * a,
              e[0][2] * a,	e[1][2] * a,	e[2][2] * a,	e[3][2] * a,
              e[0][3] * a,	e[1][3] * a,	e[2][3] * a,	e[3][3] * a);
    }
    inline Matrix4x4<T> operator / (const T & a) const
    {
      return Matrix4x4<T>
             (e[0][0] / a,	e[1][0] / a,	e[2][0] / a,	e[3][0] / a,
              e[0][1] / a,	e[1][1] / a,	e[2][1] / a,	e[3][1] / a,
              e[0][2] / a,	e[1][2] / a,	e[2][2] / a,	e[3][2] / a,
              e[0][3] / a,	e[1][3] / a,	e[2][3] / a,	e[3][3] / a);
    }
    inline Matrix4x4<T> & operator *= (const T & a)
    {
      e[0][0] *= a;	e[1][0] *= a;	e[2][0] *= a;	e[3][0] *= a;
      e[0][1] *= a;	e[1][1] *= a;	e[2][1] *= a;	e[3][1] *= a;
      e[0][2] *= a;	e[1][2] *= a;	e[2][2] *= a;	e[3][2] *= a;
      e[0][3] *= a;	e[1][3] *= a;	e[2][3] *= a;	e[3][3] *= a;
      return *this;
    }
    inline Matrix4x4<T> & operator /= (const T & a)
    {
      e[0][0] /= a;	e[1][0] /= a;	e[2][0] /= a;	e[3][0] /= a;
      e[0][1] /= a;	e[1][1] /= a;	e[2][1] /= a;	e[3][1] /= a;
      e[0][2] /= a;	e[1][2] /= a;	e[2][2] /= a;	e[3][2] /= a;
      e[0][3] /= a;	e[1][3] /= a;	e[2][3] /= a;	e[3][3] /= a;
      return *this;
    }

    inline Matrix4x4<T> operator *  (const Matrix4x4<T> & m) const
    {
      return Matrix4x4<T>
             (	e[0][0]*m.e[0][0] + e[1][0]*m.e[0][1] + e[2][0]*m.e[0][2] + e[3][0]*m.e[0][3],
               e[0][0]*m.e[1][0] + e[1][0]*m.e[1][1] + e[2][0]*m.e[1][2] + e[3][0]*m.e[1][3],
               e[0][0]*m.e[2][0] + e[1][0]*m.e[2][1] + e[2][0]*m.e[2][2] + e[3][0]*m.e[2][3],
               e[0][0]*m.e[3][0] + e[1][0]*m.e[3][1] + e[2][0]*m.e[3][2] + e[3][0]*m.e[3][3],

               e[0][1]*m.e[0][0] + e[1][1]*m.e[0][1] + e[2][1]*m.e[0][2] + e[3][1]*m.e[0][3],
               e[0][1]*m.e[1][0] + e[1][1]*m.e[1][1] + e[2][1]*m.e[1][2] + e[3][1]*m.e[1][3],
               e[0][1]*m.e[2][0] + e[1][1]*m.e[2][1] + e[2][1]*m.e[2][2] + e[3][1]*m.e[2][3],
               e[0][1]*m.e[3][0] + e[1][1]*m.e[3][1] + e[2][1]*m.e[3][2] + e[3][1]*m.e[3][3],

               e[0][2]*m.e[0][0] + e[1][2]*m.e[0][1] + e[2][2]*m.e[0][2] + e[3][2]*m.e[0][3],
               e[0][2]*m.e[1][0] + e[1][2]*m.e[1][1] + e[2][2]*m.e[1][2] + e[3][2]*m.e[1][3],
               e[0][2]*m.e[2][0] + e[1][2]*m.e[2][1] + e[2][2]*m.e[2][2] + e[3][2]*m.e[2][3],
               e[0][2]*m.e[3][0] + e[1][2]*m.e[3][1] + e[2][2]*m.e[3][2] + e[3][2]*m.e[3][3],

               e[0][3]*m.e[0][0] + e[1][3]*m.e[0][1] + e[2][3]*m.e[0][2] + e[3][3]*m.e[0][3],
               e[0][3]*m.e[1][0] + e[1][3]*m.e[1][1] + e[2][3]*m.e[1][2] + e[3][3]*m.e[1][3],
               e[0][3]*m.e[2][0] + e[1][3]*m.e[2][1] + e[2][3]*m.e[2][2] + e[3][3]*m.e[2][3],
               e[0][3]*m.e[3][0] + e[1][3]*m.e[3][1] + e[2][3]*m.e[3][2] + e[3][3]*m.e[3][3]);
    }
    inline Matrix4x4<T> & operator *= (const Matrix4x4<T> & m)
    {
      T a,b,c,d;
      a = e[0][0]; b = e[1][0]; c = e[2][0]; d = e[3][0];
      e[0][0] = a*m.e[0][0] + b*m.e[0][1] + c*m.e[0][2] + d*m.e[0][3];
      e[1][0] = a*m.e[1][0] + b*m.e[1][1] + c*m.e[1][2] + d*m.e[1][3];
      e[2][0] = a*m.e[2][0] + b*m.e[2][1] + c*m.e[2][2] + d*m.e[2][3];
      e[3][0] = a*m.e[3][0] + b*m.e[3][1] + c*m.e[3][2] + d*m.e[3][3];
      a = e[0][1]; b = e[1][1]; c = e[2][1]; d = e[3][1];
      e[0][1] = a*m.e[0][0] + b*m.e[0][1] + c*m.e[0][2] + d*m.e[0][3];
      e[1][1] = a*m.e[1][0] + b*m.e[1][1] + c*m.e[1][2] + d*m.e[1][3];
      e[2][1] = a*m.e[2][0] + b*m.e[2][1] + c*m.e[2][2] + d*m.e[2][3];
      e[3][1] = a*m.e[3][0] + b*m.e[3][1] + c*m.e[3][2] + d*m.e[3][3];
      a = e[0][2]; b = e[1][2]; c = e[2][2]; d = e[3][2];
      e[0][2] = a*m.e[0][0] + b*m.e[0][1] + c*m.e[0][2] + d*m.e[0][3];
      e[1][2] = a*m.e[1][0] + b*m.e[1][1] + c*m.e[1][2] + d*m.e[1][3];
      e[2][2] = a*m.e[2][0] + b*m.e[2][1] + c*m.e[2][2] + d*m.e[2][3];
      e[3][2] = a*m.e[3][0] + b*m.e[3][1] + c*m.e[3][2] + d*m.e[3][3];
      a = e[0][3]; b = e[1][3]; c = e[2][3]; d = e[3][3];
      e[0][3] = a*m.e[0][0] + b*m.e[0][1] + c*m.e[0][2] + d*m.e[0][3];
      e[1][3] = a*m.e[1][0] + b*m.e[1][1] + c*m.e[1][2] + d*m.e[1][3];
      e[2][3] = a*m.e[2][0] + b*m.e[2][1] + c*m.e[2][2] + d*m.e[2][3];
      e[3][3] = a*m.e[3][0] + b*m.e[3][1] + c*m.e[3][2] + d*m.e[3][3];
      return *this;
    }
    inline Vector3D<T> operator * (const Vector3D<T> & v) const
    {
      T w(e[0][3]*v.X() + e[1][3]*v.Y() + e[2][3]*v.Z() + e[3][3]);
      return Vector3D<T>
             (	(e[0][0]*v.X() + e[1][0]*v.Y() + e[2][0]*v.Z() + e[3][0])/w,
               (e[0][1]*v.X() + e[1][1]*v.Y() + e[2][1]*v.Z() + e[3][1])/w,
               (e[0][2]*v.X() + e[1][2]*v.Y() + e[2][2]*v.Z() + e[3][2])/w);
    }
    inline Vector3D<T> MultiplyDirectionVector(const Vector3D<T> & v) const
    {
      Matrix4x4<T> it = *this;
      it.e[0][3] = 0;
      it.e[1][3] = 0;
      it.e[2][3] = 0;
      it.e[3][3] = 1;
      it.e[3][2] = 0;
      it.e[3][1] = 0;
      it.e[3][0] = 0;
      it = it.Inverse().Transpose();
      return Vector3D<T>
             (	it.e[0][0]*v.X() + it.e[1][0]*v.Y() + it.e[2][0]*v.Z(),
               it.e[0][1]*v.X() + it.e[1][1]*v.Y() + it.e[2][1]*v.Z(),
               it.e[0][2]*v.X() + it.e[1][2]*v.Y() + it.e[2][2]*v.Z());
    }
    inline Matrix4x4<T> Trans       ( ) const { return Transpose(); }
    inline Matrix4x4<T> Transpose   ( ) const
    {
      return Matrix4x4<T>
             (e[0][0], e[0][1], e[0][2], e[0][3],
              e[1][0], e[1][1], e[1][2], e[1][3],
              e[2][0], e[2][1], e[2][2], e[2][3],
              e[3][0], e[3][1], e[3][2], e[3][3]);
    }
    inline Matrix4x4<T> & TransposeThis ( )
    {
      T t;
      t=e[0][1]; e[0][1]=e[1][0]; e[1][0]=t;
      t=e[0][2]; e[0][2]=e[2][0]; e[2][0]=t;
      t=e[0][3]; e[0][3]=e[3][0]; e[3][0]=t;

      t=e[1][2]; e[1][2]=e[2][1]; e[2][1]=t;
      t=e[1][3]; e[1][3]=e[3][1]; e[3][1]=t;

      t=e[2][3]; e[2][3]=e[3][2]; e[3][2]=t;
      return *this;
    }
    inline T Determinant ( ) const
    {
      Matrix4x4<T> adj = Adjoint();
      return
        + e[0][0] * adj.e[0][0] +
        + e[1][0] * adj.e[1][0] +
        + e[2][0] * adj.e[2][0] +
        + e[3][0] * adj.e[3][0];
    }
    inline T Det         ( ) const { return Determinant(); }
    inline T Det3 (const T &a, const T &b, const T &c,
                   const T &d, const T &E, const T &f,
                   const T &g, const T &h, const T &i) const
    {
      return a*(E*i-f*h) - b*(d*i-f*g) + c*(d*h-E*g);
    }
    inline Matrix4x4<T> Adjoint ( ) const
    {
      // rename for easier reference
      const T a=e[0][0], b=e[1][0], c=e[2][0], d=e[3][0];
      const T E=e[0][1], f=e[1][1], g=e[2][1], h=e[3][1];
      const T i=e[0][2], j=e[1][2], k=e[2][2], l=e[3][2];
      const T m=e[0][3], n=e[1][3], o=e[2][3], p=e[3][3];
      return Matrix4x4<T> (+Det3(f,g,h,j,k,l,n,o,p),
                           -Det3(E,g,h,i,k,l,m,o,p),
                           +Det3(E,f,h,i,j,l,m,n,p),
                           -Det3(E,f,g,i,j,k,m,n,o),
                           -Det3(b,c,d,j,k,l,n,o,p),
                           +Det3(a,c,d,i,k,l,m,o,p),
                           -Det3(a,b,d,i,j,l,m,n,p),
                           +Det3(a,b,c,i,j,k,m,n,o),
                           +Det3(b,c,d,f,g,h,n,o,p),
                           -Det3(a,c,d,E,g,h,m,o,p),
                           +Det3(a,b,d,E,f,h,m,n,p),
                           -Det3(a,b,c,E,f,g,m,n,o),
                           -Det3(b,c,d,f,g,h,j,k,l),
                           +Det3(a,c,d,E,g,h,i,k,l),
                           -Det3(a,b,d,E,f,h,i,j,l),
                           +Det3(a,b,c,E,f,g,i,j,k));
    }
    inline Matrix4x4<T>   Inverse     ( ) const
    {
      return Adjoint().Transpose() / Determinant();
    }
    inline Matrix4x4<T>   Inv         ( ) { return Inverse(); }

    inline Matrix4x4<T> & AddTranslateX(const T t) { e[3][0] += t; return *this; }
    inline Matrix4x4<T> & AddTranslateY(const T t) { e[3][1] += t; return *this; }
    inline Matrix4x4<T> & AddTranslateZ(const T t) { e[3][2] += t; return *this; }
    inline Matrix4x4<T> & AddScaleX(const T t)
    {
      e[0][0] *= t;
      e[1][0] *= t;
      e[2][0] *= t;
      e[3][0] *= t;
      return *this;
    }
    inline Matrix4x4<T> & AddScaleY(const T t)
    {
      e[0][1] *= t;
      e[1][1] *= t;
      e[2][1] *= t;
      e[3][1] *= t;
      return *this;
    }
    inline Matrix4x4<T> & AddScaleZ(const T t)
    {
      e[0][2] *= t;
      e[1][2] *= t;
      e[2][2] *= t;
      e[3][2] *= t;
      return *this;
    }
    inline Matrix4x4<T> & Translate(const T tx, const T ty, const T tz)
    {
      AddTranslateX(tx);
      AddTranslateY(ty);
      AddTranslateZ(tz);
      return *this;
    }
    inline Matrix4x4<T> & Scale(const T sx, const T sy, const T sz)
    {
      AddScaleX(sx);
      AddScaleY(sy);
      AddScaleZ(sz);
      return *this;
    }
}
; // class Matrix4x4

typedef Matrix4x4<char>   M4c;
typedef Matrix4x4<short>  M4s;
typedef Matrix4x4<int>    M4i;
typedef Matrix4x4<long>   M4l;
typedef Matrix4x4<float>  M4f;
typedef Matrix4x4<double> M4d;

#endif	// _Matrix4x4_h_
