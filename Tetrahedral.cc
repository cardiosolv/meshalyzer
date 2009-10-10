#include "DrawingObjects.h"

const int tet_num_edge=6;
static int tet_edges[tet_num_edge][2] =
  {
    {0,1},{1,2},{2,0},{0,3},{1,3},{2,3}
  };
const int tetra_iso_table[][10] = {
    { 0 },
    { 1, 3, 0, 1, 0, 2, 0, 3 },
    { 1, 3, 0, 1, 1, 3, 1, 2 },
    { 1, 4, 1, 2, 0, 2, 3, 0, 1, 3 },
    { 1, 3, 1, 2, 2, 3, 2, 0 },
    { 1, 4, 0, 1, 1, 2, 2, 3, 3, 0 },
    { 1, 4, 0, 1, 1, 3, 2, 3, 0, 2 },
    { 1, 3, 3, 0, 3, 1, 3, 2 },  
    { 1, 3, 3, 0, 3, 2, 3, 1 },
    { 1, 4, 0, 1, 0, 2, 2, 3, 1, 3 },
    { 1, 4, 0, 1, 3, 0, 2, 3, 1, 2 },
    { 1, 3, 1, 2, 2, 0, 2, 3 },
    { 1, 4, 1, 2, 1, 3, 3, 0, 0, 2 },
    { 1, 3, 0, 1, 1, 2, 1, 3 },
    { 1, 3, 0, 1, 0, 3, 0, 2 },
    { 0 }
};
const int tet_num_surf =4;
const int tetra_surface_table[][3] = {
   {0, 2, 1}, {0, 1, 3}, {0, 3, 2}, {2, 3, 1}
};


/** draw many Tetrahedrals
 *
 *  \param p0       first index of point to draw
 *  \param p1       last index of point to draw
 *  \param colour   colour to use if no data
 *  \param data     data associated with nodes (NULL for no data display)
 *  \param stride   draw every n'th point
 */
void Tetrahedral::draw( int p0, int p1, GLfloat *colour, Colourscale* cs,
                        DATA_TYPE* data, int stride, dataOpac* dataopac )
{
  if ( data != NULL ) {
    for ( int p=4*p0; p<=4*p1+3; p+= stride*4 ) {
      int an, bn, cn, dn;

      if (!_pt->vis(_node[p])||!_pt->vis(_node[p+1])||!_pt->vis(_node[p+2])||
          !_pt->vis(_node[p+3])   )
        continue;

      const GLfloat* a =  _pt->pt(an=_node[p]);
      const GLfloat* b =  _pt->pt(dn=_node[p+1]);
      const GLfloat* c =  _pt->pt(cn=_node[p+2]);
      const GLfloat* d =  _pt->pt(dn=_node[p+3]);

      GLfloat* ca = cs->colorvec( data[an] );
      GLfloat* cb = cs->colorvec( data[bn] );
      GLfloat* cc = cs->colorvec( data[cn] );
      GLfloat* cd = cs->colorvec( data[dn] );

      glBegin(GL_TRIANGLE_STRIP);
      glColor3fv( ca );
      glVertex3fv( a );
      glColor3fv( cb );
      glVertex3fv( b );
      glColor3fv( cc );
      glVertex3fv( c );
      glColor3fv( cd );
      glVertex3fv( d );
      glColor3fv( ca );
      glVertex3fv( a );
      glColor3fv( cb );
      glVertex3fv( b );
      glEnd();
    }
  } else
    for ( int p=p0; p<=p1; p++ )
      draw( p, colour, 1. );
}


/** draw a tetrahedron
 *
 *  \param e      node to highlight
 *  \param colour colour to use
 *  \param size   size of point
 */
void Tetrahedral :: draw( int t, GLfloat *colour, float size )
{
  glLineWidth( size );
  glColor4fv( colour );

  if ( t<_n ) {

    t *= 4;

    if (!_pt->vis(_node[t])||!_pt->vis(_node[t+1])||!_pt->vis(_node[t+2])||
        !_pt->vis(_node[t+3]) )
      return;

    const GLfloat* a =  _pt->pt(_node[t++]);
    const GLfloat* b =  _pt->pt(_node[t++]);
    const GLfloat* c =  _pt->pt(_node[t++]);
    const GLfloat* d =  _pt->pt(_node[t]);

    glBegin(GL_TRIANGLE_STRIP);
    glVertex3fv( a );
    glVertex3fv( b );
    glVertex3fv( c );
    glVertex3fv( d );
    glVertex3fv( a );
    glVertex3fv( b );
    glEnd();
  }
}


/** read in the point file */
bool Tetrahedral :: read( const char *fname )
{
  gzFile in;

  try {
    in = openFile( fname, "tetras" );
  } catch (...) {return false;}

  const int bufsize=1024;
  char buff[bufsize];
  if ( gzgets(in, buff, bufsize) == Z_NULL ) return false;
  sscanf( buff, "%d", &_n );
  _node    = new int[4*_n];
  _region = new int[_n];

  for ( int i=0; i<_n; i++ ) {
    gzgets(in, buff, bufsize);
    if ( sscanf( buff, "%d %d %d %d %d", _node+4*i, _node+4*i+1,
                 _node+4*i+2, _node+4*i+3, _region+i  ) < 5    ) {
      // if region not specified, assign a default of -1
      _region[i] = -1;
    }
  }
  gzclose(in);
}


/** draw the faces of an element not contaiining a node
 *
 * \param n do not draw faces associated with this node
 */
void Tetrahedral::draw_out_face( int n )
{
  for ( int i=0; i<_n*_ptsPerObj; i++ )
    if ( _node[i] == n ) {
      int elenode = (i/_ptsPerObj)*_ptsPerObj;
      glBegin(GL_TRIANGLES);
      for ( int j=0; j<4; j++ )
        if ( _node[elenode+j] != n )
          glVertex3fv( _pt->pt(_node[elenode+j]) );
      glEnd();
    }
}


/** determine if a clipping plane intersects an element.
 *
 *  \param pd     visible points
 *  \param cp     clipping plane, cp[0]x + cp[1]y +cp[2]z + cp[3] = 0
 *  \param e      the element in the list
 *  \param interp construct to interpolate data
 *
 *  \return a surface element if intersection, NULL otherwise
 *
 *  \post interp may be allocated
 */
SurfaceElement*
Tetrahedral::cut( char *pd, GLfloat* cp,
                  Interpolator<DATA_TYPE>* &interp, int e )
{
  return planecut( pd, cp, interp, tet_num_edge, tet_edges, e );
}


const int*
Tetrahedral::iso_polys(unsigned int index)
{
  return tetra_iso_table[index];
}


/* 
 * return list of lists of nodes defining the bounding surfaces
 *
 * \param v element number
 *
 * \param pointer to a vector of vectors
 */
vector<vector<int> > * 
Tetrahedral::surfaces( int v )
{
  return make_surf_nodelist( tet_num_surf, int(tetra_surface_table[1]-tetra_surface_table[0]), 
                             (const int **)tetra_surface_table, v );
}

  
