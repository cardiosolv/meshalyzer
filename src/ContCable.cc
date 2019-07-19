/** \file ContCables are cables defined by contiguously numbered points.
 * The cables are defined by two integers, the first is the node number of
 * the first point in the cable and the second is one more than the index of
 * the last point in the cable. The length of the _node array is the number
 * of cables plus 1. Cable i is defined by _node[i] and _node[i+1]
 */
#include "DrawingObjects.h"

void fillCylTexture( GLfloat *, GLfloat *, GLfloat, GLfloat, GLubyte * );
void draw_cylinder( const GLfloat *, const GLfloat *, int );
void initializeCyltexture( GLubyte *, GLuint * );
#define NUM_S 2
#define NUM_T 2

/** draw many ContCables
 *
 *  \param p0       first index of cable to draw
 *  \param p1       last index of cable to draw
 *  \param colour   colour to use if no data
 *  \param data     data associated with nodes (NULL for no data display)
 *  \param stride   draw every n'th point
 */
void ContCable::draw( int p0, int p1, GLfloat *colour, Colourscale* cs,
                      DATA_TYPE* data, int stride, dataOpac* dataopac )
{
  if ( p0>=_n || p1>=_n ) return;

  if ( (dataopac!=NULL && dataopac->on()) || colour[3]<0.95 )    // data opacity
	  translucency(true);

  GLuint texName;
  GLubyte cgrad[4*NUM_S*NUM_T]; 
  bool facetshade;
  if( _3D ) {
    facetshade =  !glIsEnabled(GL_LINE_SMOOTH );
    if( data !=NULL && !facetshade) initializeCyltexture( cgrad, &texName );
  } else {
	glLineWidth( _size );
  }

  if( data==NULL ) glColor4fv( colour );

  GLfloat a0 = colour[3], a1 = colour[3];

  for ( int i=p0; i<=p1; i+=stride ) {

    if( !_3D )
      glBegin(GL_LINE_STRIP);

    if ( data!=NULL ) {

      for ( int j=_node[i]; j<_node[i+1]; j++ ) {

        if ( !_pt->vis(j) ) continue;

        if ( dataopac!=NULL && dataopac->on() ) {       // data opacity
          a0 =  dataopac->alpha(data[j]); 
          a1 =  dataopac->alpha(data[j+1]); 
        }

        if( _3D ) {
          if( j<_node[i+1]-1 ) {
            if( facetshade )
              cs->colourize( data[j], a0 );
            else 
              fillCylTexture(  cs->colorvec( data[j]   ),
                      cs->colorvec( data[j+1] ), a0, a1, cgrad ); 
            draw_cylinder( _pt->pt(j), _pt->pt(j+1), _size );
          }
        } else {
          cs->colourize( data[j], a0 );
          glVertex3fv( _pt->pt(j) );
        }
      }
    } else {							// no data on cables
      for ( int j=_node[i]; j<_node[i+1]; j++ ) {
        if ( !_pt->vis(j) ) continue;
        if( _3D ) {
          if( j<_node[i+1]-1 )
            draw_cylinder( _pt->pt(j), _pt->pt(j+1), _size );
        } else
          glVertex3fv( _pt->pt(j) );
      }
    }

    if( !_3D )
      glEnd();
  }

  if( _3D ) {
    glDisable(GL_TEXTURE_2D );
    glDeleteTextures( 1, &texName );
  } 

  if ( (dataopac!=NULL && dataopac->on()) || colour[3]<0.95 )  // data opacity
    translucency(false);

}


/** highlight a vertex
 *
 *  \param p      node to highlight
 *  \param colour colour to use
 *  \param size   size of point
 */
void ContCable::draw( int p, GLfloat *colour, float size )
{
  if ( p<_n ) {
    glColor3fv( colour );
    glLineWidth(size);
    if( !_3D )
      glBegin( GL_LINE_STRIP );
    for ( int j=_node[p]; j<_node[p+1]; j++ ) {
      if ( !_pt->vis(j) ) continue;
      if( _3D ) {
        if( j<_node[p+1]-1 )
          draw_cylinder( _pt->pt(j), _pt->pt(j+1), size );
      } else
        glVertex3fv( _pt->pt(j) );
    }
    if( !_3D )
      glEnd();
    glLineWidth(1);
  }
}


/** read in the point file */
bool ContCable::read( const char *fname )
{
  gzFile in;

  try {
    in = openFile( fname, "cables" );
  } catch (...) { return false; }

  const int bufsize=1024;
  char      buff[bufsize];
  if ( gzgets(in, buff, bufsize) == Z_NULL ) return false;
  if ( sscanf( buff, "%d", &_n ) != 1 ) return false;
  _node = new int[_n+1];
  for ( int i=0; i<=_n; i++ ) {
    gzgets(in, buff, bufsize);
    sscanf( buff, "%d", _node+i );
  }
  gzclose(in);
  return true;
}

#ifdef USE_HDF5
bool ContCable::read(hid_t hdf_file)
{
  ch5_dataset dset_info;
  if (ch5m_cabl_get_info(hdf_file, &dset_info))
    return false;
  
  _n = dset_info.count;
  _node = new int[dset_info.count];
  if ( ch5m_cabl_get_all(hdf_file, _node) ){
    cerr << "Error reading in cables" << endl;
    _n = 0;
    delete _node;
    return false;
  }
  
  return true;
}
#endif


/** register vertices so that they can be picked
 *
 * \param p0      start index
 * \param p1      stop  index
 * \param ptDrawn list of points drawn
 */
void ContCable::register_vertices( int p0, int p1, vector<bool>& ptDrawn )
{
  for ( int i=p0; i<=p1; i++ ) {
    int p = i;
    for ( int j=_node[p]; j<_node[p+1]; j++ )
      if ( _pt->vis(j) && !ptDrawn[j] ) {
        ptDrawn[j] = true;
        glLoadName( j );
        glBegin(GL_POINTS);
        glVertex3fv( _pt->pt(j) );
        glEnd();
      }
  }
}

