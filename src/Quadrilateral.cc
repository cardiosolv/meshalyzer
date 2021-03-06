#include "DrawingObjects.h"

const int quad_iso_table[][11] = {
    { 0 },
    { 1, 2, 0, 1, 0, 3 },
    { 1, 2, 0, 1, 1, 2 },
    { 1, 2, 0, 3, 1, 2 },
    { 1, 2, 1, 2, 2, 3 },
    { 2, 2, 0, 1, 2, 3, 2, 1, 2, 0, 3 },
    { 1, 2, 0, 1, 2, 3 },
    { 1, 2, 0, 3, 2, 3 },
    { 1, 2, 0, 3, 2, 3 },
    { 1, 2, 0, 1, 2, 3 },
    { 2, 2, 0, 1, 2, 3, 2, 1, 2, 0, 3 },
    { 1, 2, 1, 2, 2, 3 },
    { 1, 2, 0, 3, 1, 2 },
    { 1, 2, 0, 1, 1, 2 },
    { 1, 2, 0, 1, 0, 3 },
    { 0 }
};

/** draw many Quadrilaterals
 *
 *  \param p0       first index of point to draw
 *  \param p1       last index of point to draw
 *  \param colour   colour to use if no data
 *  \param cs       colour scale
 *  \param data     data associated with nodes (NULL for no data display)
 *  \param stride   draw every n'th point
 *  \param dataopac data opacity
 */
void Quadrilateral::draw( int p0, int p1, GLfloat *colour, Colourscale* cs,
                          DATA_TYPE* data, int stride, dataOpac* dataopac )
{
  draw( p0, p1, colour, cs, data, stride, dataopac, _ptnrml );
}


/** draw many Quadrilaterals
 *
 *  \param p0       index of first point to draw
 *  \param p1       index of last point to draw
 *  \param colour   colour to use if no data
 *  \param cs       colour scale
 *  \param data     data associated with nodes (NULL for no data display)
 *  \param stride   draw every n'th point
 *  \param dataopac data opacity
 *  \param ptnrml   vertex normals (NULL for none)
 */
void Quadrilateral::draw( int p0, int p1, GLfloat *colour, Colourscale* cs,
                          DATA_TYPE* data, int stride, dataOpac* dataopac,
                          const GLfloat* ptnrml )
{
  if ( p0>=_n || p1>=_n ) return;

  GLboolean lightson;
  glGetBooleanv( GL_LIGHTING, &lightson );

  glBegin(GL_QUADS);

  for ( int i=4*p0; i<=4*p1; i+=4*stride ) {
    if ( !_pt->vis(_node[i]) || !_pt->vis(_node[i+1]) || !_pt->vis(_node[i+2])
         || !_pt->vis(_node[i+3])                        )
      continue;

    if ( lightson && ptnrml==NULL) glNormal3fv( nrml(i/3) );

    if ( data!=NULL ) {
      if ( dataopac->on() ) {		// data opacity
        for ( int j=0; j<4; j++ ) {
          cs->colourize( data[_node[i+j]], dataopac->alpha(data[_node[i+j]]));
          if ( lightson && ptnrml ) glNormal3fv( ptnrml+_node[i+j]*3 );
          glVertex3fv( _pt->pt(_node[i+j]) );
        }
      } else {		// no data opacity
        for ( int j=0; j<4; j++ ) {
          cs->colourize( data[_node[i+j]], colour[3] );
          if ( lightson && ptnrml ) glNormal3fv( ptnrml+_node[i+j]*3 );
          glVertex3fv( _pt->pt(_node[i+j]) );
        }
      }
    } else {							// no data
      glColor4fv( colour );
      for ( int j=0; j<4; j++ ) {
        if ( lightson && ptnrml ) glNormal3fv( ptnrml+_node[i+j]*3 );
        glVertex3fv( _pt->pt(_node[i+j]) );
      }
    }
  }
  glEnd();
}


/** draw one Quadrilaterals and do not do glBegin/glEnd
 *
 *  \param p0       index of first point to draw
 *  \param colour   colour to use if no data
 *  \param cs       colour scale
 *  \param data     data associated with nodes (NULL for no data display)
 *  \param dataopac data opacity
 *  \param ptnrml   vertex normals (NULL for none)
 *  \param lightson true if lights are on
 */
void Quadrilateral::draw( int p0, GLfloat *colour, Colourscale* cs,
                          DATA_TYPE* data, dataOpac* dataopac,
                          const GLfloat* ptnrml, bool lightson )
{
  if ( p0>=_n ) return;

  int i = 4*p0;
  if ( !_pt->vis(_node[i]) || !_pt->vis(_node[i+1]) || !_pt->vis(_node[i+2])
          || !_pt->vis(_node[i+3])                        )
    return;

  if ( lightson && ptnrml==NULL) glNormal3fv( nrml(i/3) );

  if ( data!=NULL ) {
    if ( dataopac->on() ) {		// data opacity
      for ( int j=0; j<4; j++ ) {
        cs->colourize( data[_node[i+j]], dataopac->alpha(data[_node[i+j]]));
        if ( lightson && ptnrml ) glNormal3fv( ptnrml+_node[i+j]*3 );
        glVertex3fv( _pt->pt(_node[i+j]) );
      }
    } else {		// no data opacity
      for ( int j=0; j<4; j++ ) {
        cs->colourize( data[_node[i+j]], colour[3] );
        if ( lightson && ptnrml ) glNormal3fv( ptnrml+_node[i+j]*3 );
        glVertex3fv( _pt->pt(_node[i+j]) );
      }
    }
  } else {							// no data
    glColor4fv( colour );
    for ( int j=0; j<4; j++ ) {
      if ( lightson && ptnrml ) glNormal3fv( ptnrml+_node[i+j]*3 );
      glVertex3fv( _pt->pt(_node[i+j]) );
    }
  }
}


/** draw a quadrilateral
 *
 *  \param e      element to highlight
 *  \param colour colour to use
 *  \param size   size of element
 */
void Quadrilateral :: draw( int e, GLfloat *colour, float size )
{
  if ( e<_n ) {

    int eind = 4*e;

    if (!_pt->vis(_node[eind])||!_pt->vis(_node[eind+1])||
        !_pt->vis(_node[eind+2]) || !_pt->vis(_node[eind+3]) )
      return;

    glLineWidth( size );
    glBegin(GL_QUADS );
    glColor4fv( colour );
    glVertex3fv( _pt->pt(_node[eind++]) );
    glVertex3fv( _pt->pt(_node[eind++]) );
    glVertex3fv( _pt->pt(_node[eind++]) );
    glVertex3fv( _pt->pt(_node[eind])   );
    glEnd();
  }
}


/** read in the triangle file */
bool Quadrilateral :: read( const char *fname )
{
  gzFile in;

  try {
    in = openFile( fname, "tris" );
  } catch (...) { return false; }

  const int bufsize=1024;
  char      buff[bufsize];
  int       nold=_n, nele;

  while ( gzgets(in, buff, bufsize) != Z_NULL ) {
    sscanf( buff, "%d", &nele );
    _n += nele;
    _node  = (int *)realloc( _node, _n*3*sizeof(int) );
    _nrml = (GLfloat*)realloc( _nrml, _n*sizeof(GLfloat)*3 );
    for ( int i=_n-nele; i<_n; i++ ) {
      gzgets(in, buff, bufsize);
      sscanf( buff, "%d %d %d %*d", _node+3*i, _node+3*i+1, _node+3*i+2 );
    }
  }
  gzclose(in);

  try {
    read_normals( nold, _n-1, fname );
  } catch (...) { compute_normals( nold, _n-1 ); }

  return true;
}


/** increase the number of triangles
 *
 * \param fn   file name with elements to be added
 *
 * \return true if successful
 */
bool Quadrilateral :: add( const char* fn )
{
 return false;
}



/** for a surface with ordered node numbers, compute the normals
 *
 * \param e0 first element
 * \param e1 last element
 */
void Quadrilateral::compute_normals(int e0, int e1)
{
  int e, n;

  if (_nrml==NULL) _nrml=new GLfloat[3*e1+3];
  for ( e=e0; e<=e1; e++ ) {

    const int* ele = _node+3*e;
    Vector3D<GLfloat> a(_pt->pt(ele[0]));
    Vector3D<GLfloat> b(_pt->pt(ele[1]));
    Vector3D<GLfloat> c(_pt->pt(ele[2]));

    b -= a;
    b = b.Cross(c-a);
    b.Normalize();
    _nrml[3*e]   = b.X();
    _nrml[3*e+1] = b.Y();
    _nrml[3*e+2] = b.Z();
  }
}



const int* 
Quadrilateral::iso_polys(unsigned int index)
{
  return quad_iso_table[index];
}
