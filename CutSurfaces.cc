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
  if( !(endind%ADD_INC) ) {
	_ele      = static_cast<SurfaceElement **>(realloc( _ele, 
					(endind/ADD_INC+1)*ADD_INC*sizeof(SurfaceElement *) ));
	_ptarr    = static_cast<GLfloat **>(realloc( _ptarr, 
					(endind/ADD_INC+1)*ADD_INC*sizeof(GLfloat *) ));
	_interp = static_cast<Interpolator<DATA_TYPE> **>(realloc( _interp, 
				(endind/ADD_INC+1)*ADD_INC*sizeof(Interpolator<DATA_TYPE> *)));
	_norm     = static_cast<GLfloat *>(realloc( _norm, 
					(endind/ADD_INC+1)*ADD_INC*sizeof(GLfloat)*3 ));
  }
  _ele[endind]    = se;
  _ptarr[endind]  = const_cast<GLfloat *>(const_cast<Point*>(se->pt())->pt());
  _interp[endind] = ni;
  memcpy( _norm+3*endind, n, sizeof(GLfloat)*3 ); 
  endind++;
}


/** assume that since everything is temporary, it will be our perogative
 * to destroy it all
 */
CutSurfaces::~CutSurfaces()
{
  if( endind ) {
	for( int i=0; i<endind; i++ ) {
	  delete _ele[i];
	  delete _ptarr[i];
	  delete _interp[i];
	}
	free( _ele );
	free( _ptarr );
	free( _norm );
	free( _interp );
  }
}

