#include "Surfaces.h"
#include <string.h>
#include <VecData.h>

Surfaces::Surfaces( SurfaceElement **e, int st, int end ) :is_visible(true),_filled(true),_outline(false),_ele(e),startind(st),endind(end),_vertnorm(NULL)
{
  fillcolor( 1., 0.5, 0.1 );
  outlinecolor( 0.125, 0.8, 0.7 );
  _vert = (int *)malloc( sizeof(int) );
  _vert[0] = -1;
}



void Surfaces :: fillcolor( float r, float g, float b, float a )
{
  _fillcolor[0] = r;
  _fillcolor[1] = g;
  _fillcolor[2] = b;
  _fillcolor[3] = a;
}


//! set outline colour
void Surfaces :: outlinecolor( float r, float g, float b, float a )
{
  _outlinecolor[0] = r;
  _outlinecolor[1] = g;
  _outlinecolor[2] = b;
  _outlinecolor[3] = a;
}


/** get the vertex normals for the surface
 *
 * \param vm vector of normals for all points
 *
 */
void Surfaces::get_vert_norms( GLfloat *vn )
{
  int i=0;
  
  while( _vert[i] >= 0 ){
	memcpy( vn+_vert[i]*3, _vertnorm+i*3, 3*sizeof(GLfloat) );
    i++;
  }
}
	

/** set the vertex normals for the surface
 *
 * \param vm vector of normals for all points
 *
 */
void Surfaces::determine_vert_norms( Point& pt )
{
  vector<bool> has_norm(pt.num());  // number of elements attached to node

  GLfloat *tvn = new GLfloat[pt.num()*3];
  memset(  tvn, 0, pt.num()*3*sizeof(GLfloat) );

  const int* tris = _ele[0]->obj()+startind*3;
  const GLfloat* n;
  
  for( int i=startind; i<=endind; i++ ) {
	if( (n=_ele[0]->nrml(i)) == NULL ) continue;
	for( int j=0; j<_ele[0]->ptsPerObj(); j++ ) {
	  for( int k=0; k<3; k++ ) tvn[*tris*3+k] += n[k];
	  has_norm[*tris++] = true;
	}
  }
  // count \# nodes in surface
  int numvert = 0;
  for( int i=0; i<pt.num(); i++ ) {
	if( has_norm[i] ) normalize(tvn+i*3);
	numvert++;
  }

  _vert = (int *)realloc(_vert, sizeof(int)*(numvert+1));
  _vertnorm = new GLfloat[3*numvert];
  numvert = 0;
  for( int i=0; i<pt.num(); i++ ) {
	if( has_norm[i] ) {
	  memcpy( _vertnorm+3*numvert, tvn+i*3, sizeof(GLfloat)*3 );
	  _vert[numvert++] = i;
	}
  }
  _vert[numvert] = -1;
  delete[] tvn;
}

