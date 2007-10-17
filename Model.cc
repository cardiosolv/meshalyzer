#include "Model.h"
#include<set>
#include "DataOpacity.h"
#include "gl2ps.h"
#include "VecData.h"


Model::Model(Colourscale *cs, DataOpacity *dopac ):
			_cs(cs),_dataopac(dopac),_base1(false), 
			_surface(NULL), numSurf(0), _vertnrml(NULL), 
			_numReg(0), _region(NULL), _numVol(0), _vol(NULL)
{  
  for( int i=0; i<maxobject; i++ ) _outstride[i] = 1;
}


/** read in the geometrical description
 *
 * \param flwindow
 * \param fnt      base file to open
 * \param base1    points begin numbering at 0
 */
const int bufsize=1024;
bool Model::read( const char* fnt, bool base1 )
{
  char fn[bufsize];
  strcpy( fn, fnt );
  gzFile in;

  _base1 = base1;
  pt.base1( base1 );
  
  try {
    pt.read( fn );
  }catch(...){
	fprintf(stderr, "Unable to read proper points file\n" ); 
	exit(1);
  }
  _pts = pt.pt();
  allvis.resize(pt.num());
  allvis.assign(pt.num(), true );

  if( !strcmp( ".gz", fn+strlen(fn)-3 ) ) // temporary
		  fn[strlen(fn)-3] = '\0';
  if( !strcmp( ".pts", fn+strlen(fn)-4 ) ) // temporary
		  fn[strlen(fn)-3] = '\0';
  if( fn[strlen(fn)-1] != '.' ) {
	fn[strlen(fn)+1] ='\0';
	fn[strlen(fn)] ='.';
  }

  _cnnx   = new Connection( &pt );
  _cnnx->read( fn );
  _cable  = new ContCable( &pt );
  _cable->read( fn );
  
  read_elem_file( fn );
  read_region_file( in, fn );
  determine_regions();
  
  _triele  = new Triangle( &pt );
  add_surface( fn );

  // find maximum dimension and bounding box
  const GLfloat *p = pt.pt();
  _maxdim =p[0];
  for( int i=1; i<pt.num()*3; i++ )
	if( p[i]>_maxdim ) _maxdim = p[i];

  _vertnrml= new GLfloat[3*pt.num()];

  return true;
}


/** determine which points are part of all the regions 
 *
 * \pre all objects (vertices, cables, etc.) have been read in
 */
void Model::determine_regions()
{
  if (!_numVol && !_numReg  ){
    _region = (Region **)calloc( 1, sizeof(Region *) );
    _region[0] = new Region( pt.num() ); //0 ==default region label
	_numReg = 1;
	return;
  }

  if( _numVol ) {
	_region = (Region **)realloc( _region, (_numReg+1)*sizeof(Region *) );
	_region[_numReg] = new Region(_vol, _numVol, pt.num(), _vol[0]->region(0));
	_numReg++;
	for( int i=1; i<_numVol; i++ ) {
	  int r;
	  for( r=0; r<_numReg; r++ )
		if( _region[r]->label() == _vol[i]->region(0) )
		  break;
	  if( r==_numReg ) {
		_numReg++;
		_region = (Region**)realloc( _region, sizeof(Region)*_numReg );
		_region[_numReg-1]=new Region(_vol,_numVol,pt.num(),_vol[i]->region(0));
	  }
	}
  }

  // make sure that all points are included
  vector<bool> hasregion(pt.num(),false);
  for( int r=0; r<_numReg; r++ ) 
	for( int i=0; i<pt.num(); i++ )
	  hasregion[i] = hasregion[i] | _region[r]->member(i);

  for( int i=0; i<pt.num(); i++ )
	if( !hasregion[i] ) {  // point without a region detected
	  int  r, newlabel=0;  // must create a unique region number
	  while( true ) {
		for( r=0; r<_numReg; r++ )
		  if( _region[r]->label() == newlabel )
			break;;
		if( r==_numReg )
		  break;
		newlabel++;
	  }
	  _numReg++;
	  _region = (Region**)realloc( _region, sizeof(Region)*_numReg );
	  _region[_numReg-1] = new Region( pt.num(), newlabel, false );
	  for( int i=0; i<pt.num(); i++ )
		if( !hasregion[i] ) _region[_numReg-1]->member( i, true );
	  break;
	}


#ifdef _ARCH_PPC
  heapsort( _region, _numReg, sizeof(Region *), Region_sort );
#else
  qsort( _region, _numReg, sizeof(Region *), Region_sort );
#endif

  // find first cable in layer
  if( _cable != NULL ) {
	for( int r=0; r<_numReg; r++ ) {
	  for( int i=0; i<_cable->num(); i++ ) {
		const int *o = _cable->obj(i);
		int  c;
		for( c=o[0]; c<o[1]; c++ )
		  if( _region[i]->member(c) == true ) {
			_region[r]->first(Cable,i);
			break;
		  }
		if( c<o[1] )
		  break;
	  }
	}
  }

  // find first connection in layer
  if( _cnnx != NULL ) {
	for( int r=0; r<_numReg; r++ ) {
	  for( int i=0; i<_cnnx->num(); i++ ) {
		const int *o = _cnnx->obj(i);
		if( _region[r]->member(o[0])==true||_region[r]->member(o[1])==true ) {
		  _region[r]->first(Cnnx,i);
		  break;
		}
	  }
	}
  }
}

/** add a surface by reading in a .tri file, also try reading a normal file
 *
 * \param fn   file containing tri's
 *
 * \return     the \#surfaces, -1 if not successful
 */
int Model::add_surface( const char *fn )
{
  int oldNumEle = _triele->num();

  /* determine file name */
  gzFile in;
  string fname(fn);
  if( (in=gzopen( fname.c_str(), "r" )) == NULL ) {
	fname += "tri";
	if( (in=gzopen( fname.c_str(), "r" )) == NULL ) {
	  fname += "s";
	  if( (in=gzopen( fname.c_str(), "r" )) == NULL ) {
		fname += ".gz";
		if( (in=gzopen( fname.c_str(), "r" )) == NULL ) {
		  fname = fn;
		  fname += "tri.gz";
		  if( (in=gzopen( fname.c_str(), "r" )) == NULL ) {
			fprintf( stderr, "Could not open surface file\n" );
			return -1;
		  }
		}
	  }
	}
  }

  try {
	_triele->add( fname.c_str() );
  }
  catch(...) { return -1; }
  
  int sn = numSurf;
  _surface=(Surfaces **)realloc(_surface,++numSurf*sizeof(Surfaces *));
  _surface[numSurf-1] = new Surfaces( (SurfaceElement**)(&_triele), oldNumEle, _triele->num()-1 );

  // reread tri file and see if more than 1 surface was specified
  int o_ntri, ntri, a, b;
  char buff[bufsize];
  // read first surface
  if( gzgets(in,buff,bufsize)!=Z_NULL && 
                 sscanf( buff, "%d %d %d", &o_ntri, &a, &b ) < 3 )
	for( int i=0; i<o_ntri; i++ )
      gzgets( in, buff, bufsize );	
  // look for more surfaces
  while( gzgets(in,buff,bufsize)!=Z_NULL && 
		   sscanf( buff, "%d %d %d", &ntri, &a, &b ) < 3 ) {
	for( int i=0; i<ntri; i++ )
      gzgets( in, buff, bufsize );
	_surface[numSurf-1]->end(oldNumEle+o_ntri-1);
	_surface=(Surfaces **)realloc(_surface,++numSurf*sizeof(Surfaces *));
	_surface[numSurf-1] = new Surfaces( (SurfaceElement**)(&_triele) );
	_surface[numSurf-1]->start(_surface[numSurf-2]->end()+1);
    _surface[numSurf-1]->end(_surface[numSurf-1]->start()+ntri-1);
	oldNumEle += o_ntri;
	o_ntri = ntri;
  }
  gzclose( in );
  
  for( int s=sn; s<numSurf; s++ )
	_surface[s]->determine_vert_norms( pt );

  return numSurf;
}


/** set if an object is shown in a region
 */
void Model :: showobj( Object_t obj, bool *r, bool f )
{
  for( int i=0; i<_numReg; i++ )
	if( r[i] )
	  _region[i]->show( obj, f );
}


/*
 * return color of object for surface s
 */
GLfloat* Model:: get_color( Object_t obj, int s )
{
  return _region[s<0?0:s]->get_color(obj);
}


void Model:: set_color( Object_t obj, int s, float r, float g, float b, float a )
{
  if( obj==Surface) {
	if( s<0 )
	  for( int i=0; i<numSurf; i++ )
		surface(i)->fillcolor( r, g, b, a );
	else
	  surface(s)->fillcolor( r, g, b, a );
  } else if( obj==SurfEle) {
	if( s<0 )
	  for( int i=0; i<numSurf; i++ )
		surface(i)->outlinecolor( r, g, b, a );
	else
	  surface(s)->outlinecolor( r, g, b, a );
  } else {
	if( s<0 )
	  for( int i=0; i<_numReg; i++ )
		_region[i]->set_color( obj, r, g, b, a );
	else
	  _region[s]->set_color( obj, r, g, b, a );
  }
}

void Model :: visibility( int r, bool a )
{
  _region[r]->visible(a);
}

void Model :: opacity( int s, float opac )
{
  for( int i=s<0?0:s; i<(s<0?_numReg:s+1); i++ ){
	GLfloat *c = _region[i]->get_color(Cable);
	c[3] = opac;
	c = _region[i]->get_color(Vertex);
	c[3] = opac;
	c = _region[i]->get_color(Cnnx);
	c[3] = opac;
  }
}

void Model :: randomize_color( Object_t obj )
{
  const double min_brightness=1.5;		// minimum brighness for colour
  long maxl = ~0;
  int num;
  if( obj==Surface || obj==SurfEle )
	num = numSurf;
  else
	num = _numReg;
  
  for( int i=0; i<num; i++ ) {
	GLfloat *c;
    if( obj==Surface )
	  c = surface(i)->fillcolor();
	else if( obj==SurfEle )
	  c = surface(i)->outlinecolor();
	else
	  c = _region[i]->get_color( obj );
	c[0] = double(random())/double(maxl);
	c[1] = double(random())/double(maxl);
	c[2] = double(random())/double(maxl);
	float sum = c[0] + c[1] + c[2];
	if( sum < min_brightness ) {		// ensure the surface is bright enough 
	  c[0] *= min_brightness/sum;	
	  c[1] *= min_brightness/sum;	
	  c[2] *= min_brightness/sum;	
	}
  }
}



Model::~Model()
{
  delete[] _pts;
  delete[] _cnnx;
  //delete[] _ptnrml;
  for( int i=0; i<_numVol; i++ )
	delete _vol[i];
  delete _vol;
  delete _cable;
  for( int i=0; i<numSurf; i++ )
	delete _surface[i];
  delete _triele;
  free( _surface );
}



#ifndef OBJ_CLASS
void Model :: draw_triele( int ele, DataColouring datcol, GLfloat opaque,
								DATA_TYPE* data, bool lightson )
{
  int eind = 3*ele;

  if( !pt.vis(_element[eind]) || !pt.vis(_element[eind+1]) ||
										  !pt.vis(_element[eind+2]) )
	return;

  if( lightson && _elemNrml[ele] ) 
	glNormal3fv( _nrml+eind );

  if( datcol == on ) {
	if( opaque>opaque_min ) {
	  _cs->colourize( data[_element[eind]] );
	  glVertex3fv( _pts+3*_element[eind] );
	  _cs->colourize( data[_element[eind+1]] );
	  glVertex3fv( _pts+3*_element[eind+1] );
	  _cs->colourize( data[_element[eind+2]] );
	  glVertex3fv( _pts+3*_element[eind+2] );
	} else {
	  _cs->colourize( data[_element[eind]], opaque );
	  glVertex3fv( _pts+3*_element[eind] );
	  _cs->colourize( data[_element[eind+1]], opaque);
	  glVertex3fv( _pts+3*_element[eind+1] );
	  _cs->colourize( data[_element[eind+2]], opaque);
	  glVertex3fv( _pts+3*_element[eind+2] );
	}
  } else if( datcol == off ) {
	glVertex3fv( _pts+3*_element[eind] );
	glVertex3fv( _pts+3*_element[eind+1] );
	glVertex3fv( _pts+3*_element[eind+2] );
  } else if( datcol == opacval ) {
	_cs->colourize( data[_element[eind]], 
			_dataopac->do[Surface].alpha(data[_element[eind]]) );
	glVertex3fv( _pts+3*_element[eind] );
	_cs->colourize( data[_element[eind+1]], 
			_dataopac->do[Surface].alpha(data[_element[eind+1]]) );
	glVertex3fv( _pts+3*_element[eind+1] );
	_cs->colourize( data[_element[eind+2]], 
			_dataopac->do[Surface].alpha(data[_element[eind+2]]) );
	glVertex3fv( _pts+3*_element[eind+2] );
  }
}
#endif


/** output highlight information
 *
 *  \param hinfo   window in which to output text
 *  \param hilight list of highlighted objects
 */
void Model::hilight_info( HiLiteInfoWin* hinfo, int* hilight, DATA_TYPE* data )
{
  hinfo->clear();
  char* txt  = new char[256];
  char* ts   = NULL;

  // Volume ELements
  if( _numVol ){
	int hivol=hilight[Tetrahedron];
	sprintf( txt, "@b@C%6dVolume Element: %d of %d", FL_RED, hivol, _numVol );
	hinfo->add( txt );
	hinfo->add( "nodes:\t" );
	const int* tet=_vol[hivol]->obj();
	for( int i=0; i<_vol[hivol]->ptsPerObj(); i++ ) {
	  if( data != NULL )
		sprintf(txt, "%6d -> %f", tet[i], data[tet[i]] );
	  else
		sprintf( txt, "%6d", tet[i] );
	  if( tet[i]==hilight[Vertex] ) 
		sprintf( txt,"@B%d%s", FL_GRAY, ts=strdup(txt) );
	  hinfo->add( txt );
	}
  }
  hinfo->add("");
  if( ts != NULL ){
	free(ts);
	ts=NULL;
  }

// SurfEles
if( _triele->num() ) {
  sprintf(txt, "@b@C%6dTriangle: %d of %d", FL_BLUE, hilight[SurfEle],
					_triele->num());
	hinfo->add( txt );
	hinfo->add( "nodes:\t" );
	int elenode = hilight[SurfEle]*3;
	for( int i=0; i<3; i++ ) {
	  int node=_triele->obj(hilight[SurfEle])[i];
	  if( data != NULL )
		sprintf( txt,"\t%6d -> %f", node, data[node] );
	  else
		sprintf( txt, "\t%6d", node );
	  if( node==hilight[Vertex] ) 
		sprintf( txt,"@B%d%s", FL_GRAY, ts=strdup(txt) );
	  hinfo->add( txt );
	}
	if( ts != NULL ){
	  free(ts);
	  ts=NULL;
	}
	//normal
	const GLfloat* n=_triele->nrml(hilight[SurfEle]); 
	if( n != NULL ) {
	  hinfo->add( "normal:\t" );
	  sprintf( txt, "(%f, %f, %f)", n[0], n[1], n[2] );
	  hinfo->add( txt );
	}
	for( int s=0; s,numSurf; s++ ) {
	  if( hilight[SurfEle]>=surface(s)->start() &&
	      hilight[SurfEle]<=surface(s)->end()     ){
	    sprintf( txt, "in surface %6d", s );
	    hinfo->add( txt );
	    break;
	  }
	}
	hinfo->add( "" );
  }
  // Vertex info
  sprintf(txt, "@b@C%6dVertex: %d of %d", FL_GREEN, hilight[Vertex],
		  pt.num() );
  hinfo->add( txt );
  if( data != NULL ) {
	sprintf( txt, "value: %f", data[hilight[Vertex]] );
  	hinfo->add( txt );
  }
  const GLfloat*offset = pt.offset();
  sprintf( txt, "( %.6g, %.6g, %.6g )", _pts[hilight[Vertex]*3]+offset[0],
		  _pts[hilight[Vertex]*3+1]+offset[1],
		  					_pts[hilight[Vertex]*3+2]+offset[2]);
  hinfo->add( txt );
  /*
  string reginfo( "in surface: " );
  for( int s=0; s<_numReg; s++ )
	if( hilight[Vertex]>=_surface[s]->start(Vertex) && 
			hilight[Vertex]<=_surface[s]->end(Vertex)		){
	  
	  break;
	}
  hinfo->add( txt );
  */
  if( _cable->num() ) {
	const int* cab=_cable->obj();
	for( int cabnum=0; cabnum<_cable->num();cabnum++ )
	  if(cab[cabnum]<=hilight[Vertex] && cab[cabnum+1]>=hilight[Vertex]){
		sprintf(txt, "in cable: %d", cabnum );
		hinfo->add( txt );
		break;
	  }
  }
  if( _cnnx->num() ) {
	int cnum;
	for( cnum=0; cnum<_cnnx->num();cnum++ ) {
	  const int* conn = _cnnx->obj(cnum);
	  if( conn[0]==hilight[Vertex] || conn[1]==hilight[Vertex] ){
		sprintf(txt, "in connection: %d", cnum );
		hinfo->add( txt );
		break;
	  }
	}
  }

  set<int> att_nodes;	// list nodes in attached tri's and tet's

  if( _cable->num() ) {
	sprintf( txt, "Attached cables:" );
	hinfo->add( txt );
	const int* cab = _cable->obj();
	for( int i=0; i<_cable->num(); i++ )
	  if( cab[i]<=hilight[Vertex] && cab[i+1]>hilight[Vertex]){
		sprintf( txt, "\t%6d", i );
		if( i==hilight[Cable] ) 
		  sprintf( txt,"@B%d%s", FL_GRAY, ts=strdup(txt) );
		hinfo->add( txt );
	  }
	if( ts != NULL ){
	  free(ts);
	  ts=NULL;
	}
  }
  if( _triele->num() ) {
	sprintf( txt, "Attached triangles:" );
	hinfo->add( txt );
	const int* ele=_triele->obj();
	for( int i=0; i<3*_triele->num(); i++ )
	  if( ele[i]==hilight[Vertex] ){
		sprintf( txt, "\t%6d", i/3 );
		if( i/3==hilight[SurfEle] ) 
		  sprintf( txt,"@B%d%s", FL_GRAY, ts=strdup(txt) );
		hinfo->add( txt );
		for( int j=3*(i/3); j<3*(i/3)+3; j++ )
		  if( ele[j]!=hilight[Vertex] ) att_nodes.insert(ele[j]);
	  }
	if( ts != NULL ){
	  free(ts);
	  ts=NULL;
	}
  }
  if( _numVol ) {
	sprintf( txt, "Attached volume elements:" );
	hinfo->add( txt );
	for( int i=0; i<_numVol; i++ ) {
	  const int* tet=_vol[i]->obj();
	  for( int j=0; j<_vol[i]->ptsPerObj(); j++ )
	    if( tet[j]==hilight[Vertex] ) {
		  sprintf( txt, "\t%d", i );
		if( tet[j]==hilight[Tetrahedron] ) 
		  sprintf( txt,"@B%d%s", FL_GRAY, ts=strdup(txt) );
		hinfo->add( txt );
		for( int k=0; k<_vol[i]->ptsPerObj(); k++ ){
		  if( tet[k]!=hilight[Vertex] ) att_nodes.insert(tet[k]);
		}
	  }	  
	}
	if( ts != NULL ){
	  free(ts);
	  ts=NULL;
	}
  }
  sprintf( txt, "Attached nodes:" );
  hinfo->add( txt );
  for( set<int>::iterator p=att_nodes.begin(); p!=att_nodes.end(); p++ ){
	sprintf( txt, "\t%d", *p );
	hinfo->add( txt );
  }
  hinfo->add( "" );
  //Cables
  if( _cable->num() ) {
	sprintf(txt, "@b@C%6dCable: %d of %d", FL_CYAN, hilight[Cable],
					_cable->num() );
	hinfo->add( txt );
	hinfo->add( "nodes:" );
	sprintf( txt, "\t%6d", *(_cable->obj(hilight[Cable])) );
	if( _cable->obj(hilight[Cable])[0]==hilight[Vertex] ) 
	  sprintf( txt,"@B%d%s", FL_GRAY, ts=strdup(txt) );
	hinfo->add( txt );
	sprintf( txt, "\t%6d", *(_cable->obj(hilight[Cable]+1))-1 );
	if( _cable->obj(hilight[Cable]+1)[0]-1==hilight[Vertex] ) 
	  sprintf( txt,"@B%d%s", FL_GRAY, ts=strdup(txt) );
	hinfo->add( txt );
	hinfo->add( "" );
	if( ts != NULL ){
	  free(ts);
	  ts=NULL;
	}
  }
  //Connections
  if( _cnnx->num() ) {
	sprintf(txt, "@b@C%6dConnection: %d of %d", FL_MAGENTA, hilight[Cnnx],
							_cnnx->num() );
	hinfo->add( txt );
	hinfo->add( "nodes:\t" );
	sprintf( txt, "\t%6d", _cnnx->obj(hilight[Cnnx])[0] ); 
	if( _cnnx->obj(hilight[Cnnx])[0] ==hilight[Vertex] ) 
	  sprintf( txt,"@B%d%s", FL_GRAY, ts=strdup(txt) );
	hinfo->add( txt );
	sprintf( txt, "\t%6d", _cnnx->obj(hilight[Cnnx])[1] ); 
	if( _cnnx->obj(hilight[Cnnx])[1]==hilight[Vertex] ) 
	  sprintf( txt,"@B%d%s", FL_GRAY, ts=strdup(txt) );
	hinfo->add( txt );
	hinfo->add("");
	if( ts != NULL ){
	  free(ts);
	  ts=NULL;
	}
  }
  hinfo->window->show();
}


/** try reading in optional regions file
 *
 * \param in
 * \param fnb base file name endinging in "."
 */
void Model::read_region_file( gzFile in, const char *fnb )
{
  char fn[1024];
  int  numNewReg;

  strcat( strcpy( fn, fnb ), "region" );
  if( (in=gzopen(fn, "r" )) != NULL || (in=gzopen(strcat(fn,".gz"),"r")) != NULL ) {
    char buff[bufsize];
	gzgets(in, buff, bufsize);
	sscanf( buff, "%d", &numNewReg );
	_numReg += numNewReg;
	_region = (Region **)realloc( _region, _numReg*sizeof(Region*) );
	for( int i=_numReg-numNewReg; i<_numReg; i++ ) {
	  int firstpt, lastpt, label;
	  gzgets(in, buff, bufsize);
	  sscanf( buff, "%d %d %d", &firstpt, &lastpt, &label );
	  _region[i] = new Region( pt.num(), label, false );
	  for( int e=firstpt; e<=lastpt; e++ )
		_region[i]->member(e,true);
	}
	gzclose(in);
  }
}


int Model::number(Object_t a )
{
  switch( a ) {
	  case Vertex:
		  return pt.num();
	  case Cnnx:
		  return _cnnx->num();
	  case Cable:
		  return _cable->num();
	  case Surface:
		  return numSurf;
	  case SurfEle:
		  return _triele->num();
	  case Tetrahedron:
		  return _numVol;
	  case RegionDef:
		  return _numReg;
  }
}


/** set vertex normals for a surface
 *
 * \param s surface \#
 */
const GLfloat* 
Model::vertex_normals(int s){
  
  if( s<0 || s>=numSurf )
	return NULL;

  surface(s)->get_vert_norms( _vertnrml );

  return _vertnrml;
}


/** read in an element file
 *
 * \param fn file name base
 */
bool Model :: read_elem_file( const char *fname )
{
  gzFile in;
  bool tets = false;
  char eletype[3] = "Tt";
  const int bufsize=1024;
  char buf[bufsize];
  
  try {
	in   = openFile( fname, "tetras" );
	tets = true;
  }
  catch(...) {
	try {
	  in = openFile( fname, "elem" );
	}
	catch(...) {return false;}
  }

  if( gzgets(in, buf, bufsize) == Z_NULL ) return false;
  sscanf( buf, "%d", &_numVol );
  
  _vol = new VolElement*[_numVol];

  for( int i=0; i< _numVol; i++ ) {
    if( gzgets(in, buf, bufsize) == Z_NULL ) return false;
	int n[9];
	if( tets ) 
	  sscanf( buf, "%d %d %d %d %d %d %d %d %d", n, n+1, n+2, n+3, n+4 );
	else
	  sscanf( buf, "%2s %d %d %d %d %d %d %d %d %d", eletype,
			n, n+1, n+2, n+3, n+4, n+5, n+6, n+7, n+8 );
    if( !strcmp( eletype, "Tt" ) ) {
	  _vol[i] = new Tetrahedral( &pt );
	  _vol[i]->add( n, n[4] );
	} else if( !strcmp( eletype, "Hx" ) ) {
	  _vol[i] = new Hexahedron( &pt );
	  _vol[i]->add( n, n[8] );
	} else if( !strcmp( eletype, "Py" ) ) {
	  _vol[i] = new Pyramid( &pt );
	  _vol[i]->add( n, n[5] );
	} else if( !strcmp( eletype, "Pr" ) ) {
	  _vol[i] = new Prism( &pt );
	  _vol[i]->add( n, n[6] );
	} else {
	  fprintf( stderr, "Unsupported element type\n" );
	  exit(1);
	}
  }
}
