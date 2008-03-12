/** \file Connections are segments joined by 2 points. The node array is
 * twice as large as the number of connections. For connection i, the nodes
 * defining it are located in _node[2*i] and node[2+i=1]
 */
#include "DrawingObjects.h"

#include "VecData.h"

//#define USE_QUADRIC_OBJS

#ifdef USE_QUADRIC_OBJS

void draw_cylinder( const GLfloat *start, const GLfloat* end, int radius )
{  
  GLUquadric* quado = gluNewQuadric();
  gluQuadricDrawStyle( quado, GLU_FILL );
  gluQuadricOrientation(quado, GLU_INSIDE);

  glPushAttrib( GL_POLYGON_BIT );
  glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
  GLfloat rotvect[3], zaxis[]={0,0,1};
  glPushMatrix();
  glTranslatef( start[0], start[1], start[2] );
  GLfloat h[3];
  sub( end, start, h );
  float height = magnitude(h);
  // determine angle between z-axis and data vector
  float angle = -acos( dot( h, zaxis )/height )*180./M_PI;
  // determine rotation axis
  cross( h, zaxis, rotvect );
  glRotatef( angle, rotvect[0], rotvect[1], rotvect[2] );
  gluCylinder( quado, radius, radius, height, 10, 2 );
  glPopMatrix();
  gluDeleteQuadric(quado);
  glPopAttrib( );
}
#endif

/** draw many Points
 *  
 *  \param p0       first connection of region
 *  \param p1       last connection of region
 *  \param colour   colour to use if no data
 *  \param data     data associated with nodes (NULL for no data display)
 *  \param stride   draw every n'th point
 */
void Connection::draw( int p0, int p1, GLfloat *colour, Colourscale* cs,
					 DATA_TYPE* data, int stride, dataOpac* dataopac )
{
  if( p0>=_n || p1>=_n ) return;

#ifndef USE_QUADRIC_OBJS
  glBegin(GL_LINES);
#endif
  for( int i=p0; i<=p1; i+=stride ) {
	if( !_pt->vis(_node[i*2]) || !_pt->vis(_node[i*2+1]) )
	  continue;
	if( data != NULL ) {
#ifdef USE_QUADRIC_OBJS
	  cs->colourize( data[_node[i*2]], colour[3] );
	  draw_cylinder( _pt->pt(_node[i*2]), _pt->pt(_node[i*2+1]), 0.4 );
#else
	  cs->colourize( data[_node[i*2]], colour[3] );
	  glVertex3fv(_pt->pt(_node[i*2]));
	  cs->colourize( data[_node[i*2+1]], colour[3] );
	  glVertex3fv(_pt->pt(_node[i*2+1]));
#endif
	} else {
#ifdef USE_QUADRIC_OBJS
	  draw_cylinder( _pt->pt(_node[i*2]), _pt->pt(_node[i*2+1]), 0.4 );
#else
	  glVertex3fv(_pt->pt(_node[i*2]));
	  glVertex3fv(_pt->pt(_node[i*2+1]));
#endif
	}
  }
#ifndef USE_QUADRIC_OBJS
  glEnd();
#endif
}


/** highlight a connection
 *
 *  \param p      node to highlight
 *  \param colour colour to use
 *  \param size   size of point
 */
void Connection :: draw( int p, GLfloat *colour, float size )
{
  if( p<_n ) {
	glColor3fv( colour );
	glLineWidth(size);
#ifdef USE_QUADRIC_OBJS
	draw_cylinder( _pt->pt(_node[p*2]), _pt->pt(_node[p*2+1]), 0.4 );
#else
	glBegin( GL_LINES );
	glVertex3fv( _pt->pt(_node[p*2]   ) );
	glVertex3fv( _pt->pt(_node[p*2+1] ) );
	glEnd();
#endif
	glLineWidth(1);
  }
}


/** read in the connection file */
bool Connection :: read( const char *fname )
{
  gzFile in;
  
  try {
    in = openFile( fname, "cnnx" );
  }
  catch(...) { return false; }

  const int bufsize=1024;
  char      buff[bufsize];
  if( gzgets(in, buff, bufsize) == Z_NULL ) return false;
  if( sscanf( buff, "%d", &_n ) != 1 ) return false;
  _node = new int[2*_n];
	for( int i=0; i<2*_n; i+=2 ) {
	  gzgets(in, buff, bufsize);
    sscanf( buff, "%d %d", _node+i, _node+i+1 );
  }
  gzclose(in);
}

