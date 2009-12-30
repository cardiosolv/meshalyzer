#include "AuxGrid.h"

char *get_line(gzFile);

int get_num( gzFile in )
{
  int number;
  if( !sscanf( get_line(in), "%d", &number ) )
	throw 1;
  return number;
}

/* constructor 
 *
 * \param fn        base file name
 * \param pt_offset center of model coordinates
 *
 * \throw 1 if any error in input
 */
AuxGrid::AuxGrid( char *fn, const GLfloat* pt_offset ):_num_tm_grid(1),
		_num_tm_data(0),_data(NULL),_display(true)
{
  char *ext = strstr( fn, ".pts_t" );
  if( ext ) *ext = '\0';

  gzFile pt_in;
  try {
    pt_in = openFile( fn, ".pts_t" );
  }catch(...) {throw 1;}

  _num_tm_grid = get_num( pt_in );

  gzFile elem_in;
  try { 
    elem_in = openFile( fn, ".elem_t" );
  } catch(...) {elem_in=NULL;}

  gzFile data_in;
  try {
    data_in = openFile( fn, ".dat_t" );
  } catch(...) {data_in=NULL;}

  //sanity checks
  if( elem_in && get_num( elem_in ) != _num_tm_grid ) {
    cerr << "number of vertex and element time instances do not match" << endl;
    throw 1;
  }
  if( data_in ) {
    _num_tm_data = get_num( data_in );
    if( _num_tm_grid>1 && _num_tm_grid!=_num_tm_data ) {
      cerr << "number of vertex and data time instances do not match" << endl;
      throw 1;
    }
  }

  _num_mod = _num_tm_grid>_num_tm_data?_num_tm_grid:_num_tm_data;

  _data = new DATA_TYPE*[_num_tm_data];
  memset( _data, 0, sizeof(DATA_TYPE *)*_num_tm_data );

  // read in first time step
  _model.push_back(new Model());
  _model.back()->read_instance( pt_in, elem_in, pt_offset );
  if( data_in ) 
	read_data_instance( data_in, _model.back()->number(Vertex), _data[0] );

  for( int i=1; i<_num_mod; i++ ) {
    if( _num_tm_grid>1 ) {
      _model.push_back(new Model());
      _model.back()->read_instance( pt_in, elem_in, pt_offset );  
    }
    if( _num_tm_data>1 )
      read_data_instance( data_in, _model.back()->number(Vertex), _data[i] ); 
  }
  for( int i=0; i<maxobject; i++ ) {
    _show[i]     = true;
    _datafied[i] = false;
  }
  threeD( Cnnx, true );
}


/** draw the grid at a particular time 
 *
 * \param t the time
 */
void 
AuxGrid :: draw( int t )
{
  if( !_display || t<0 || t>=_num_mod ) return;

  Model *m = _model[_num_tm_grid==1 ? 0 : t ];

  if( _autocol ) {
    optimize_cs(t);
    //auxmincolval->value(cs.min());
    //auxmaxcolval->value(cs.max());
  }

  if( _show[Vertex] ) {
	GLfloat r[]={0,1,0,1};
	m->pt.draw( 0, m->pt.num()-1, m->get_color(Vertex),
			&cs, (_data&&_datafied[Vertex])?_data[t]:NULL, 1, NULL );
  }

  if( _show[Cnnx] ) {
	m->_cnnx->draw( 0, m->_cnnx->num()-1, m->get_color(Cnnx),
			&cs, (_data&&_datafied[Cnnx])?_data[t]:NULL, 1, NULL );
  }

  if( _show[SurfEle]  ) {
	glPushAttrib(GL_POLYGON_BIT);
	if( _surf_fill ) {
	  glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
	} else {
	  glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
	  glLineWidth(m->size(SurfEle,0));
	}

	GLfloat r[]={0,1,1,1};
    m->surface(0)->draw( m->get_color(SurfEle), &cs, 
			     (_data&&_datafied[SurfEle])?_data[t]:NULL, 1, NULL, 
			                    m->vertex_normals(m->surface(0)) );
    glPopAttrib();
  }

  if( _show[VolEle] && m->_vol ) {
    glPushAttrib(GL_POLYGON_BIT);
    if( _vol_fill ) {
      glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    } else {
      glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
      glLineWidth(m->size(VolEle,0));
    }
    for( int i=0; i<m->number(VolEle); i++ )
      m->_vol[i]->draw( 0, m->_cnnx->num()-1, m->get_color(VolEle),
              &cs, (_data&&_datafied[VolEle])?_data[t]:NULL, 1, NULL );
  }
  glPopAttrib();
}


/** read in data for one time instance 
 *
 * \param in   input file
 * \param npt  number of expected data points for instance
 * \param data where to store data
 *
 * \throw 1 input error
 * \post data will be allocated
 */
void 
AuxGrid::read_data_instance( gzFile in, int npt, DATA_TYPE*& data )
{
  int n;
  sscanf( get_line(in), "%d", &n );
  if( n!=npt )
    throw 1;
  data = new DATA_TYPE[n];
  for( int i=0; i<n; i++ ) {
    if( !sscanf(get_line( in ), "%f", data+i ) )
      throw 1;
  }
}


/** destructor */
AuxGrid::~AuxGrid()
{
  for( int i=0; i<_model.size(); i++ ) {
	delete   _model[i];
	delete[] _data[i];
  }

  _model.clear();
  delete[] _data;
}


/** set the size of objects */
void
AuxGrid::size( Object_t o, float s )
{
  for( int i=0; i<_model.size(); i++ ) {
    switch(o) {
        case Vertex:
            _model[i]->pt.size(s);
            break;
        case Cnnx:
            _model[i]->_cnnx->size(s);
            break;
        case SurfEle:
        case VolEle:
            break;
    }
    _model[i]->size(o,-1,s);
  }
}


/** set the 3Dedness of objects */
void
AuxGrid::threeD( Object_t o, bool s )
{
  for( int i=0; i<_model.size(); i++ ){
    switch(o) {
        case Vertex:
            _model[i]->pt.threeD(s);
            break;
        case Cnnx:
            _model[i]->_cnnx->threeD(s);
            break;
        case SurfEle:
        case VolEle:
            break;
    }
    _model[i]->threeD(o,-1,s);
  }
}


/** set the colour of objects */
void
AuxGrid::color( Object_t o, GLfloat *s )
{
  for( int i=0; i<_model.size(); i++ )
    _model[i]->set_color(o,-1,s[0],s[1],s[2],s[3]);
}


void 
AuxGrid :: optimize_cs( int tm )
{
  if( _data==NULL ) return;
  DATA_TYPE min=_data[tm][0], max=_data[tm][0];
  for( int i=1; i<_model[tm]->pt.num(); i++ ) {
    if( _data[tm][i]<min )
      min=_data[tm][i];
    if( _data[tm][i]>max )
      max=_data[tm][i];
  }
  cs.calibrate( min, max );
}


/** assignment operator */
AuxGrid& 
AuxGrid::operator=( const AuxGrid &a ) 
{
  memcpy( _show, a._show, maxobject*sizeof(bool) );
  memcpy( _datafied, a._datafied, maxobject*sizeof(bool) );
  _display   = a._display;
  _surf_fill = a._surf_fill;
  _vol_fill  = a._vol_fill;
  _autocol   = a._autocol;
  color( Vertex,  a.color( Vertex )  );
  color( Cnnx,    a.color( Cnnx )    );
  color( SurfEle, a.color( SurfEle ) );
  color( VolEle,  a.color( VolEle )  );
  threeD( Vertex,  a.threeD( Vertex )  );
  threeD( Cnnx,    a.threeD( Cnnx )    );
  threeD( SurfEle, a.threeD( SurfEle ) );
  threeD( VolEle,  a.threeD( VolEle )  );
  size( Vertex,  a.size( Vertex )  );
  size( Cnnx,    a.size( Cnnx )    );
  size( SurfEle, a.size( SurfEle ) );
  size( VolEle,  a.size( VolEle )  );

  return *this;
}
