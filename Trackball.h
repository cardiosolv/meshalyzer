// Trackball.h
// (c) Ingmar Bitter '99
// $Id: Trackball.h,v 1.6 2006/01/19 12:50:31 vigmond Exp $

// Trackball: class to track current trackball position and movement

#ifndef _Trackball_h_	// prevent multiple includes
#define _Trackball_h_

#include "Global.h"
#include "Mouse.h"
#include "Quaternion.h"
#include "Vector3D.h"
#include <FL/Fl.H> // FL_BUTTON*
#include <math.h>
#include <FL/gl.h>
#include <string>

#define SPIN (FL_BUTTON1)
#define DRAG (FL_BUTTON2)
#define ZOOM (FL_BUTTON3)
#define BUTTON_MASK (FL_BUTTON1 | FL_BUTTON2 | FL_BUTTON3)

class Trackball
{
  public:
    Mouse & mouse;  // screen pixel coordinates, origin at (0,0)
    float size;
    float scale;            // uniform object scale factor
    V3f v3f_trans;          // translation
    V3f p3f_origin;         // point to rotate about
    Quaternion qSpin;       // quaternion incremental rotation
    Quaternion qRot;        // quaternion total rotation
    bool isSpinning;         // if the user did a SPIN
    bool isChanged;          // if the view changed
    bool transformed;        // if the transform has been applied
  public:
    static void	Demo (int argc, char *argv[]);
    inline Trackball (Mouse & m) : mouse(m) { Reset(); }

    inline Trackball & Reset()
    {
      mouse.Reset();
      mouse.SetOglPosAndSize(-1,1,2,2);
      size = 1; scale=1; v3f_trans(0,0,0); p3f_origin(0,0,0); qSpin(1,0,0,0); qRot(1,0,0,0);
      isSpinning = false;
      isChanged = true;
      transformed = true;
      return *this;
    }
    friend ostream & operator << (ostream & os, Trackball & trackball) { return trackball.Ostream(os); }
    inline ostream & Ostream (ostream & os)
    {
      os << "trackball: size="<<size<<" "<<mouse;
      return os;
    }
    const char * Str() { stringstream stream; stream << *this; return stream.str().c_str(); }

    // size of the virtual trackball as fraction of window
    // usefull range: .5 to 1.5, 1.0 default
    inline void  SetSize(float newSize) { size = newSize; assert (size>0); }
    inline float GetSize( )             { return size; }
    // uniform scene scale factor
    inline void  SetScale(float s) { scale = s;    } // uniform
    inline float GetScale()        { return scale; } // scale factor
    // trackball/scene center of rotation
    inline void  SetOrigin(float x, float y, float z) { p3f_origin(x, y, z); }
    inline const V3f & GetOrigin( )                        { return p3f_origin;  }
    // trackball/scene rotation (angle assumed in [-pi, +pi])
    inline void  SetRotation(float angle, V3f & axis) { qRot.SetRotationAboutAxis(angle, axis);  }
    inline void  Rotation(float angle, V3f & axis) {  qSpin.SetRotationAboutAxis(angle, axis);qRot = qSpin*qRot; }
    inline void  Rotation(Quaternion spin) { qRot = spin*qRot; }
    inline const Quaternion & GetRotation( ) { return qRot; }
    // trackball/scene translation
    inline void  SetTranslation(float x, float y, float z) { v3f_trans(x, y, z); }
    inline const V3f & GetTranslation( )                   { return v3f_trans;   }
    void DoTransform();
    void Update();
    void Spin();
    float DepthOnSphere(float x, float y);
    void  save( const char * );
    void  read( const char * );
    void  save( ofstream& );
    void  read( ifstream& );
}
; // Trackball

#endif	// _Trackball_h_

