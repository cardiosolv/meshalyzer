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
		_val.push_back(val);
        _polygon.push_back(lpoly[j]);
        num_lines++;
      }
    }
  }
  return num_lines;
}

int 
IsoLine :: process( CutSurfaces *s, DATA_TYPE *dat )
{
  int num_lines=0;
  for( int i=0; i<_nl; i++ ){
    double val = _nl==1? _v0: _v0 + i*(_v1-_v0)/(float)(_nl-1.);

    for( int j=0; j<s->num(); j++ ) {

      DATA_TYPE idata[s->ele(j)->ptsPerObj()];
        for ( int v=0; v<s->ele(j)->ptsPerObj(); v++ )
          idata[v] = s->interpolate( j, dat, v );
      int npoly;
      MultiPoint **lpoly = s->ele(j)->isosurf( dat, val, npoly );
      for( int j=0; j<npoly; j++ ) {
        _polygon.push_back(lpoly[j]);
		_val.push_back(val);
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
  if( cs==NULL )
    for( int i=0; i<_polygon.size(); i++ ) {
      _polygon[i]->draw(0, _color, size );
  } else {
    for( int i=0; i<_polygon.size(); i++ ) {
      _polygon[i]->draw( 0, cs->colorvec(_val[i]), size );
	}
  }
}

