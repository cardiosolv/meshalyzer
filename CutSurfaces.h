#ifndef CUTSURFACES_H
#define CUTSURFACES_H

#include <stdio.h>
#include <stdlib.h>
#include "DrawingObjects.h"
#include "Surfaces.h"
#include "Interpolator.h"


/** Class for a temporary cut plane
 *
 *  It is assumed that vertices will be changed if the plane is moved as
 *  well as all the elements
 */
class CutSurfaces : public Surfaces
{
  public:
    CutSurfaces();
    ~CutSurfaces();
    void get_vert_norms( GLfloat *vn ){}
    void determine_vert_norms( PPoint & ){}
    void addEle( SurfaceElement *e, GLfloat *n, Interpolator<DATA_TYPE>* );
    DATA_TYPE  interpolate( int e, DATA_TYPE *d, int p )
    {
      return _interp[e][p].interpolate( d );
    }
    DATA_TYPE  interpolate( int e, DATA_TYPE *d, int p, double *bc )
    {
      return _interp[e][p].interpolate( d, bc );
    }
    GLfloat* norm( int e ){ return _norm+3*e; }
  protected:
    GLfloat      **_ptarr;
    GLfloat      *_norm;
    Interpolator<DATA_TYPE> **_interp;
};

#endif
