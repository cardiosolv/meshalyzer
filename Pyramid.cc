#include "DrawingObjects.h"

const int pyr_num_edge=8;
static int pyr_edges[pyr_num_edge][2] =
  {
    {0,1},{1,2},{2,3},{3,0},{0,4},{1,4},{2,4},{3,4}
  };
static const int pyramid_iso_table[][15] = {
    { 0 },
    { 1, 3, 0, 1, 0, 3, 0, 4 }, 
    { 1, 3, 1, 0, 1, 2, 1, 4 },
    { 1, 4, 0, 4, 4, 1, 1, 2, 0, 3 },
    { 1, 3, 1, 2, 2, 3, 2, 4 },
    { 1, 4, 0, 4, 4, 2, 2, 3, 3, 0 },
    { 1, 4, 1, 4, 1, 0, 2, 3, 2, 4 },
    { 1, 5, 2, 4, 1, 4, 0, 4, 0, 3, 3, 2 },
    { 1, 3, 2, 3, 3, 0, 3, 4 },
    { 1, 4, 1, 0, 0, 4, 4, 3, 3, 2 },
    { 2, 3, 0, 1, 1, 2, 1, 4, 3, 0, 3, 3, 4, 2, 3 },
    { 1, 5, 1, 4, 4, 0, 4, 3, 3, 2, 2, 1 },
    { 1, 4, 1, 2, 2, 4, 4, 3, 3, 0 },
    { 1, 5, 2, 4, 4, 3, 0, 4, 0, 1, 1, 2 },
    { 1, 5, 0, 1, 1, 4, 4, 2, 4, 3, 0, 3 },
    { 1, 4, 0, 4, 4, 1, 4, 2, 4, 3 },   
    { 1, 4, 0, 4, 4, 1, 4, 2, 4, 3 },
    { 1, 5, 0, 1, 1, 4, 4, 2, 4, 3, 0, 3 },
    { 1, 5, 2, 4, 4, 3, 0, 4, 0, 1, 1, 2 },
    { 1, 4, 1, 2, 2, 4, 4, 3, 3, 0 },
    { 1, 5, 1, 4, 4, 0, 4, 3, 3, 2, 2, 1 },
    { 2, 3, 0, 1, 1, 2, 4, 1, 3, 0, 3, 3, 4, 2, 3 },
    { 1, 4, 1, 0, 0, 4, 4, 3, 3, 2 },
    { 1, 3, 2, 3, 3, 0, 3, 4 },
    { 1, 5, 2, 4, 1, 4, 0, 4, 0, 3, 3, 2 },
    { 1, 4, 1, 4, 1, 0, 2, 3, 2, 4 },
    { 1, 4, 0, 4, 4, 2, 2, 3, 3, 0 },
    { 1, 3, 1, 2, 2, 3, 2, 4 },
    { 1, 4, 0, 4, 4, 1, 1, 2, 0, 3 },
    { 1, 3, 1, 0, 1, 2, 1, 4 },
    { 1, 3, 0, 1, 0, 3, 0, 4 },
    { 0 }
};
const int pyr_num_surf = 5;
const int pyr_surface_table[][4] = {
   {0,1,2,3}, {4,2,1,-1}, {4,3,2,-1}, {4,0,3,-1}, {4,1,0,-1}
};


/** draw many Pyramids
 *
 *  \param p0       first index of point to draw
 *  \param p1       last index of point to draw
 *  \param colour   colour to use if no data
 *  \param data     data associated with nodes (NULL for no data display)
 *  \param stride   draw every n'th point
 */
void Pyramid::draw( int p0, int p1, GLfloat *colour, Colourscale* cs,
                    DATA_TYPE* data, int stride, dataOpac* dataopac )
{
  if ( data != NULL ) {
    for ( int p=5*p0; p<=5*p1+4; p+= stride*5 ) {
      int an, bn, cn, dn, en;

      if (!_pt->vis(_node[p])||!_pt->vis(_node[p+1])||!_pt->vis(_node[p+2])||
          !_pt->vis(_node[p+3])||!_pt->vis(_node[p+4])  )
        continue;

      const GLfloat* a =  _pt->pt(an=_node[p]);
      const GLfloat* b =  _pt->pt(dn=_node[p+1]);
      const GLfloat* c =  _pt->pt(cn=_node[p+2]);
      const GLfloat* d =  _pt->pt(dn=_node[p+3]);
      const GLfloat* e =  _pt->pt(dn=_node[p+4]);

      GLfloat* ca = cs->colorvec( data[an] );
      GLfloat* cb = cs->colorvec( data[bn] );
      GLfloat* cc = cs->colorvec( data[cn] );
      GLfloat* cd = cs->colorvec( data[dn] );
      GLfloat* ce = cs->colorvec( data[en] );

      glBegin(GL_TRIANGLE_FAN);
      glColor3fv( ce );
      glVertex3fv( e );
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
      glEnd();
      glBegin(GL_QUADS);
      glColor3fv( ca );
      glVertex3fv( a );
      glColor3fv( cb );
      glVertex3fv( b );
      glColor3fv( cc );
      glVertex3fv( c );
      glColor3fv( cd );
      glVertex3fv( d );
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
void Pyramid :: draw( int t, GLfloat *colour, float size )
{
  glLineWidth( size );
  glColor4fv( colour );

  if ( t<_n ) {

    t *= 5;

    if (!_pt->vis(_node[t])||!_pt->vis(_node[t+1])||!_pt->vis(_node[t+2])||
        !_pt->vis(_node[t+3])||!_pt->vis(_node[t+4])  )
      return;

    const GLfloat* a =  _pt->pt(_node[t++]);
    const GLfloat* b =  _pt->pt(_node[t++]);
    const GLfloat* c =  _pt->pt(_node[t++]);
    const GLfloat* d =  _pt->pt(_node[t++]);
    const GLfloat* e =  _pt->pt(_node[t]);

    glBegin(GL_TRIANGLE_FAN);
    glVertex3fv( e );
    glVertex3fv( a );
    glVertex3fv( b );
    glVertex3fv( c );
    glVertex3fv( d );
    glVertex3fv( a );
    glEnd();
    glBegin(GL_QUADS);
    glVertex3fv( a );
    glVertex3fv( b );
    glVertex3fv( c );
    glVertex3fv( d );
    glEnd();
  }
}


/** read in the point file */
bool Pyramid :: read( const char *fname )
{
  gzFile in;

  try {
    in = openFile( fname, "pyr" );
  } catch (...) {return false;}

  const int bufsize=1024;
  char buff[bufsize];
  if ( gzgets(in, buff, bufsize) == Z_NULL ) return false;
  sscanf( buff, "%d", &_n );
  _node    = new int[5*_n];
  _region = new int[_n];

  for ( int i=0; i<_n; i++ ) {
    gzgets(in, buff, bufsize);
    if ( sscanf( buff, "%d %d %d %d %d %d", _node+5*i, _node+5*i+1,
                 _node+5*i+2, _node+5*i+3, _node+5*i+4, _region+i  ) < 6   ) {
      // if region not specified, assign a default of -1
      _region[i] = -1;
    }
  }
  gzclose(in);
}


/** draw the faces of an element not contaiining a node
 *
 * \param cn do not draw faces associated with this node
 */
void Pyramid::draw_out_face( int cn )
{
  for ( int i=0; i<_n*_ptsPerObj; i++ )
    if ( _node[i] == cn ) {
      int elenode = (i/_ptsPerObj)*_ptsPerObj;
      int nn = i%_ptsPerObj;

      if ( nn==4 ) {
        glBegin(GL_QUADS);
        glVertex3fv( _pt->pt(_node[elenode+0]) );
        glVertex3fv( _pt->pt(_node[elenode+1]) );
        glVertex3fv( _pt->pt(_node[elenode+2]) );
        glVertex3fv( _pt->pt(_node[elenode+3]) );
        glEnd();
      } else {
        glBegin(GL_TRIANGLES);
        glVertex3fv( _pt->pt(_node[elenode+(nn+3)%4]) );
        glVertex3fv( _pt->pt(_node[elenode+(nn+2)%4]) );
        glVertex3fv( _pt->pt(_node[elenode+4]) );

        glVertex3fv( _pt->pt(_node[elenode+(nn+2)%4]) );
        glVertex3fv( _pt->pt(_node[elenode+(nn+1)%4]) );
        glVertex3fv( _pt->pt(_node[elenode+4]) );
        glEnd();
      }
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
Pyramid::cut( char *pd, GLfloat* cp,
              Interpolator<DATA_TYPE>* &interp, int e )
{
  return planecut( pd, cp, interp, pyr_num_edge, pyr_edges, e );
}


/* return the list of intersection polygons for isosurfaces
 *
 *  \param index for each node, the bit is true if the isoval is exceeded
 *  
 * \return the row in the table
 */
const int* Pyramid::iso_polys(unsigned int index)
{
  return pyramid_iso_table[index];
}


/* 
 * return list of lists of nodes defining the bounding surfaces
 *
 * \param ft face table
 * \param v  element number
 *
 * \param pointer to a vector of vectors
 *
 * \pre the size of ft is big enough
 */
int 
Pyramid::surfaces( int ft[][MAX_NUM_SURF_NODES+1], int v )
{
  return make_surf_nodelist( v, ft, pyr_num_surf, 
        int(pyr_surface_table[1]-pyr_surface_table[0]), (const int **)pyr_surface_table );
}
