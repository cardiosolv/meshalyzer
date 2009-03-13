#include "Surfaces.h"
#include "Region.h"
#include <string.h>

int RRegion_sort( const void *a, const void *b )
{
  return (*(RRegion **)a)->label() - (*(RRegion **)b)->label();
}

void RRegion:: initialize( int n, int l )
{
  is_visible = true;
  _label = l;
  set_color( Vertex, 0., 1., 0. );
  set_color( Cable, 0., 0., 1. );
  set_color( Cnnx, 1., 0., 1. );
  set_color( Tetrahedron, 0., 0., 1. );
  memset( startind, 0, maxobject*sizeof(int)  );
  memset( endind,   0, maxobject*sizeof(int)  );
  memset( showobj,  0, maxobject*sizeof(bool) );
  memset( _3D,      0, maxobject*sizeof(bool) );
  _member.resize(n);
  _member.assign(n,false);
  for( int i=0; i<maxobject; i++ ) _size[i] = 1.;
}


/** constructor
 *
 * \param v   volume list
 * \param nv  number of volumes
 * \param n   number of points
 * \param l   label for region
 */
RRegion::RRegion( VolElement **v, int nv, int n, int l )
{
  startind[Tetrahedron] = -1;

  initialize( n, l );
  for ( int j=0; j<nv; j++ ) {
    const int* ele = v[j]->obj();
    for ( int e=0; e<v[j]->num(); e++ )
      if ( v[j]->region(e) == l ) {
        for ( int i=0; i<v[j]->ptsPerObj(); i++ ) {
          _member[ele[e*v[j]->ptsPerObj()+i]] = true;
        }
        endind[Tetrahedron] = j;
        if ( startind[Tetrahedron]<0 ) startind[Tetrahedron] = j;
      }
  }

  startind[Vertex] = -1;
  for ( int i=0; i<n; i++ )
    if ( _member[i] == true ) {
      endind[Vertex] = i;
      if ( startind[Vertex]<0 ) startind[Vertex] = i;
    }
}


/** constructor
 *
 * \param n number of points
 * \param l label for region
 * \param b initialial value
 */
RRegion::RRegion( int n, int l, bool b )
{
  initialize( n, l );
  _member.assign(n, b);
}


void RRegion :: set_color( Object_t obj, float r, float g, float b, float a )
{
  color[obj][0] = r;
  color[obj][1] = g;
  color[obj][2] = b;
  color[obj][3] = a;
}

