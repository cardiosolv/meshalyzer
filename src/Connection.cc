/** \file Connections are segments joined by 2 points. The node array is
 * twice as large as the number of connections. For connection i, the nodes
 * defining it are located in _node[2*i] and node[2+i=1]
 */
#include "DrawingObjects.h"

#include "VecData.h"
#ifdef USE_HDF5
#include <ch5/ch5.h>
#endif

void draw_cylinder( const GLfloat *start, const GLfloat* end, int radius )
{
  // cylinder
  GLUquadric* quado = gluNewQuadric();
  gluQuadricDrawStyle( quado, GLU_FILL );
  gluQuadricOrientation(quado, GLU_INSIDE);
  GLboolean facetshade = !glIsEnabled(GL_LINE_SMOOTH );
  gluQuadricTexture(quado, facetshade?GL_FALSE:GL_TRUE );
  
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
  gluCylinder( quado, radius, radius, height, 10, 5 );
  glPopMatrix();
  gluDeleteQuadric(quado);
  glPopAttrib( );
}


#define NUM_S 2
#define NUM_T 2
/** fill the texture buffer so that data is linearly interpolated over the cylinder
 *
 * \param co      initial colour
 * \param c1      final colour
 * \param a0      intial alpha
 * \param a1      final alpha
 * \param texture texture to fill 
 */
void fillCylTexture( GLfloat *c0, GLfloat *c1, GLfloat a0, GLfloat a1, 
                                                           GLubyte *texture )
{
  for( int i=0; i<NUM_S; i++ )
	for( int j=0; j<NUM_T; j++ ) { 
	  for( int b=0; b<3; b++ ) 
	    texture[4*(NUM_T*i+j)+b] = (i*c1[b]+(NUM_S-i-1)*c0[b])*255./(NUM_S-1);
	  texture[4*(NUM_T*i+j)+3] = (i*a1+(NUM_S-i-1)*a0)*255./(NUM_S-1);
	  }

   glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, NUM_S, NUM_T, GL_RGBA,
            GL_UNSIGNED_BYTE, texture );
}


/** set up a texture to be used to draw a cylinder
 *
 *  \param cgrad the texture map 
 *  \param tn    the texture reference
 */
void
initializeCyltexture( GLubyte *cgrad, GLuint *tn )
{
  glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
  glGenTextures( 1, tn );
  glBindTexture( GL_TEXTURE_2D, *tn );
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, NUM_S, NUM_T, 0, GL_RGBA,
                                           GL_UNSIGNED_BYTE, cgrad );
  glEnable( GL_TEXTURE_2D );
  glColor4f( 1., 1., 1., 1. );   // will be modulated with texture
}


/** draw many PPoints
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
  void draw_sphere( const GLfloat *, float );

  if ( p0>=_n || p1>=_n ) return;

  if ( (dataopac!=NULL && dataopac->on()) || colour[3]<0.95 )   // data opacity
	  translucency(true);


  GLuint texName;
  GLubyte cgrad[4*NUM_S*NUM_T];
  bool    facetshade;
  if( _3D ) {
    facetshade = !glIsEnabled(GL_LINE_SMOOTH );
    if( data !=NULL && !facetshade) initializeCyltexture( cgrad, &texName );
  } else if( !_3D ){
	glLineWidth( _size );
	glBegin(GL_LINES);
  }

  if( data==NULL ) glColor4fv( colour );


  for ( int i=p0; i<=p1; i+=stride ) {

    int n0 = _node[i*2], n1=_node[i*2+1];

	if ( !_pt->vis( n0 ) )
	  continue;
	
	if ( data != NULL ) {

	  GLfloat a0 = colour[3], a1 = colour[3];
	  if ( dataopac!=NULL && dataopac->on() ) {       // data opacity
		a0 =  dataopac->alpha(data[n0]); 
		a1 =  dataopac->alpha(data[n1]); 
	  }

	  if( _3D ) {
		
        cs->colourize( data[n0], a0 );
        draw_sphere(  _pt->pt(n0), 0.99*_size );
        if( !facetshade) 
		  cs->colourize( data[n1], a1 );
        draw_sphere(  _pt->pt(n1), 0.99*_size );

		if( facetshade )
          cs->colourize( data[n0] );
        else
          fillCylTexture( cs->colorvec( data[n0] ),
			              cs->colorvec( data[n1] ), a0, a1, cgrad ); 
		draw_cylinder( _pt->pt(n0), _pt->pt(n1), _size );

	  }else{
		cs->colourize( data[n0], a0 );
		glVertex3fv(_pt->pt(n0));
		cs->colourize( data[n1], a1 );
		glVertex3fv(_pt->pt(n1));
	  }
	} else {
	  if( _3D ){
        draw_sphere(  _pt->pt(n0), _size );
        draw_sphere(  _pt->pt(n1), _size );
		draw_cylinder( _pt->pt(n0), _pt->pt(n1), _size );
	  }else{
		glVertex3fv(_pt->pt(n0));
		glVertex3fv(_pt->pt(n1));
	  }
	}
  }

  if ( dataopac!=NULL && dataopac->on() )        // data opacity
	  translucency(false);

  if( _3D ) {
	glDisable(GL_TEXTURE_1D );
	glDeleteTextures( 1, &texName );
  } else
    glEnd();

  if ( (dataopac!=NULL && dataopac->on()) || colour[3]<0.95 )     // data opacity
	  translucency(false);
}


/** add a list of connections
 *  \param n  number of connections
 *  \param nl node list [c0_0, c0_1, c1_0, c1_1, c2_0, etc]
 */
void Connection:: add( int n, int *nl )
{
  int *new_nl = new int[(_n+n)*_ptsPerObj];
  memcpy( new_nl, _node, _n*sizeof(int)*_ptsPerObj );
  delete _node;
  memcpy( new_nl+_n*sizeof(int)*_ptsPerObj, nl, n*sizeof(int)*_ptsPerObj );
  _node = new_nl;
  _n += n;
}


/** highlight a connection
 *
 *  \param p      node to highlight
 *  \param colour colour to use
 *  \param size   size of point
 */
void Connection::draw( int p, GLfloat *colour, float size )
{
  if ( p<_n ) {
    glColor3fv( colour );
    glLineWidth(size);
    if( _3D )
      draw_cylinder( _pt->pt(_node[p*2]), _pt->pt(_node[p*2+1]), size );
    else{
      glBegin( GL_LINES );
      glVertex3fv( _pt->pt(_node[p*2]   ) );
      glVertex3fv( _pt->pt(_node[p*2+1] ) );
      glEnd();
    }
    glLineWidth(1);
  }
}


/** read in the connection file */
bool Connection::read( const char *fname )
{
  gzFile in;

  try {
    in = openFile( fname, "cnnx" );
  } catch (...) { return false; }

  const int bufsize=1024;
  char      buff[bufsize];
  if ( gzgets(in, buff, bufsize) == Z_NULL ) return false;
  if ( sscanf( buff, "%d", &_n ) != 1 ) return false;
  _node = new int[2*_n];
  for ( int i=0; i<2*_n; i+=2 ) {
    gzgets(in, buff, bufsize);
    sscanf( buff, "%d %d", _node+i, _node+i+1 );
  }
  gzclose(in);
}

#ifdef USE_HDF5
bool Connection::read(hid_t hdf_file)
{
  ch5_dataset dset_info;
  if( ch5m_conn_get_info(hdf_file, &dset_info) )
    return false;
  
  _n = dset_info.count;
  _node = new int[_n * dset_info.width];
  if ( ch5m_conn_get_all(hdf_file, _node) ){
    cerr << "Error reading in connections" << endl;
    _n = 0;
    delete _node;
    return false;
  }
  
  return true;
}
#endif
