#include "Orienter.h"

void draw_arrow( GLUquadricObj* quado, GLfloat stick, GLfloat head,
                                         GLfloat stick_rad, GLfloat head_rad );

Orienter::Orienter(int x, int y, int w, int h, const char *l )
    : Fl_Gl_Tb_Window(x, y, w, h, l),_cp(2)
{
  gluobj = gluNewQuadric();
  gluQuadricDrawStyle( gluobj, GLU_FILL );
  gluQuadricOrientation(gluobj, GLU_INSIDE);
  cp(_cp);
}

void
Orienter:: draw()
{ 
  //valid(1);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  glPushAttrib(GL_ALL_ATTRIB_BITS);
  glEnable( GL_DEPTH_TEST );
  glEnable(GL_NORMALIZE);
  glEnable( GL_POINT_SMOOTH );
  glEnable( GL_LINE_SMOOTH );
  glEnable( GL_BLEND );
  glShadeModel(GL_SMOOTH);
  glDepthFunc( GL_LEQUAL );
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glViewport(0,0,w(),h());
  glOrtho( -1.05, 1.05, -1.1, 1.1, -10, 10 );
  glClearColor( 1, 1, 1, 1 );
  glClear( GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT );

  // rotate clipping plane only if the orienter was moved, and 
  // not if the model was rotated
  if( !trackball.transformed ){ 
    GLdouble *x = static_cast<ClipPlane*>(parent()->user_data())->plane(_cp);
    Quaternion pcoeff(0, x[0], x[1], x[2]);
    pcoeff.Normalize();
    Quaternion newpcoeff= trackball.qSpin*pcoeff*trackball.qSpin.GetConjugate();
    ((ClipPlane *)(parent()->user_data()))->update_dir( newpcoeff.x,
    newpcoeff.y, newpcoeff.z,_cp );
  }
  
  trackball.Rotation(_view); // adjust for model rotation
  trackball.DoTransform();

  // draw cliiping plane
  glColor4f( 0, 1, 0, 0.2 );
  glBegin(GL_QUADS);
  glVertex3f(-1, -1, 0 );
  glVertex3f(-1,  1, 0 );
  glVertex3f( 1,  1, 0 );
  glVertex3f( 1, -1, 0 );
  glEnd();
  //draw the arrow
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT1);
  glColor3f( 0, 0, 1 );
  glTranslatef( 0, 0, -1 );
  draw_arrow( gluobj, 1.6, 0.4, 0.1, 0.2 );
  glPopMatrix();

  trackball.Rotation(_view.GetConjugate()); // remove model rotation

  glPopAttrib();
}



int
Orienter :: handle( int event )
{
  Fl_Gl_Window::handle(event); // without this, no redraw() called, but why?
  return  Fl_Gl_Tb_Window::handle(event);
}


/** work on a new cliiping plane
 *
 * \param cp cliiping plane
 */
void 
Orienter :: cp( int cp )
{
  _cp = cp;
  GLdouble *x = static_cast<ClipPlane*>(parent()->user_data())->plane(cp);
  V3f plane( x[0], x[1], x[2] );
  V3f zaxis(0,0,1);
  float angle = -acos( plane.Dot(zaxis) );
  V3f rv = plane.Cross(zaxis);
  trackball.SetRotation(angle, rv);
  parent()->redraw();
}
