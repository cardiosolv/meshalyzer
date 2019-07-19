#include "DrawingObjects.h"
#include "gzFileBuffer.h"

static const int tri_iso_table[][6] = { 
  {0,0,0,0,0,0},{1,2,0,1,0,2},{1,2,1,0,1,2},{1,2,1,2,0,2},
  {1,2,2,0,2,1},{1,2,0,1,2,1},{1,2,0,1,0,2},{0,0,0,0,0,0} };

/** draw many Triangles
 *
 *  \param p0       first index of point to draw
 *  \param p1       last index of point to draw
 *  \param colour   colour to use if no data
 *  \param cs       colour scale
 *  \param data     data associated with nodes (NULL for no data display)
 *  \param stride   draw every n'th point
 *  \param dataopac data opacity
 */
void Triangle::draw( int p0, int p1, GLfloat *colour, Colourscale* cs,
                     DATA_TYPE* data, int stride, dataOpac* dataopac )
{
  draw( p0, p1, colour, cs, data, stride, dataopac, _ptnrml );
}

/** draw one Triangle but do not call glBegin/glEnd
 *
 *  \param p0       index of first point to draw
 *  \param colour   colour to use if no data
 *  \param cs       colour scale
 *  \param data     data associated with nodes (NULL for no data display)
 *  \param dataopac data opacity
 *  \param ptnrml   vertex normals (NULL for none)
 */
void Triangle::draw( int p0, GLfloat *colour, Colourscale* cs,
                     DATA_TYPE* data, dataOpac* dataopac,
                     const GLfloat* ptnrml, bool lightson )
{
  if ( p0>=_n ) return;

  int i=3*p0;
  if ( !_pt->vis(_node[i]) || !_pt->vis(_node[i+1]) || !_pt->vis(_node[i+2]) )
    return;

  for ( int j=0; j<3; j++ ) {
    if (data)
      cs->colourize( data[_node[i+j]], dataopac->on() ? dataopac->alpha(data[_node[i+j]]) : colour[3] );
    else
      glColor4fv( colour );

    if ( lightson && ptnrml ) 
      glNormal3fv( ptnrml+_node[i+j]*3 );
    glVertex3fv( _pt->pt(_node[i+j]) );
  }	    
}


/** draw many Triangles
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
void Triangle::draw( int p0, int p1, GLfloat *colour, Colourscale* cs,
                     DATA_TYPE* data, int stride, dataOpac* dataopac,
                     const GLfloat* ptnrml )
{
  if ( p0>=_n || p1>_n ) return;

  GLboolean lightson;
  glGetBooleanv( GL_LIGHTING, &lightson );

  glBegin(GL_TRIANGLES);

  for ( int i=3*p0; i<=3*p1; i+=3*stride ) {
    if ( !_pt->vis(_node[i]) || !_pt->vis(_node[i+1]) || !_pt->vis(_node[i+2]) )
      continue;

    for ( int j=0; j<3; j++ ) 
    {
      if (data)
        cs->colourize( data[_node[i+j]], dataopac->on() ? dataopac->alpha(data[_node[i+j]]) : colour[3] );
      else
        glColor4fv( colour );

      if ( lightson && ptnrml ) 
        glNormal3fv( ptnrml+_node[i+j]*3 );
      glVertex3fv( _pt->pt(_node[i+j]) );
    }	    
  }

  glEnd();
}


/** draw a triangle
 *
 *  \param e      element to highlight
 *  \param colour colour to use
 *  \param size   size of element
 */
void Triangle :: draw( int e, GLfloat *colour, float size )
{
  if ( e<_n ) {

    int eind = 3*e;

    if (!_pt->vis(_node[eind])||!_pt->vis(_node[eind+1])||!_pt->vis(_node[eind+2]))
      return;

    glLineWidth( size );
    glBegin(GL_TRIANGLES );
    glColor4fv( colour );
    glVertex3fv( _pt->pt(_node[eind++]) );
    glVertex3fv( _pt->pt(_node[eind++]) );
    glVertex3fv( _pt->pt(_node[eind])   );
    glEnd();
  }
}


/** read in the triangle file */
bool Triangle :: read( const char *fname )
{
  gzFile in;

  try {
    in = openFile( fname, "tris" );
  } catch (...) { return false; }

  const int bufsize=1024;
  char      buff[bufsize];
  int       nold=_n, nele;

  gzFileBuffer file(in);
  while ( file.gets(buff, bufsize) != Z_NULL ) {
    sscanf( buff, "%d", &nele );
    _n += nele;
    _node  = (int *)realloc( _node, _n*3*sizeof(int) );
    _nrml = (GLfloat*)realloc( _nrml, _n*sizeof(GLfloat)*3 );
    for ( int i=_n-nele; i<_n; i++ ) {
      file.gets(buff, bufsize);
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
bool Triangle :: add( const char* fn )
{
  int    nele;
  string nfn = fn;

  try {
    nele = countInFile( fn );
  } catch (...) { return false; }

  const int bufsize=1024;
  char      buff[bufsize];

  _node  = (int *)realloc( _node,  3*(_n+nele)*sizeof(int) );
  _nrml = (GLfloat*)realloc( _nrml, (_n+nele)*sizeof(GLfloat)*3 );

  int nold = _n;
  _n += nele;

  gzFile in = gzopen( fn, "r" );

  for ( int i=_n-nele; i<_n; i++ ) {
    if ( gzgets(in, buff, bufsize)==Z_NULL )
      throw 1;
    if ( sscanf( buff, "%d %d %d %*d", _node+3*i, _node+3*i+1, _node+3*i+2 )<3 )
      i--;
  }

  gzclose(in);

  // remove tri or tris suffix
  if ( nfn.substr( nfn.size()-4 ) == ".tri" )
    nfn.erase( nfn.size()-3 );
  else if ( nfn.substr( nfn.size()-5 ) == ".tris" )
    nfn.erase( nfn.size()-4 );

  try {
    read_normals( nold, _n-1, nfn.c_str() );
  } catch (...) {
    compute_normals( nold, _n-1 );
  }

  return true;
}


/** Count the number of triangles in a file
 *
 *  if the first line contains less than 3 integers, assume it is the number
 *  of Triangles
 *
 *  \param fn file name 0 must have .tri or .tris suffix
 *
 *  \return the \# of elements
 */
int Triangle::countInFile( const char* fn )
{
  int        numtri=0;
  int        a, b, c;
  static int bufsize=1024;
  char       buff[bufsize];
  gzFile     in;

  if ( (in=gzopen( fn, "r" )) == NULL )
    throw(1);

  gzgets(in, buff, bufsize);
  if ( sscanf( buff, "%d %d %d", &a, &b, &c )==3 ) {
    numtri = 1;
    while ( gzgets(in, buff, bufsize) != Z_NULL ) {
      if ( sscanf( buff, "%d %d %d", &a, &b, &c ) == 3 ) numtri++;
    }
  } else {
    do {
      numtri += a;
      for ( int i=0; i<a; i++ ) gzgets(in, buff, bufsize);
    } while ( gzgets( in, buff, bufsize)!=Z_NULL &&
              sscanf( buff, "%d %d %d", &a, &b, &c )==1 );
  }

  gzclose( in );

  if ( numtri < 1 )
    throw(1);

  return numtri;
}


/** for a surface with ordered node numbers, compute the normals
 *
 * \param e0 first element
 * \param e1 last element
 */
void Triangle::compute_normals(int e0, int e1)
{
  if ( _nrml==NULL ) _nrml=new GLfloat[e1*3+3];

  for ( int e=e0; e<=e1; e++ ) {

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



const int* Triangle::iso_polys(unsigned int index)
{
  return tri_iso_table[index];
}
