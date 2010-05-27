#include "DrawingObjects.h"

const int prism_num_edge=9;
const int prism_edges[prism_num_edge][2] =
  {
    {0,1},{1,2},{2,0},{0,3},{1,5},{2,4},{3,4},{4,5},{5,3}
  };
const int prism_iso_table[][17] = {
    { 0 },
    { 1, 3, 0, 1, 0, 2, 0, 3 },
    { 1, 3, 1, 0, 1, 2, 1, 5 },
    { 1, 4, 0, 2, 1, 2, 1, 5, 0, 3 },
    { 1, 3, 0, 2, 1, 2, 2, 4 },
    { 1, 4, 0, 3, 2, 4, 2, 1, 1, 0 },
    { 1, 4, 0, 2, 0, 1, 1, 5, 2, 4 },
    { 1, 3, 0, 3, 2, 4, 1, 5 },
    { 1, 3, 0, 3, 3, 4, 3, 5 },
    { 1, 4, 0, 2, 0, 1, 3, 5, 3, 4 },
    { 2, 3, 0, 1, 1, 5, 0, 3, 3, 1, 2, 3, 5, 3, 4 }, 
    { 1, 4, 4, 3, 3, 5, 1, 5, 1, 2 },
    { 2, 3, 0, 3, 0, 2, 2, 4, 3, 2, 1, 3, 5, 3, 4 },
    { 1, 5, 0, 1, 5, 3, 3, 4, 4, 2, 2, 1 },
    { 2, 4, 0, 3, 0, 2, 2, 4, 3, 4, 3, 0, 1, 3, 5, 1, 5 },
    { 1, 4, 2, 4, 1, 5, 3, 5, 3, 4 },
    { 1, 3, 3, 4, 4, 5, 2, 4 },
    { 2, 3, 0, 3, 3, 4, 4, 2, 3, 0, 1, 0, 2, 4, 5 },
    { 2, 3, 3, 4, 4, 5, 2, 4, 3, 2, 1, 1, 0, 1, 5 },
    { 2, 3, 0, 2, 0, 3, 3, 4, 4, 1, 2, 2, 4, 4, 5, 1, 5 },
    { 1, 4, 0, 2, 2, 1, 5, 4, 4, 3 },
    { 1, 5, 0, 3, 3, 4, 4, 5, 2, 1, 1, 0 },
    { 1, 5, 0, 1, 0, 2, 3, 4, 4, 5, 5, 1 },
    { 1, 4, 0, 3, 3, 4, 4, 5, 5, 1 },
    { 1, 4, 0, 3, 2, 4, 4, 5, 5, 3 },
    { 1, 5, 2, 4, 3, 5, 4, 5, 0, 1, 0, 2 },
    { 2, 4, 1, 5, 5, 4, 4, 2, 2, 1, 3, 0, 1, 0, 3, 3, 5 },
    { 2, 3, 1, 2, 2, 0, 3, 5, 3, 2, 4, 4, 5, 1, 5 },
    { 1, 5, 0, 3, 3, 5, 5, 4, 1, 2, 0, 2 },
    { 1, 4, 0, 1, 1, 2, 4, 5, 5, 3 },
    { 2, 3, 0, 3, 3, 5, 1, 5, 3, 0, 1, 0, 2, 5, 4 },
    { 1, 3, 1, 5, 5, 4, 5, 3 }, 
    { 1, 3, 1, 5, 5, 4, 5, 3 },
    { 2, 3, 0, 3, 3, 5, 1, 5, 3, 0, 1, 0, 2, 5, 4 },
    { 1, 4, 0, 1, 1, 2, 4, 5, 5, 3 },
    { 1, 5, 0, 3, 3, 5, 5, 4, 1, 2, 0, 2 },
    { 2, 3, 1, 2, 2, 0, 3, 5, 3, 2, 4, 4, 5, 1, 5 },
    { 2, 4, 1, 5, 5, 4, 4, 2, 2, 1, 3, 0, 1, 0, 3, 3, 5 },
    { 1, 5, 2, 4, 3, 5, 4, 5, 0, 1, 0, 2 },
    { 1, 4, 0, 3, 2, 4, 4, 5, 5, 3 },
    { 1, 4, 0, 3, 3, 4, 4, 5, 5, 1 },
    { 1, 5, 0, 1, 0, 2, 3, 4, 4, 5, 5, 1 },
    { 1, 5, 0, 3, 3, 4, 4, 5, 2, 1, 1, 0 },
    { 1, 4, 0, 2, 2, 1, 5, 4, 4, 3 },
    { 2, 3, 0, 2, 0, 3, 3, 4, 4, 1, 2, 2, 4, 4, 5, 1, 5 },
    { 2, 3, 3, 4, 4, 5, 2, 4, 3, 0, 1, 1, 2, 1, 5 },
    { 2, 3, 0, 3, 3, 4, 4, 2, 3, 0, 1, 0, 2, 4, 5 },
    { 1, 3, 3, 4, 4, 5, 2, 4 },
    { 1, 4, 2, 4, 1, 5, 3, 5, 3, 4 },
    { 2, 4, 0, 3, 0, 2, 2, 4, 3, 4, 3, 0, 1, 3, 5, 1, 5 },
    { 1, 5, 0, 1, 5, 3, 3, 4, 4, 2, 2, 1 },
    { 2, 3, 0, 3, 0, 2, 2, 4, 3, 2, 1, 3, 5, 3, 4 },
    { 1, 4, 4, 3, 3, 5, 1, 5, 1, 2 },
    { 2, 3, 0, 1, 1, 5, 0, 3, 3, 1, 2, 3, 5, 3, 4 }, 
    { 1, 4, 0, 2, 0, 1, 3, 5, 3, 4 },
    { 1, 3, 0, 3, 3, 4, 3, 5 },
    { 1, 3, 0, 3, 2, 4, 1, 5 },
    { 1, 4, 0, 2, 0, 1, 1, 5, 2, 4 },
    { 1, 4, 0, 3, 2, 4, 2, 1, 1, 0 },
    { 1, 3, 0, 2, 1, 2, 2, 4 },
    { 1, 4, 0, 2, 1, 2, 1, 5, 0, 3 },
    { 1, 3, 1, 0, 1, 2, 1, 5 },
    { 1, 3, 0, 1, 0, 2, 0, 3 },
    { 0 }
};
const int prism_num_surf = 5;
const int prism_surface_table[][4] = {
   {0,1,2,-1}, {0,2,4,3}, {2,1,5,4}, {1,5,3,0}, {3,4,5,-1}
};


/** draw many Prisms
 *
 *  \param p0       first index of point to draw
 *  \param p1       last index of point to draw
 *  \param colour   colour to use if no data
 *  \param data     data associated with nodes (NULL for no data display)
 *  \param stride   draw every n'th point
 */
void Prism::draw( int p0, int p1, GLfloat *colour, Colourscale* cs,
                  DATA_TYPE* data, int stride, dataOpac* dataopac )
{
  if ( data != NULL ) {
    for ( int p=6*p0; p<=6*p1+3; p+= stride*6 ) {
      int an, bn, cn, dn, en, fn;

      if (!_pt->vis(_node[p])||!_pt->vis(_node[p+1])||!_pt->vis(_node[p+2])||
          !_pt->vis(_node[p+3]) ||!_pt->vis(_node[p+4])||!_pt->vis(_node[p+5]) )
        continue;

      const GLfloat* a =  _pt->pt(an=_node[p]);
      const GLfloat* b =  _pt->pt(dn=_node[p+1]);
      const GLfloat* c =  _pt->pt(cn=_node[p+2]);
      const GLfloat* d =  _pt->pt(dn=_node[p+3]);
      const GLfloat* e =  _pt->pt(en=_node[p+4]);
      const GLfloat* f =  _pt->pt(fn=_node[p+5]);

      GLfloat* ca = cs->colorvec( data[an] );
      GLfloat* cb = cs->colorvec( data[bn] );
      GLfloat* cc = cs->colorvec( data[cn] );
      GLfloat* cd = cs->colorvec( data[dn] );
      GLfloat* ce = cs->colorvec( data[en] );
      GLfloat* cf = cs->colorvec( data[fn] );

      glBegin(GL_TRIANGLES);
      glColor3fv( ca );
      glVertex3fv( a );
      glColor3fv( cb );
      glVertex3fv( b );
      glColor3fv( cc );
      glVertex3fv( c );
      glColor3fv( cd );
      glVertex3fv( d );
      glColor3fv( ce );
      glVertex3fv( e );
      glColor3fv( cf );
      glVertex3fv( f );
      glEnd();
      glBegin(GL_QUAD_STRIP);
      glColor3fv( ca );
      glVertex3fv( a );
      glColor3fv( cd );
      glVertex3fv( d );
      glColor3fv( cc );
      glVertex3fv( c );
      glColor3fv( ce );
      glVertex3fv( e );
      glColor3fv( cb );
      glVertex3fv( b );
      glColor3fv( cf );
      glVertex3fv( f );
      glColor3fv( ca );
      glVertex3fv( a );
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
void Prism :: draw( int t, GLfloat *colour, float size )
{
  glLineWidth( size );
  glColor4fv( colour );

  if ( t<_n ) {

    t *= 6;

    if (!_pt->vis(_node[t])||!_pt->vis(_node[t+1])||!_pt->vis(_node[t+2])||
        !_pt->vis(_node[t+3]) ||!_pt->vis(_node[t+4])||!_pt->vis(_node[t+5]) )
      return;

    const GLfloat* a =  _pt->pt(_node[t++]);
    const GLfloat* b =  _pt->pt(_node[t++]);
    const GLfloat* c =  _pt->pt(_node[t++]);
    const GLfloat* d =  _pt->pt(_node[t++]);
    const GLfloat* e =  _pt->pt(_node[t++]);
    const GLfloat* f =  _pt->pt(_node[t]);

    glBegin(GL_TRIANGLES);
    glVertex3fv( a );
    glVertex3fv( b );
    glVertex3fv( c );
    glVertex3fv( d );
    glVertex3fv( e );
    glVertex3fv( f );
    glEnd();
    glBegin(GL_QUAD_STRIP);
    glVertex3fv( a );
    glVertex3fv( d );
    glVertex3fv( c );
    glVertex3fv( e );
    glVertex3fv( b );
    glVertex3fv( f );
    glVertex3fv( a );
    glVertex3fv( d );
    glEnd();
  }
}


/** read in the point file */
bool Prism :: read( const char *fname )
{
  gzFile in;

  try {
    in = openFile( fname, "tetras" );
  } catch (...) {return false;}

  const int bufsize=1024;
  char buff[bufsize];
  if ( gzgets(in, buff, bufsize) == Z_NULL ) return false;
  sscanf( buff, "%d", &_n );
  _node    = new int[6*_n];
  _region = new int[_n];

  for ( int i=0; i<_n; i++ ) {
    gzgets(in, buff, bufsize);
    if ( sscanf( buff, "%d %d %d %d %d %d %d", _node+6*i, _node+6*i+1,
                 _node+6*i+2, _node+6*i+3, _node+6*i+4, _node+6*i+5, _region+i ) < 7 ) {
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
void Prism::draw_out_face( int cn )
{
  for ( int i=0; i<_n*_ptsPerObj; i++ )
    if ( _node[i] == cn ) {
      int nn = i%_ptsPerObj;
      int elenode = i - nn;

      glBegin(GL_TRIANGLES);
      if ( nn>2 ) {
        glVertex3fv( _pt->pt(_node[elenode+3]) );
        glVertex3fv( _pt->pt(_node[elenode+4]) );
        glVertex3fv( _pt->pt(_node[elenode+5]) );
      } else {
        glVertex3fv( _pt->pt(_node[elenode+0]) );
        glVertex3fv( _pt->pt(_node[elenode+1]) );
        glVertex3fv( _pt->pt(_node[elenode+2]) );
      }
      glEnd();
      glBegin(GL_QUADS);
      if ( nn==1 || nn==5 ) {
        glVertex3fv( _pt->pt(_node[elenode+0]) );
        glVertex3fv( _pt->pt(_node[elenode+2]) );
        glVertex3fv( _pt->pt(_node[elenode+4]) );
        glVertex3fv( _pt->pt(_node[elenode+3]) );
      } else if ( nn==0 || nn==3 ) {
        glVertex3fv( _pt->pt(_node[elenode+2]) );
        glVertex3fv( _pt->pt(_node[elenode+1]) );
        glVertex3fv( _pt->pt(_node[elenode+5]) );
        glVertex3fv( _pt->pt(_node[elenode+4]) );
      } else {
        glVertex3fv( _pt->pt(_node[elenode+0]) );
        glVertex3fv( _pt->pt(_node[elenode+3]) );
        glVertex3fv( _pt->pt(_node[elenode+5]) );
        glVertex3fv( _pt->pt(_node[elenode+1]) );
      }
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
Prism::cut( char* pd, GLfloat* cp,
            Interpolator<DATA_TYPE>* &interp, int e )
{
  return planecut( pd, cp, interp, prism_num_edge, prism_edges, e );
}


const int* 
Prism::iso_polys(unsigned int index)
{
  return prism_iso_table[index];
}


/* 
 * return list of lists of nodes defining the bounding surfaces
 *
 * \param ft face table to be filled in
 * \param v element number
 *
 * \param pointer to a vector of vectors
 */
int
Prism::surfaces( int **ft, int v )
{
  return make_surf_nodelist( v, ft, prism_num_surf,
          int(prism_surface_table[1]-prism_surface_table[0]), 
                             (const int **)prism_surface_table );
}
