// Quaternion.h
// (c) Ingmar Bitter '99
// $Id: Quaternion.h,v 1.2 2004/03/09 22:21:59 vigmond Exp $

#ifndef _Quaternion_h_	// prevent multiple includes
#define _Quaternion_h_

#include "Global.h"
#include <math.h>
#include "Vector3D.h"
#include "Matrix4x4.h"


class Quaternion
{
  public:
    float w,x,y,z;
    Quaternion() : w(1),x(0),y(0),z(0) { }
    Quaternion(float inw, float inx, float iny, float inz) : w(inw),x(inx),y(iny),z(inz) { }
    template<class T> Quaternion( const Vector3D<T> &a) : w(0.),x(a.e[0]),y(a.e[1]),z(a.e[2]) {}
    inline void operator () (float inw, float inx, float iny, float inz) { w=inw; x=inx; y=iny; z=inz; }

    inline float operator [] (int index)
    {
      assert(index < 0 && index < 4);
switch (index) { case 0: return w; case 1: return x; case 2: return y; case 3: return z;}
      ; return 0;
    }
    inline bool operator == ( Quaternion &q ) const
    {
      return w==q.w && x==q.x && y==q.y && z==q.z;
    }
    inline bool operator != ( Quaternion &q ) const
    {
      return w!=q.w || x!=q.x || y!=q.y || z!=q.z;
    }
    inline const float & operator [] (int index) const
    {
      assert(index >= 0 && index < 4);
switch (index) { case 0: return w; case 1: return x; case 2: return y; case 3: return z;}
      ; return w;
    }
    friend ostream& operator<<(ostream& os, Quaternion q )
    {
      os << q.w << " " << q.x << " " << q.y << " " << q.z;
      return os;
    }
    friend istream& operator>>(istream& os, Quaternion &q )
    {
      os >> q.w >> q.x >> q.y >> q.z;
      return os;
    }
    inline float Length() { return sqrt(w*w + x*x + y*y + z*z); }
    inline Quaternion & Normalize() { 
      float l=Length(); w/=l; x/=l; y/=l; z/=l;return *this; 
    }
    inline void SetRotationAboutAxis(float angle, V3f & axis)
    {
      w = cos(angle / 2.); float s=sin(angle/2.);
      axis.Normalize();
      x = s*axis.X();  y = s*axis.Y();  z = s*axis.Z();
    }
    inline void SetConjugate() { x=-x; y=-y; z=-z; }
    inline const Quaternion GetConjugate() const {
      return Quaternion(w,-x,-y,-z); 
    }
    inline const Quaternion operator + (const Quaternion & q) const 
    { 
      return Quaternion(w+q.w,x+q.x,y+q.y,z+q.z);
    }
    inline const Quaternion operator - (const Quaternion & q) const 
    { 
      return Quaternion(w-q.w,x-q.x,y-q.y,z-q.z);
    }
    inline const Quaternion operator * (double a) const 
    { 
      return Quaternion(a*w,a*x,a*y,a*z);
    }
    inline const Quaternion operator * (const Quaternion & q) const 
    { 
      return Muliply(q); 
    }
    inline const Quaternion Mul(const Quaternion & q) const { return Muliply(q);}
    inline const Quaternion Muliply(const Quaternion & q) const
    {
      return  Quaternion(w*q.w - x*q.x - y*q.y - z*q.z,
                         w*q.x + x*q.w + y*q.z - z*q.y,
                         w*q.y - x*q.z + y*q.w + z*q.x,
                         w*q.z + x*q.y - y*q.x + z*q.w);
    }
    inline void operator *= (const Quaternion & q)
    {
      float tw = w*q.w - x*q.x - y*q.y - z*q.z;
      float tx = w*q.x + x*q.w + y*q.z - z*q.y;
      float ty = w*q.y - x*q.z + y*q.w + z*q.x;
      float tz = w*q.z + x*q.y - y*q.x + z*q.w;
      w = tw; x=tx; y=ty; z=tz;
    }

    inline Quaternion Rotate( const Quaternion &v ) { return *this * v * this->GetConjugate(); }
    template<class T> 
    inline Vector3D<T> Rotate( const Vector3D<T> &v ) {Quaternion qv(v); qv = *this * qv * this->GetConjugate(); return V3f(qv.x,qv.y,qv.z); }

    inline const M4f GetRotationMatrix() const
    {
      return M4f(w*w + x*x - y*y - z*z, 2*(x*y+w*z),           2*(x*z-w*y),                0,
                 2*(x*y-w*z),           w*w - x*x + y*y - z*z, 2*(y*z-w*x),                0,
                 2*(x*z-w*y),           2*(y*z-w*x),           w*w - x*x - y*y + z*z,      0,
                 0,                     0,                     0,      w*w + x*x + y*y + z*z);
    }
    inline void SetRotationMatrix(M4f &m) const
    {
      m(w*w + x*x - y*y - z*z, 2*(x*y+w*z),           2*(x*z-w*y),                0,
        2*(x*y-w*z),           w*w - x*x + y*y - z*z, 2*(y*z-w*x),                0,
        2*(x*z-w*y),           2*(y*z-w*x),           w*w - x*x - y*y + z*z,      0,
        0,                     0,                     0,      w*w + x*x + y*y + z*z);
    }
    inline void SetOglRotationMatrix(M4f &m) const
    {
      m(1.-2.*(y*y + z*z), 2.*(x*y-w*z),      2.*(x*z+w*y),    0.,
        2.*(x*y + w*z),    1.-2.*(x*x + z*z), 2.*(y*z-w*x),    0.,
        2.*(x*z - w*y),    2*(y*z+w*x),       1.-2.*(x*x+y*y), 0.,
        0.,                0.,                0.,              1.);
    }
    inline void SetOglRotationMatrix(float m[16]) const
    {
      m[0]=1.-2.*(y*y+z*z); m[4]=   2.*(x*y-w*z); m[ 8]=   2.*(x*z+w*y); m[12]=0.;
      m[1]=   2.*(x*y+w*z); m[5]=1.-2.*(x*x+z*z); m[ 9]=   2.*(y*z-w*x); m[13]=0.;
      m[2]=   2.*(x*z-w*y); m[6]=   2.*(y*z+w*x); m[10]=1.-2.*(x*x+y*y); m[14]=0.;
      m[3]=0.;              m[7]=0.;              m[11]=0.;              m[15]=1.;
    }
    inline Quaternion Slurp( Quaternion &final, float t )
    {
      if( t<0 or t>1 ) return *this;

      Quaternion v0 = *this;
      Quaternion v1 = final;
      v0.Normalize();
      v1.Normalize();
      
      // this is from Wikipedia
      double dot = v0.w*v1.w+v0.x*v1.x+v0.y*v1.y+v0.z*v1.z;
      if (dot < 0.0f) {
        v1 = v1*-1;
        dot = -dot;
      }  
      const double DOT_THRESHOLD = 0.9995;
      if (dot > DOT_THRESHOLD) {
        Quaternion result = v0 + (v1-v0)*t;
        result.Normalize();
        return result*((1-t)*Length() + t*final.Length());
      }
      double theta_0     = acos(dot);  
      double theta       = theta_0*t;    
      double sin_theta   = sin(theta);
      double sin_theta_0 = sin(theta_0);
      double s0 = cos(theta) - dot * sin_theta / sin_theta_0;
      double s1 = sin_theta / sin_theta_0;

      Quaternion on_sph =(v0*s0) + (v1*s1);

      return on_sph*((1-t)*Length() + t*final.Length());
    }
}
; // class Quaternion

template<class T>
Quaternion
shortest_arc( Vector3D<T> a, Vector3D<T> b ) 
{
  Quaternion q;
  T dp = a.Dot(b);
  if( dp >=  0.9999 ) { return Quaternion( 1,0,0,0 ); }
  if( dp <= -0.9999 ) { return Quaternion( 0,1,0,0 ); }
  Vector3D<T> cp = a.Cross(b);
  q.x = cp.X();
  q.y = cp.Y();
  q.z = cp.Z();
  q.w = sqrt((a.SquareLength()) * (b.SquareLength())) + dp;
  q.Normalize();
  return q;
}

#endif // _Quaternion_h_
