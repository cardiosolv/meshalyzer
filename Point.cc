#include "DrawingObjects.h"
#include "VecData.h"
#include "gzFileBuffer.h"
#ifdef USE_HDF5
#include <ch5/ch5.h>
#endif

/** draw a nice 3D sphere 
 *
 * \param ctr    position
 * \param radius radius
 */
void draw_sphere( const GLfloat *ctr, float radius )
{
  GLUquadric* quado = gluNewQuadric();
  gluQuadricOrientation(quado, GLU_INSIDE);
  gluQuadricDrawStyle( quado, GLU_FILL );
  gluQuadricNormals( quado, GLU_SMOOTH );
  gluQuadricTexture( quado, GL_FALSE );

  glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
  glPushMatrix();
  glTranslatef( ctr[0], ctr[1], ctr[2] );
  gluSphere( quado, radius, 16, 16 );
  glPopMatrix();
  gluDeleteQuadric(quado);
}


/** draw many PPoints
 *
 *  \param p0       first index of point to draw
 *  \param p1       last index of point to draw
 *  \param colour   colour to use if no data
 *  \param data     data associated with nodes (NULL for no data display)
 *  \param stride   draw every n'th point
 */
void PPoint::draw( int p0, int p1, GLfloat *colour, Colourscale* cs,
                  DATA_TYPE* data, int stride, dataOpac* dataopac )
{
  if ( p0>=_n || p1>=_n ) return;

  glPushAttrib(GL_POLYGON_BIT);

  if( !_3D ) {
    glPointSize( _size );
    glBegin( GL_POINTS );
  }

  if ( data != NULL ) {					// data determines colour
    if ( dataopac!=NULL && dataopac->on() ) {       // data opacity
      translucency(true);
      for ( int i=p0; i<=p1; i+=stride ) {
        if ( !(*_visible)[i] ) continue;
        cs->colourize( data[i], dataopac->alpha(data[i]) );
        if( _3D )
          draw_sphere( _pts+3*i, _size );
        else
          glVertex3fv( _pts+3*i );
      }
      translucency(false);
    } else {                             // no data opacity
      for (int i=p0; i<=p1; i+=stride ) {
        if ( !(*_visible)[i] ) continue;
        cs->colourize( data[i], colour[3] );
        if( _3D )
          draw_sphere( _pts+3*i, _size );
        else
          glVertex3fv( _pts+3*i );
      }
    }
  } else {								 // all same colour
    glColor4fv( colour );
    for (int i=p0; i<=p1; i+=stride ){
      if ( (*_visible)[i] )
        if( _3D )
          draw_sphere( _pts+3*i, _size );
        else
          glVertex3fv( _pts+3*i );
    }
  }

  glEnd();
  glPopAttrib();
}


/** draw a single vertex
 *
 *  \param p      node to draw
 *  \param colour colour to use
 *  \param size   size of point
 */
void PPoint :: draw( int p, GLfloat *colour, float size )
{
  if ( p<_n ) {
    glColor3fv( colour );
    if( _3D ) {
      draw_sphere( _pts+3*p, size );
    } else {
      glPointSize(size);
      glBegin( GL_POINTS );
      glVertex3fv( _pts+3*p );
      glEnd();
    }
  }
}


/** register a point so it can be picked
 *
 * \param p       point to register
 * \param ptDrawn list of nodes registered or not
 */
void PPoint::register_vertex( int p, vector<bool>& ptDrawn )
{
  if ( !ptDrawn[p] ) { // only register once
    ptDrawn[p] = true;
    glLoadName( p );
    glBegin(GL_POINTS);
    glVertex3fv( _pts+3*p );
    glEnd();
  }
}


/** read in the point file */
bool PPoint :: read( const char *fname )
{
  gzFile in = openFile( fname, "pts" );

  const int bufsize=1024;
  char      buff[bufsize];
  if ( gzgets(in, buff, bufsize) == Z_NULL ) throw 1;
  if ( sscanf( buff, "%d", &_n ) != 1 ) throw 2;

  gzFileBuffer file(in);

  if ( _base1 ) _n++;					// add initial bogus point
  _pts = (GLfloat *)malloc(_n*3*sizeof(GLfloat));

  GLfloat min[3], max[3];

  for ( int i=0; i<3*_n; i+=3 ) {

    if ( i==3 && _base1 ) {				// copy the first point
      for ( int j=0; j<3; j++ )
        _pts[i+j] = _pts[i+j-3];
      continue;
    }
    if ( file.gets(buff, bufsize) == Z_NULL ) throw 2;
    if ( sscanf( buff, "%f %f %f", _pts+i, _pts+i+1, _pts+i+2 ) < 3 )
      throw 3;
    for ( int ti=0; ti<3; ti++ ) {
      if ( !i || _pts[i+ti]>max[ti] ) max[ti] = _pts[i+ti];
      if ( !i || _pts[i+ti]<min[ti] ) min[ti] = _pts[i+ti];
    }
  }

  // centre the model about the origin and find maximum distance from origin
  for ( int ti=0; ti<3; ti++ ) _offset[ti] = (min[ti]+max[ti])/2;

  gzclose(in);

  _allvis.resize( _n );
  _allvis.assign( _n, true );

  return true;
}

#ifdef USE_HDF5
bool PPoint :: read(hid_t hdf_file)
{
  ch5_dataset info;
  if (ch5m_pnts_get_info(hdf_file, &info)) return false;
  
  _pts = (GLfloat*) malloc(info.count * info.width * sizeof(GLfloat));
  _n   = info.count;
  if (ch5m_pnts_get_all(hdf_file, _pts)) {
    free(_pts);
    return false;
  }
  
  // Centre model on origin
  float min[3], max[3];
  for (int i = 0; i < _n * 3; i += 3) {
    for (int j = 0; j < 3; j++) {
      if (i == 0 || _pts[i+j] > max[j]) max[j] = _pts[i+j];
      if (i == 0 || _pts[i+j] < min[j]) min[j] = _pts[i+j];
    }
  }
  for (int i = 0; i < 3; i++) _offset[i] = (min[i]+max[i])/2;
  
  _allvis.resize(info.count);
  _allvis.assign(info.count, true );
  
  return true;
}
#endif


/** add some points
 *
 *  \param n number to add
 *  \param p points stored in array as x1 y1 z1 x2 y2 z2 ... xn yn zn
 */
void
PPoint :: add( GLfloat *p, int n )
{
  _n += n;
  _pts = (GLfloat *)realloc( _pts, _n*3*sizeof(GLfloat) );
  memcpy( _pts+3*(_n-n), p, n*3*sizeof(GLfloat) );
  _allvis.resize( _n );
  _allvis.assign( _n, true );
  setVis( true );
}


/** make the points dynamic 
 *
 *  \param fn  the name of the dynamic points file
 *  \param ntm the number of times currently loaded
 *
 *  \retval 0 success
 *  \retval 1 invalid file
 *  \retval 2 incompatible number of points
 */
int
PPoint :: dynamic( const char *fn, int ntm )
{
  ThreadedData<float>* newDynPt;

  try{ 
    newDynPt = new ThreadedData<float>( fn, _n*3, false );
  }
  catch(...) {
    return 1;
  }
  if( ntm==1 || ntm==0 || ( newDynPt->max_tm()+1==ntm ) ) {
    delete _dynPt;
    _dynPt = newDynPt; 
    return 0;
  } else
    return 2;
}


/** set points to the current time 
 *
 *  \param a the time
 *
 *  \note if the requested time is greater than the maximum and the 
 *        maximum is greater than 0, it is an error
 */
void PPoint :: time( int a )
{
  if( _dynPt==NULL || a==_tm || a>_dynPt->max_tm() ) return;

  if( _pts = _dynPt->slice( a ) ) 
    _tm = a;
}
