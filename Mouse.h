// Mouse.h
// (c) Ingmar Bitter '99
// $Id: Mouse.h,v 1.1 2004/02/06 21:50:57 vigmond Exp $

// Mouse: class to track current mouse position and movement

#ifndef _Mouse_h_	// prevent multiple includes
#define _Mouse_h_

#include "Global.h"
#include "sstream"

class Mouse {
public:
  int   u,v, uOld,vOld, du,dv; float uw,vh;  // screen pixel coordinates, origin at (0,0)
  float x,y, xOld,yOld, dx,dy, x0,y1,xw,yh;  // ogl coordinates
  int buttonsPressed;
public:
  static void	Demo (int argc, char *argv[]);

  inline Mouse () { Reset(); }

  inline Mouse & Reset() {
    u=v = uOld=vOld = du=dv = 0;           uw=vh=1;
    x=y = xOld=yOld = dx=dy = 0;  x0=y1=0; xw=yh=1;
    return *this; }
  inline void SetWindowSize(int uwnew, int vhnew) { uw = uwnew; vh = vhnew; }
  inline void SetOglPosAndSize(float x0left, float y1top, float xwnew, float yhnew) { 
  x0 = x0left; y1 = y1top;  xw = xwnew; yh = yhnew; }
  inline void TransformScreenToOgl() {
    x = x0 + u*(xw/uw); xOld = x0 + uOld*(xw/uw);
    y = y1 - v*(yh/vh); yOld = y1 - vOld*(yh/vh); }
 inline void UpdatePos(int unew, int vnew) { 
    uOld = u;  u=unew;  du = u-uOld;  xOld = x;  x = x0 + u * xw/uw;  dx = x-xOld;
    vOld = v;  v=vnew;  dv = v-vOld;  yOld = y;  y = y1 - v * yh/vh;  dy = y-yOld;
  }
  inline void UpdateButtons(int buttonsDown) { buttonsPressed = buttonsDown; }
  friend ostream & operator << (ostream & os, Mouse & mouse) { return mouse.Ostream(os); }
  inline ostream & Ostream (ostream & os) {
    os << "mouse: u="<<u<<" v="<<v<<" uw="<<uw<<" vh="<<vh
       <<    " || x="<<x<<" y="<<y<<" xOld="<<xOld<<" yOld="<<yOld<<" xw="<<xw<<" yh="<<yh;
    return os;
  }
  const char * Str() { stringstream stream; stream << *this; return stream.str().c_str(); }
}; // Mouse

#endif	// _Mouse_h_

