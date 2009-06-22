#include "IsoSurface.h"

IsoSurface::IsoSurface(Model* m, DATA_TYPE *dat, double v, vector<bool>&member)
:_val(v)
{
  for( int i=0; i<m->numVol(); i++ ) {
    if( member[i] ) {
      int npoly;
      MultiPoint **lpoly = m->_vol[i]->isosurf( dat, _val, npoly );
      for( int j=0; j<npoly; j++ )
        polygon.push_back(lpoly[j]);
    }
  }
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
    polygon[i]->draw(0, colour, 1 );
  }
}

