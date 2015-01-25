#include "IsoLines.h"

/** determine if the branch cut passes through the element 
 *
 *  \param d   data values
 *  \param n   number of data
 *  \param min range min  
 *  \param max range max
 *  \param tol allowed difference
 *
 *  \return true iff the branch passes thorugh the element
 */
bool
cross_branch( DATA_TYPE *d, int n, double min, double max, double tol )
{
  float mine=d[0], maxe=d[0];

  for( int i=0; i<n; i++ ) {
    if( d[i] > maxe ) 
        maxe = d[i];

    if( d[i] < mine ) 
        mine = d[i];
  }
  if( maxe-mine > (1.-tol)*(max-min) )
    return true;

  return false;
}


int 
IsoLine :: process( Surfaces *s, DATA_TYPE *dat )
{
  int num_lines=0;

  for( int i=0; i<_nl; i++ ){
    double val = _nl==1? _v0: _v0 + i*(_v1-_v0)/(float)(_nl-1.);
    for( int j=0; j<s->num(); j++ ) {
      int npoly;
      MultiPoint **lpoly = s->ele(j)->isosurf( dat, val, npoly );
      if( npoly && _branch ) {
        const int*nodes= s->ele(j)->obj();
        DATA_TYPE edat[MAX_NUM_SURF_NODES];
        for( int i=0; i< s->ele(j)->ptsPerObj(); i++ ){
          edat[i] = dat[nodes[i]];
        }
        if(cross_branch(edat,s->ele(j)->ptsPerObj(), _branch_range[0], _branch_range[1], _branch_tol))
          continue;
      }
      for( int k=0; k<npoly; k++ ) {
		_val.push_back(val);
        _polygon.push_back(lpoly[k]);
        num_lines++;
      }
    }
  }
  return num_lines;
}


#if 0
int 
IsoLine :: process( CutSurfaces *s, DATA_TYPE *dat )
{
  int num_lines=0;
  DATA_TYPE *edat = NULL;

  for( int i=0; i<_nl; i++ ){
    double val = _nl==1? _v0: _v0 + i*(_v1-_v0)/(float)(_nl-1.);

    for( int j=0; j<s->num(); j++ ) {

      DATA_TYPE idata[s->ele(j)->ptsPerObj()];
        for ( int v=0; v<s->ele(j)->ptsPerObj(); v++ )
          idata[v] = s->interpolate( j, dat, v );
      int npoly;
      MultiPoint **lpoly = s->ele(j)->isosurf( idata, val, npoly );
      if( npoly && _branch ) {
        const int*nodes= s->ele(j)->obj();
        edat = (DATA_TYPE *)realloc(edat, sizeof(DATA_TYPE)*s->ele(j)->ptsPerObj() );
        for( int i=0; i< s->ele(j)->ptsPerObj(); i++ ){
          edat[i] = dat[nodes[i]];
        }
        if(cross_branch(edat,s->ele(j)->ptsPerObj(), _branch_range[0], _branch_range[1], _branch_tol))
          continue;
      for( int k=0; k<npoly; k++ ) {
        _polygon.push_back(lpoly[k]);
		_val.push_back(val);
        num_lines++;
      }
    }
  }
  return num_lines;
}
#endif

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

