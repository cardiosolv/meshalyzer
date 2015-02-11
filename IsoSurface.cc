// shared
#include <VecData.h>

// project
#include "IsoSurface.h"

#define BRANCH_TOL 0.2

/** constructor
 *
 * \param m      model
 * \param dat    data displayed on model
 * \param v      isovalue
 * \param member true if part of model
 * \param t      time
 */
IsoSurface::IsoSurface(Model* m, DATA_TYPE *dat, double v, 
		       vector<bool>&member, int t, double *branch )
  :_vertnorm(NULL),_vert(NULL),_val(v),_tm(t)
{
  double brmin = branch ? branch[0]+BRANCH_TOL*(branch[1]-branch[0]) : 0;
  double brmax = branch ? branch[1]-BRANCH_TOL*(branch[1]-branch[0]) : 0;

#pragma omp parallel for schedule(dynamic,100)
  for( int i=0; i<m->numVol(); i++ ) {
    if( member[i] ) {

      // ignore elements crossing the branch cut
      if( branch ) {
        const int* nodes= m->_vol[i]->obj();
        bool low=false, high=false;
        for( int n=0; n<m->_vol[i]->ptsPerObj(); n++ ) {
          if( dat[nodes[n]] <= brmin ) low=true;
          if( dat[nodes[n]] >= brmax ) high=true;
        }
        if( low && high ) 
          continue;
      }

      int npoly;
      MultiPoint **lpoly = m->_vol[i]->isosurf( dat, _val, npoly );

      if (lpoly != NULL){
        for( int j=0; j<npoly; j++ )
        {
#pragma omp critical
          polygon.push_back(static_cast<SurfaceElement*>(lpoly[j]));
        }
        delete[] lpoly;
      }
    }
  }
    
  // determine the vertex normals for all the polygons
  //determine_vert_norms(m->pt);
}

IsoSurface::~IsoSurface()
{
  for( int i=0; i<polygon.size(); i++ ) {
    delete polygon[i]->pt();
    delete polygon[i];
  }
  delete _vertnorm;
}

void IsoSurface::draw()
{
  glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
  for( int i=0; i<polygon.size(); i++ ) {
    polygon[i]->draw(0, 0, colour, NULL, NULL, 1, NULL, polygon[i]->ptnrml() );
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
  GLfloat *tvn = (GLfloat *)calloc(pt.num()*3,sizeof(GLfloat));

#pragma omp parallel for
  for ( int i = 0; i < polygon.size(); i++ ) {
    const GLfloat* n;
    if ( (n=polygon[i]->nrml(0)) == NULL ) continue;
    const int *pnt = polygon[i]->obj();
    for ( int j=0; j<polygon[i]->ptsPerObj(); j++ ) {
      for ( int k=0; k<3; k++ ) 
#pragma omp atomic
        tvn[3*pnt[j]+k] += n[k];
      has_norm[pnt[j]] = true;
    }
  }
  // count \# nodes in surface
  int numvert = 0;
#pragma omp parallel for
  for ( int i=0; i<pt.num(); i++ ) {
    if ( has_norm[i] ) {
      normalize(tvn+i*3);
#pragma omp atomic
      numvert++;
    }
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
  free(tvn);
}
