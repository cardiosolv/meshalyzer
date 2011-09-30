// Trackball.C
// (c) Ingmar Bitter '99
// $Id: Trackball.C,v 1.4 2004/03/10 08:43:09 vigmond Exp $

#include "Trackball.h"
#include <iostream>
#include <fstream>

void Trackball::Demo(int, char**) {
  Mouse mouse;
  Trackball trackball(mouse);
  cout << endl <<"Demo of class Trackball";
  cout << endl <<"size : " << sizeof(Trackball) << " Bytes";
  cout << endl <<"public member functions:";
  cout << endl << "  trackball " << trackball;
  cout << endl << "End of demo of class Trackball " << endl << endl;
} // Demo

// apply transformations according to current state
// call this right before rendering the scene
// (enclose both by gl push & pos matrix)
void Trackball::DoTransform() {
  M4f m4fRot;

  // now at current transformation matrix M
  // change state to M' = M * Mtranslate * Mscale * Mrotate  * MmoveOrigin
  // points will be grown as p' = M' * p;
    
  // Translate
  glTranslatef(v3f_trans.X(), v3f_trans.Y(), v3f_trans.Z());
  // Scale
  glScalef(scale,scale,scale);
  // Rotate
  qRot.SetOglRotationMatrix(m4fRot);
  glMultMatrixf(m4fRot.m);
  // move origin
  glTranslatef(p3f_origin.X(), p3f_origin.Y(), p3f_origin.Z());
  
  transformed = true;
} // DoTransform

// update the trackball state using the current mouse state
void Trackball::Update() {
  V3f *v;

  // SPIN+DRAG (first two buttons) translates the origin of rotation
  if (mouse.buttonsPressed == SPIN+DRAG) {
    v = &p3f_origin; mouse.buttonsPressed = DRAG; }
  else 
    v = &v3f_trans;

  // If the user drags the mouse, Spin or [O]Trans are updated.  If no
  // mouse buttons are pressed, just keep on applying the previous spin
  // rotation, over and over.

  switch (mouse.buttonsPressed) {
  case SPIN:
    Spin();  // updates Spin, which we then apply.
    qRot = qSpin * qRot;
    isChanged = true;
    break;
  case DRAG:
    //qRot = qSpin * qRot; 
    *v += V3f(mouse.dx, mouse.dy, 0.);  
    isChanged = true;
    break;
  case ZOOM: 
    //qRot = qSpin * qRot; 
    scale *= 1+.001*(mouse.du+mouse.dv);  
    isChanged = true;
    break;
  case SPIN+ZOOM:
    v3f_trans (0,0,0); qSpin(1,0,0,0);
    p3f_origin(0,0,0); qRot (1,0,0,0);
    break;
  case DRAG+ZOOM:
    scale=1;
    break;
  case SPIN+DRAG+ZOOM:
    v3f_trans (0,0,0); qSpin(1,0,0,0);
    p3f_origin(0,0,0); qRot (1,0,0,0);
    scale=1;
    break;
  default:
    //qRot = qSpin * qRot;
	break;
  }
  transformed = !isChanged;
} // Update
  
// Implementation of a virtual trackball.  Original code by Gavin Bell, lots
// of ideas from Thant Tessman and the August '88 issue of SigGraph's "Computer
// Graphics," pp. 121-129.
// cleaned up and "objectified" by Ingmar Bitter

// Ok, simulate trackball spinning.  Project the mouse positions onto the virtual
// trackball, then figure out the axis of rotation, which is the cross product
// of O P1 and O P2 (O is the center of the ball).  Note: This is a deformed
// trackball -- it's a trackball in the center, but is deformed into a
// hyperbolic solid of rotation away from the center.
void Trackball::Spin() {
  // computes incremental spin quaternion associated with last mouse movement
  if (mouse.du == 0 && mouse.dv == 0) {
    qSpin(1,0,0,0); // mouse didn't move => no rotation
    isSpinning = false;
    return;
  }
  isSpinning = true;

  float theta, t;
  V3f p1, p2, axis, d;

  // Scale the old and new mouse positions to (-1, 1).
  //mouse.TransformScreenToOgl();

  // First, figure out z-coordinates for projection of P1 and P2 to
  // the deformed sphere.
  // project 2D points onto deformed sphere top get 3D points
  p1(mouse.xOld, mouse.yOld, DepthOnSphere(mouse.xOld, mouse.yOld));
  p2(mouse.x,    mouse.y,    DepthOnSphere(mouse.x,    mouse.y));

  // Now the axis of rotation.
  axis = p1.Cross(p2);

  // Figure out how much to rotate around that axis.  
  /*
  d = p1 - p2;
  t = d.Length();
  if (t > 1.0) t = 1.0;
  if (t < -1.0) t = -1.0;
  theta = asin(t);
  */
  // new method to compute spin angle using dot product which doesn't
  // assume a unit radius sphere
  theta = acos( p1.Dot(p2)/p1.Norm()/p2.Norm() );

  // Return a rotation quaternion
  qSpin.SetRotationAboutAxis(theta, axis);
} // Spin

// Project an x, y pair onto a sphere of radius r OR a hyperbolic sheet
// if we are away from the center of the sphere.
float Trackball::DepthOnSphere(float x, float y) {
  float d, t, z, r=size;

  d = sqrt(x*x + y*y);
  if (d < r * M_SQRT1_2) {
    // inside sphere
    z = sqrt(r*r - d*d);
  } else {
    // on hyperbola
    t = r / M_SQRT2;
    z = t*t / d;
  }
  return z;
} // DepthOnSphere

// save the transform into a file
void Trackball::save( const char *fn )
{
  string xfn( fn );
  if( xfn.rfind( ".xfrm" ) == string::npos ) xfn += ".xfrm";
  ofstream xfrmfile( xfn.c_str() );
  save( xfrmfile );
  xfrmfile.close();
}

// save the transform into a file
void Trackball::save( ofstream& xfrmfile )
{
  xfrmfile << scale << endl;
  xfrmfile << v3f_trans.X() <<" "<<v3f_trans.Y() <<" " <<v3f_trans.Z()<< endl;
  xfrmfile << p3f_origin.X()<<" "<<p3f_origin.Y()<<" "<<p3f_origin.Z()<< endl;
  xfrmfile << qSpin << endl;
  xfrmfile << qRot << endl;
}

// read the tranform from a file
void Trackball::read( const char *fn )
{
  ifstream xfrmfile( fn );
  if( !xfrmfile ) return;
  read( xfrmfile );
  xfrmfile.close();
}

// read the tranform from a file
void Trackball::read( ifstream& xfrmfile )
{
  xfrmfile >> scale;
  xfrmfile >> v3f_trans;
  xfrmfile >> p3f_origin;
  xfrmfile >> qSpin.w >> qSpin.x >> qSpin.y >> qSpin.z;
  xfrmfile >> qRot.w >> qRot.x >> qRot.y >> qRot.z;
}

// end of Trackball.C
