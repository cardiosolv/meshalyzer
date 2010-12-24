// shared
#include <VecData.h>

// project
#include "IsoSurface.h"

/** constructor
 *
 * \param m      model
 * \param dat    data displayed on model
 * \param v      isovalue
 * \param member true if part of model
 * \param t      time
 */
IsoSurface::IsoSurface(Model* m, DATA_TYPE *dat, double v, 
		       vector<bool>&member, int t)
  :_vertnorm(NULL),_vert(NULL),_val(v),_tm(t)
{
  for( int i=0; i<m->numVol(); i++ ) {
    if( member[i] ) {
      int npoly;
      MultiPoint **lpoly = m->_vol[i]->isosurf( dat, _val, npoly );
	  
      if (lpoly != NULL){
        for( int j=0; j<npoly; j++ )
        {
          polygon.push_back(static_cast<SurfaceElement*>(lpoly[j]));
        }
        delete[] lpoly;
      }
    }
  }
    
  // determine the vertex normals for all the polygons
  determine_vert_norms(m->pt);
}

IsoSurface::~IsoSurface()
{
  for( int i=0; i<polygon.size(); i++ ) {
    delete polygon[i]->pt();
    delete polygon[i];
  }
}

void IsoSurface::draw()
{
  glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
  for( int i=0; i<polygon.size(); i++ ) {
    polygon[i]->draw(0, 0, colour, NULL, NULL, 1, NULL, _vertnorm );
  }
}

/******************************************************************************
 * \brief Determine the vertex normal vector of the \a polygon
 *
 * \note Applying the Gourade shading by taking the average
 *       of all adjacent vertex normals
 *
 * \param polygon - the polygon to determine the vertex normal vector
 *
 ******************************************************************************/
void IsoSurface::determine_vert_norms(PPoint& pt)
{
  vector<bool> has_norm(pt.num());  // if elements attached to node

  GLfloat *tvn = new GLfloat[pt.num()*3];
  memset( tvn, 0, pt.num()*3*sizeof(GLfloat) );

  const GLfloat* n;

  for ( int i = 0; i < polygon.size(); i++ )
  {
    if ( (n=polygon[i]->nrml(0)) == NULL ) continue;
    const int *pnt = polygon[i]->obj();
    for ( int j=0; j<polygon[i]->ptsPerObj(); j++ ) {
      for ( int k=0; k<3; k++ ) tvn[3*pnt[j]+k] += n[k];
      has_norm[pnt[j]] = true;
    }
  }
  // count \# nodes in surface
  int numvert = 0;
  for ( int i=0; i<pt.num(); i++ ) {
    if ( has_norm[i] ) normalize(tvn+i*3);
    numvert++;
  }

  _vert = (int *)realloc(_vert, sizeof(int)*(numvert+1));
  _vertnorm = new GLfloat[3*numvert];
  numvert = 0;

  for ( int i=0; i<pt.num(); i++ ) {
    if ( has_norm[i] ) {
      memcpy( _vertnorm+3*numvert, tvn+i*3, sizeof(GLfloat)*3 );
      _vert[numvert++] = i;
    }
  }

  _vert[numvert] = -1;
  delete[] tvn;
}
