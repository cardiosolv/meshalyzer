#include "CutSurfaces.h"
#include <string.h>
#include <VecData.h>
#include <stdlib.h>

const int ADD_INC = 1000;

CutSurfaces::CutSurfaces() : _norm(NULL),_ptarr(NULL), _interp(NULL)
{
  fillcolor( 1., 0, 0.1 );
  outlinecolor( 0.125, 0.8, 0.7 );
}


/** add a Surface element to the surface
 *
 *  \param se the element
 *  \param n  the normal
 *  \param ni list for data interpolation
 *
 *  \pre each element pointer points to a single element and not a list
 *
 *  \note points are not shred between elements
 */
void
CutSurfaces :: addEle( SurfaceElement *se, GLfloat *n, Interpolator<DATA_TYPE> *ni )
{
  int _numele = _ele.size();
  if ( !(_ele.size()%ADD_INC) ) {
    _ptarr    = static_cast<GLfloat **>(realloc( _ptarr,
                                        (_numele/ADD_INC+1)*ADD_INC*sizeof(GLfloat *) ));
    _interp = static_cast<Interpolator<DATA_TYPE> **>(realloc( _interp,
              (_numele/ADD_INC+1)*ADD_INC*sizeof(Interpolator<DATA_TYPE> *)));
    _norm     = static_cast<GLfloat *>(realloc( _norm,
                                       (_numele/ADD_INC+1)*ADD_INC*sizeof(GLfloat)*3 ));
  }
  _ele.push_back(se);
  _ptarr[_numele]  = const_cast<GLfloat *>(const_cast<Point*>(se->pt())->pt());
  _interp[_numele] = ni;
  memcpy( _norm+3*_numele, n, sizeof(GLfloat)*3 );
}


/** assume that since everything is temporary, it will be our perogative
 * to destroy it all
 */
CutSurfaces::~CutSurfaces()
{
  if ( !_ele.empty() ) {
    for ( int i=0; i<_ele.size(); i++ ) {
      delete _ele[i];
      delete _ptarr[i];
      delete _interp[i];
    }
    free( _ptarr );
    free( _norm );
    free( _interp );
    _ele.clear();
  }
}

