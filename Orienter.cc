#include "Orienter.h"

void draw_arrow( GLUquadricObj* quado, GLfloat stick, GLfloat head,
                                         GLfloat stick_rad, GLfloat head_rad );

Orienter::Orienter(int x, int y, int w, int h, const char *l )
    : Fl_Gl_Tb_Window(x, y, w, h, l),_cp(2),_dl(0)
{
  cp(_cp);
  redraw();
}


/** set up the graphics since it is not guaranteed that a valid OpenGL
 *  context will be available in the constructor
 */
void 
Orienter:: init_graphics()
{
  gluobj = gluNewQuadric();
  gluQuadricDrawStyle( gluobj, GLU_FILL );
  gluQuadricOrientation(gluobj, GLU_INSIDE);
  gluQuadricNormals( gluobj, GLU_SMOOTH );
  _dl = glGenLists(1);
  glNewList(_dl, GL_COMPILE);
    glPushAttrib(GL_ALL_ATTRIB_BITS);
#ifdef DRAW_SQUARE_PLANE
    //draw clipping plane
    glColor4f( 0, 1, 0, 0.2 );
    glBegin(GL_QUADS);
    glVertex3f(-1, -1, 0 );
    glVertex3f(-1,  1, 0 );
    glVertex3f( 1,  1, 0 );
    glVertex3f( 1, -1, 0 );
    glEnd();
#else // draw sphere 
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glColor4f( 0, 0, 0, 1 );
    glBegin(GL_POLYGON);
    for( float i=0; i<360; i+=5 ) 
      glVertex3f( 0, cos(i*M_PI/180.), sin(i/180*M_PI) );
    glEnd();
    glBegin(GL_POLYGON);
    for( float i=0; i<360; i+=5 ) 
      glVertex3f( cos(i/180*M_PI), 0, sin(i/180*M_PI) );
    glEnd();
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glBegin(GL_POLYGON);
    for( float i=0; i<360; i+=5 ) 
      glVertex3f( cos(i/180*M_PI), sin(i/180*M_PI), 0 );
    glEnd();
    glColor4f( 0, 1, 0, 0.2 );
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glBegin(GL_POLYGON);
    for( float i=0; i<360; i+=5 ) 
      glVertex3f( cos(i/180*M_PI), sin(i/180*M_PI), 0 );
    glEnd();
#endif
    //draw arrow
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT1);
    GLfloat ambient0[] = { 0., 0., 0., 1. };
    GLfloat diffuse0[] = { 0.1, 0.1, 0.1, 1. };
    GLfloat specular0[] = { 1., 0., 0., 1. };
    glMaterialfv(GL_FRONT, GL_SPECULAR, specular0);
    glMaterialfv(GL_FRONT, GL_SHININESS, specular0);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse0);
    glLightfv(GL_LIGHT1, GL_AMBIENT, ambient0);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse0);
    glLightfv(GL_LIGHT1, GL_SPECULAR, specular0);
    glTranslatef( 0, 0, -1 );
    glColor4f( 1, 0, 1, 1 );
    draw_arrow( gluobj, 1.6, 0.4, 0.1, 0.2 );
    glPopAttrib();
    glEndList();
}


void
Orienter:: draw()
{ 
  if( !valid() ) {
    valid( 1 );
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glLineWidth( 1. );
    glPointSize( 10.0 );
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    glEnable( GL_DEPTH_TEST );
    glEnable(GL_NORMALIZE);
    glEnable( GL_POINT_SMOOTH );
    glEnable( GL_LINE_SMOOTH );
    glEnable( GL_BLEND );
    glShadeModel(GL_SMOOTH);
    glEnable( GL_MULTISAMPLE );
    glDepthFunc( GL_LEQUAL );
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glViewport(0,0,w(),h());
    glOrtho( -1.05, 1.05, -1.1, 1.1, -10, 10 );
    glClearColor( 1, 1, 1, 1 );
  }

  glClear( GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT );

  glPushMatrix();
  
  // rotate clipping plane only if the orienter was moved, and 
  // not if the model was rotated
  if( !trackball.transformed ){ 
    GLdouble *x = static_cast<ClipPlane*>(parent()->user_data())->plane(_cp);
    Quaternion cp_norm(0, x[0], x[1], x[2]);
    cp_norm.Normalize();
    Quaternion new_norm= trackball.qSpin*cp_norm*trackball.qSpin.GetConjugate();
    ((ClipPlane *)(parent()->user_data()))->update_dir( new_norm.x,
                                           new_norm.y, new_norm.z, _cp );
  }
  
  trackball.Rotation(_view); // adjust for model rotation
  trackball.DoTransform();

  if(!_dl) {
    init_graphics();
  } 
  glCallList(_dl);
  glPopMatrix();

  trackball.Rotation(_view.GetConjugate()); // remove model rotation

}



int
Orienter :: handle( int event )
{
  Fl_Gl_Window::handle(event); // without this, no redraw() called, but why?
  return  Fl_Gl_Tb_Window::handle(event);
}


/** work on a new clipping plane
 *
 * \param cp clipping plane
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
