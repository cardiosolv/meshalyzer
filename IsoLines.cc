#include "IsoLines.h"

int 
IsoLine :: process( Surfaces *s, DATA_TYPE *dat )
{
  int num_lines=0;
  for( int i=0; i<_nl; i++ ){
    double val = _nl==1? _v0: _v0 + i*(_v1-_v0)/(float)(_nl-1.);
    for( int j=0; j<s->num(); j++ ) {
      int npoly;
      MultiPoint **lpoly = s->ele(j)->isosurf( dat, val, npoly );
      for( int j=0; j<npoly; j++ ) {
        _polygon.push_back(lpoly[j]);
        num_lines++;
      }
    }
  }
  return num_lines;
}


IsoLine::~IsoLine()
{
  for( int i=0; i<_polygon.size(); i++ ) {
    delete _polygon[i]->pt();
    delete _polygon[i];
  }
}

void IsoLine::draw( Colourscale *cs, GLfloat size )
{
  for( int i=0; i<_polygon.size(); i++ ) {
    _polygon[i]->draw(0, _color, size );
  }
}

