// shared
#include "VecData.h"

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
 * \param branch branch cut information (NULL if none)
 */
IsoSurface::IsoSurface(Model* m, DATA_TYPE *dat, double v, 
		       vector<bool>&member, int t, double *branch )
  :_vertnorm(NULL),_val(v),_tm(t)
{
  double brmin = branch ? branch[0]+BRANCH_TOL*(branch[1]-branch[0]) : 0;
  double brmax = branch ? branch[1]-BRANCH_TOL*(branch[1]-branch[0]) : 0;

  EdgePtMap ep_map;

#pragma omp parallel for schedule(dynamic,100) shared(ep_map, _pts, polygon)
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
      MultiPoint **lpoly = m->_vol[i]->isosurf( dat, _val, npoly, _pts, ep_map );

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
    
  determine_vert_norms();
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
  for( int i=0; i<polygon.size(); i++ ) 
    polygon[i]->draw(0, 0, colour, NULL, NULL, 1, NULL, _vertnorm );
}

/******************************************************************************
 * \brief Determine vertex normal vectors  of the surface
 *
 * \note Applying the Gourade shading by taking the average
 *       of all adjacent vertex normals
 *
 ******************************************************************************/
void IsoSurface::determine_vert_norms()
{
  vector<bool> has_norm(_pts.num());      //!< does the vertex have a normal?
  _vertnorm = new GLfloat[3*_pts.num()];

#pragma omp parallel for
  for ( int i=0; i<polygon.size(); i++ ) {
    const GLfloat* n;
    if ( (n=polygon[i]->nrml(0)) == NULL ) continue;
    const int *pnt = polygon[i]->obj();
    for ( int j=0; j<polygon[i]->ptsPerObj(); j++ ) {
      for ( int k=0; k<3; k++ ) 
#pragma omp atomic
        _vertnorm[3*pnt[j]+k] += n[k];
      has_norm[pnt[j]] = true;
    }
  }

#pragma omp parallel for
  for ( int i=0; i<_pts.num(); i++ ) {
    if ( has_norm[i] ) 
      normalize(_vertnorm+i*3);
  }
}


/** output the surface as an auxilliary grid
 *
 * \param basename base filename
 *
 * \pre the files have been opened and the number of elements and points 
 *      written to it
*/
void 
IsoSurface::saveAux( string basename, int pt_offset ) 
{
  string ptfile = basename+".pts_t";
  FILE*pout = fopen( ptfile.c_str(), "a" );
  const GLfloat *p = _pts.pt();
  for( int i=0; i<_pts.num(); i++, p+=3 )
    fprintf( pout, "%.1f %.1f %.1f\n", p[0], p[1], p[2] );
  fclose( pout );

  string elemfile = basename+".elem_t";
  FILE *eout = fopen( elemfile.c_str(), "a" );
  for( auto const& poly: polygon ) {
    const int* n = poly->obj();
    for( int j=0; j<poly->ptsPerObj(); j++ ){
      fprintf( pout, "%d ", n[j]+pt_offset );
    }
    fprintf( pout, "\n" );
  }
  fclose( eout );
}

