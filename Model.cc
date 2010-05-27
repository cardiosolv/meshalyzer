#include "Model.h"
#include <set>
#include <map>
#include <vector>
#include <string>
#include "DataOpacity.h"
#include "gl2ps.h"
#include "VecData.h"
#include <sstream>
#include <FL/filename.H>
#include "gzFileBuffer.h"
#include "logger.hpp"

struct Face {
  int nsort[MAX_NUM_SURF_NODES];  //!< sorted nodes
  int norig[MAX_NUM_SURF_NODES];  //!< nodes in original order
  int nnode;                      //!< number of nodes
};

int intcmp( const void *a, const void *b ){ return *(int *)a-*(int *)b; }

/** compare 2 faces for sorting */
bool cmpface( const Face *A, const Face *B)
{
  if( A->nnode != B->nnode ) return  A->nnode < B->nnode;
  for(  int i=0; i< A->nnode; i++ )
    if( A->nsort[i] != B->nsort[i] ) return  A->nsort[i] < B->nsort[i];
  return false;
};


/** make a face from a node list
 *
 * \param f new face
 * \param n number of nodes in surface face
 * \param orig nodes in original order
 */
void
make_face( Face *f, int n, int* orig )
{
  f->nnode = n;
  memcpy( f->norig, orig, n*sizeof(int) );
  memcpy( f->nsort, orig, n*sizeof(int) );
  qsort( f->nsort, n, sizeof(int), intcmp ); 
}


/** read in a line from a file, ignoring lines beginning with "#"
 *
 * \warn (GD) lines are limited to 1024 bytes.
 * \warn not threadsafe
 * \return pointer to a static buffer
 */
char *
get_line( gzFile in )
{
  const int bufsize=1024;
  static char buf[bufsize];
  char *retval;
  do {
	retval = gzgets( in, buf, bufsize );
  } while( retval != Z_NULL && buf[0]=='#' );
  
  return retval==Z_NULL?NULL:buf;
}


/** return a new unique region label
 */
int Model::new_region_label()
{
  int  r, newlabel=0; 
  while ( true ) {
    for ( r=0; r<_numReg; r++ )
      if ( _region[r]->label() == newlabel )
        break;
    if ( r==_numReg )
      break;
    newlabel++;
  }
  return newlabel;
}


Model::Model():
    _base1(false), _surface(NULL), _vertnrml(NULL),
    _numReg(0), _region(NULL), _numVol(0), _vol(NULL), _cnnx(NULL) 
{
  for ( int i=0; i<maxobject; i++ ) {
    _outstride[i] = 1;
  }
}


/** read in the geometrical description
 *
 * \param flwindow
 * \param fnt      base file to open
 * \param base1    points begin numbering at 0
 * \param no_elems do not read element file
 */
const int bufsize=1024;
bool Model::read( const char* fnt, bool base1, bool no_elems )
{
  LOG_TIMER_RESET;

  char fn[bufsize];
  strcpy( fn, fnt );
  gzFile in;
  _base1 = base1;
  pt.base1( base1 );

  try {
    pt.read( fn );
  } catch (...) {
    fprintf(stderr, "Unable to read proper points file\n" );
    exit(1);
  }
  allvis.resize(pt.num());
  allvis.assign(pt.num(), true );
  LOG_TIMER("pt.read()");

  if ( !strcmp( ".gz", fn+strlen(fn)-3 ) ) // temporary
    fn[strlen(fn)-3] = '\0';
  if ( !strcmp( ".pts", fn+strlen(fn)-4 ) ) // temporary
    fn[strlen(fn)-3] = '\0';
  if ( fn[strlen(fn)-1] != '.' ) {
    fn[strlen(fn)+1] ='\0';
    fn[strlen(fn)] ='.';
  }
  _file = fn;


  _cnnx   = new Connection( &pt );
  _cnnx->read( fn );
  LOG_TIMER("cnnx->read()");
  
  _cable  = new ContCable( &pt );
  _cable->read( fn );
  LOG_TIMER("cable->read()");

  if( !no_elems ) read_elem_file( fn );
  LOG_TIMER("read_elem_file()");

  read_region_file( in, fn );
  LOG_TIMER("read_region_file()");

  determine_regions();
  LOG_TIMER("determine_regions()");

  add_surface_from_tri( fn );
  LOG_TIMER("add_surface_from_tri()");

  add_surface_from_surf( fn );
  LOG_TIMER("add_surface_from_surf()");

  add_surface_from_elem( fn );
  LOG_TIMER("add_surface_from_elem()");

  // find maximum dimension and bounding box
  const GLfloat *p = pt.pt();
  _maxdim =p[0];
  for ( int i=1; i<pt.num()*3; i++ )
    if ( p[i]>_maxdim ) _maxdim = p[i];

  _vertnrml= new GLfloat[3*pt.num()];

  return true;
}


/** determine which points are part of all the regions
 *
 * \pre all objects (vertices, cables, etc.) have been read in
 */
void Model::determine_regions()
{
  if (!_numVol && !_numReg  ) {
    _region = (RRegion **)calloc( 1, sizeof(RRegion *) );
    _region[0] = new RRegion( pt.num() ); //0 ==default region label
    _numReg = 1;
    return;
  }

  if ( _numVol ) {
    _region = (RRegion **)realloc( _region, (_numReg+1)*sizeof(RRegion *) );
    _region[_numReg] = new RRegion(_vol, _numVol, pt.num(), _vol[0]->region(0));
    _numReg++;
    for ( int i=1; i<_numVol; i++ ) {
      int r;
      for ( r=0; r<_numReg; r++ )
        if ( _region[r]->label() == _vol[i]->region(0) )
          break;
      if ( r==_numReg ) {
        _numReg++;
        _region = (RRegion**)realloc( _region, sizeof(RRegion)*_numReg );
        _region[_numReg-1]=new RRegion(_vol,_numVol,pt.num(),_vol[i]->region(0));
      }
    }
  }

  // make sure all elements are included
  vector<bool> hasEleRegion(_numVol,false);
  for ( int r=0; r<_numReg; r++ )
    for ( int i=0; i<_numVol; i++ )
      hasEleRegion[i] = hasEleRegion[i] || _region[r]->ele_member(i);
  for ( int i=0; i<_numVol; i++ )
    if ( !hasEleRegion[i] ) { // point without a region detected
      _region = (RRegion**)realloc( _region, sizeof(RRegion)*(_numReg+1) );
      _region[_numReg] = new RRegion( 
                             _vol, _numVol, pt.num(), new_region_label() );
      _numReg++;
      for ( int i=0; i<_numVol; i++ )
        if ( !hasEleRegion[i] ) _region[_numReg-1]->ele_member( _vol, i, true );
     break;
    }

  // make sure that all points are included
  vector<bool> hasPtRegion(pt.num(),false);
  for ( int r=0; r<_numReg; r++ )
    for ( int i=0; i<pt.num(); i++ )
      hasPtRegion[i] = hasPtRegion[i] || _region[r]->pt_member(i);

  for ( int i=0; i<pt.num(); i++ )
    if ( !hasPtRegion[i] ) { // point without a region detected
      _region = (RRegion**)realloc( _region, sizeof(RRegion)*(_numReg+1) );
      _region[_numReg] = new RRegion( pt.num(), _numVol, 
                                            new_region_label(), false );
      _numReg++;
      for ( int i=0; i<pt.num(); i++ )
        if ( !hasPtRegion[i] ) _region[_numReg-1]->pt_member( i, true );
      break;
    }


#ifdef _ARCH_PPC
  heapsort( _region, _numReg, sizeof(RRegion *), RRegion_sort );
#else
  qsort( _region, _numReg, sizeof(RRegion *), RRegion_sort );
#endif

  // find first cable in layer
  if ( _cable != NULL ) {
    for ( int r=0; r<_numReg; r++ ) {
      for ( int i=0; i<_cable->num(); i++ ) {
        const int *o = _cable->obj(i);
        int  c;
        for ( c=o[0]; c<o[1]; c++ )
          if ( _region[i]->pt_member(c) == true ) {
            _region[r]->first(Cable,i);
            break;
          }
        if ( c<o[1] )
          break;
      }
    }
  }

  // find first connection in layer
  if ( _cnnx != NULL ) {
    for ( int r=0; r<_numReg; r++ ) {
      for ( int i=0; i<_cnnx->num(); i++ ) {
        const int *o = _cnnx->obj(i);
        if ( _region[r]->pt_member(o[0])==true||_region[r]->pt_member(o[1])==true ) {
          _region[r]->first(Cnnx,i);
          break;
        }
      }
    }
  }
}


/** add surface by selecting 2D elements from .elem file
 *
 *  A surface will be created for each different region specified.
 *  Also, all elements withoyut a region specified will form a surface.
 *
 *  \param base model name
 */
int Model::add_surface_from_elem( const char *fn )
{
  /* determine file name */
  gzFile in;
  string fname(fn);
  if ( (in=gzopen( (fname+="elem").c_str(), "r" )) == NULL )
    if ((in=gzopen( (fname+=".gz").c_str(), "r" )) == NULL ) {
      return -1;
    }

  // Count the number of surface elements in each surface
  char buff[bufsize];
  gzgets(in,buff,bufsize);           // throw away first line
  map<string,int> surfs;             // number of elements in each surface

  gzFileBuffer file(in);
  while ( file.gets(buff,bufsize) != Z_NULL ) {
    char   surfnum[10]="";
    if ( !strncmp(buff,"Tr",2) ) {
      if ( sscanf(buff, "%*s %*d %*d %*d %s", surfnum )<1 )
        strcpy( surfnum, "EMPTY" );
    } else if ( !strncmp(buff,"Qd",2) ) {
      if ( sscanf(buff, "%*s %*d %*d %*d %*d %s", &surfnum )<1 )
        strcpy( surfnum, "EMPTY" );
    }
    if ( strlen(surfnum) )
      surfs[surfnum]++;
  }
  if ( !surfs.size() ) return numSurf();

  // allocate new surfaces
  int oldnumSurf = _surface.size();
  for ( int s=oldnumSurf; s<oldnumSurf+surfs.size(); s++ )
    _surface.push_back( new Surfaces( &pt ) );
  map<string,int> surfmap;
  map<string,int> :: iterator iter = surfs.begin();
  for ( int s=oldnumSurf; iter!=surfs.end(); iter++, s++ ) {
    _surface[s]->num(iter->second );
    surfmap[iter->first] = s;   // map region to surface index
  }

  surfs.clear();                // now use for current element in each surface
  gzrewind(in);

  gzFileBuffer file2(in);

  gzgets(in,buff,bufsize);      //throw away first line
  while( file2.gets(buff,bufsize) !=Z_NULL ) {
	char etype[10],reg[10];
	int  idat[4];
	if( !strncmp(buff,"Tr",2) ) {
	  if(sscanf( buff,"%s %d %d %d %s", etype, idat, idat+2, idat+1, reg)<5 )
		strcpy(reg,"EMPTY");
	  _surface[surfmap[reg]]->ele(surfs[reg]) = new Triangle( &pt );
	} else  if( !strncmp(buff,"Qd",2) ) {
	  if(sscanf( buff,"%s %d %d %d %d %s", etype, idat, idat+3, idat+2, 
				  idat+1, reg)<6 )
		strcpy(reg,"EMPTY");
	  _surface[surfmap[reg]]->ele(surfs[reg]) = new Quadrilateral( &pt );
	} else
	  continue;  //ignore volume elements
	_surface[surfmap[reg]]->ele(surfs[reg])->define(idat);
	_surface[surfmap[reg]]->ele(surfs[reg])->compute_normals(0,0);
	surfs[reg]++;
  }
  gzclose( in );

  for ( int s=oldnumSurf; s<_surface.size(); s++ ) {
    _surface[s]->determine_vert_norms( pt );
    ostringstream slabel(fname);
    slabel << s-oldnumSurf;
    _surface[s]->label( slabel.str() );
  }

  return _surface.size();
}


/** find bounding surfaces for all the regions
 *
 *  We do this by adding the faces of all elements into a tree and removing a face if
 *  it appears twice
 */
int Model::add_region_surfaces()
{
  // set up matrix to hold element faces
  int **faces = new int *[MAX_NUM_SURF];
  for( int i=0; i<MAX_NUM_SURF; i++ ) faces[i] = new int[MAX_NUM_SURF_NODES+1];
  int numNewSurf=0;

  for( int r=0; r<_numReg; r++ )  {
    
    set<Face*, bool (*)(const Face*, const Face*)> facetree(cmpface);
    Face *newface = new Face;
    
    for( int e=0; e<_numVol; e++ )
      if( _region[r]->ele_member(e) ) {
        int ns = _vol[e]->surfaces( faces );
        for( int i=0; i<ns; i++ ) {
          make_face( newface, faces[i][0], faces[i]+1 );
          set<Face*>::iterator iter = facetree.find(newface);
          if (iter != facetree.end()) {
            delete *iter;
            facetree.erase(iter);
          } else {
            facetree.insert(newface);
            newface = new Face;  // added to list, get memory for a new face
          }
        }
      }

    // count the number of faces left in the list
    delete newface;

    if( facetree.size() ) {             // convert the left over faces into a surface
      numNewSurf++;
      
      _surface.push_back( new Surfaces( &pt ) );
      _surface.back()->num( facetree.size() );

      int e=0;
      for(set<Face*>::iterator iter=facetree.begin();
          iter!=facetree.end();
          ++iter) {

        newface = *iter;

        if( newface->nnode == 3 ) {
          _surface.back()->ele(e) = new Triangle( &pt );
        } else if( newface->nnode==4 ) {
          _surface.back()->ele(e) = new Quadrilateral( &pt );
        } else
          assert(0);

        _surface.back()->ele(e)->define( newface->norig );
        _surface.back()->ele(e)->compute_normals(0,0);

        delete newface; // no longer needed
        e++;
      }
      _surface.back()->determine_vert_norms( pt );
      ostringstream regnum;
      regnum << "Reg " << r;
      _surface.back()->label( regnum.str() );
    }
  }
  for( int i=0; i<MAX_NUM_SURF; i++ ) delete[] faces[i];
  delete[] faces;
  return numNewSurf;
}


/** add a surface by reading in a .surf file
 *
 * \param fn   file containing surface elements
 *
 * \return     the \#surfaces added, -1 if not successful
 */
int Model::add_surface_from_surf( const char *fn )
{
  /* determine file name */
  gzFile in;
  string fname(fn);
  if ( (in=gzopen( fname.c_str(), "r" )) == NULL ) {
    fname += "surf";
    if ( (in=gzopen( fname.c_str(), "r" )) == NULL ) {
        fname += ".gz";
        if ( (in=gzopen( fname.c_str(), "r" )) == NULL ) {
            return -1;
        }
    }
  }

  int  nd[5], nele;
  char buff[bufsize];
  int  surfnum = 0;
  char surflabel[bufsize];

  gzFileBuffer file(in);
  while ( file.gets(buff,bufsize)!=Z_NULL && sscanf(buff, "%d", &nele )==1 ) {
    _surface.push_back( new Surfaces( &pt ) );

    // use specified surface label if available
    if( sscanf( buff, "%d %s", &nele, surflabel ) != 2 ) 
      sprintf( surflabel, "%s:%d", fl_filename_name(fname.c_str()), surfnum );
    _surface.back()->label( surflabel );

    _surface.back()->num(nele);
    for ( int i=0; i<nele; i++ ) {
      int nl[5];
      char etype[12];
      if ( file.gets(buff,bufsize) == Z_NULL ||
              sscanf(buff, "%s %d %d %d %d %d",
                             etype, nl, nl+1, nl+2, nl+3, nl+4 ) < 4 ) {
        _surface.pop_back();
        return surfnum;
      }
      if( !strcmp(etype, "Tr" ) )
        _surface.back()->ele(i) = new Triangle( &pt );
      else if( !strcmp(etype, "Qd") )
        _surface.back()->ele(i) = new Quadrilateral( &pt );
      else {
        _surface.pop_back();
        return surfnum;
      }
      _surface.back()->ele(i)->define(nl);
      _surface.back()->ele(i)->compute_normals(0,0);
    }
    _surface.back()->determine_vert_norms( pt );
    surfnum++;
  } 

  return surfnum;
}


/** add a surface by reading in a .tri file, also try reading a normal file
 *
 * \param fn   file containing tri's
 *
 * \return     the \#surfaces, -1 if not successful
 */
int Model::add_surface_from_tri( const char *fn )
{
  /* determine file name */
  gzFile in;
  string fname(fn);
  if ( (in=gzopen( fname.c_str(), "r" )) == NULL ) {
    fname += "tri";
    if ( (in=gzopen( fname.c_str(), "r" )) == NULL ) {
      fname += "s";
      if ( (in=gzopen( fname.c_str(), "r" )) == NULL ) {
        fname += ".gz";
        if ( (in=gzopen( fname.c_str(), "r" )) == NULL ) {
          fname = fn;
          fname += "tri.gz";
          if ( (in=gzopen( fname.c_str(), "r" )) == NULL ) {
            //fprintf( stderr, "Could not open surface file\n" );
            return -1;
          }
        }
      }
    }
  }

  int  nd[3], ntri;
  char buff[bufsize];
  gzgets(in,buff,bufsize);
  bool multi_surface = sscanf( buff, "%d %d %d", &ntri, nd+1, nd+2 ) < 3;
  int surfnum = 0;

  if ( multi_surface ) {

    do {
      _surface.push_back( new Surfaces( &pt ) );

      // use specified surface label if available
      char surflabel[1024];
      if( sscanf( buff, "%d %s", &ntri, surflabel ) != 2 ) 
        sprintf( surflabel, "%s:%d", fl_filename_name(fname.c_str()), surfnum );
      _surface.back()->label( surflabel );

      _surface.back()->num(ntri);
      for ( int i=0; i<ntri; i++ ) {
        _surface.back()->ele(i) = new Triangle( &pt );
        int nl[3];
        if ( gzgets(in,buff,bufsize) == Z_NULL ||
             sscanf(buff, "%d %d %d", nl, nl+1, nl+2 ) < 3 ) {
          _surface.pop_back();
          return surfnum;
        }
        _surface.back()->ele(i)->define(nl);
        _surface.back()->ele(i)->compute_normals(0,0);
      }
      _surface.back()->determine_vert_norms( pt );
      surfnum++;
    } while ( gzgets(in,buff,bufsize)!=Z_NULL && sscanf(buff, "%d",&ntri)==1 );

  } else {  // 1 surface
    
    int nl[3];
    nl[0]=ntri;nl[1]=nd[1];nl[2]=nd[2];
    _surface.push_back( new Surfaces( &pt ) );
    int curele = 0;
    do {
      if(  sscanf(buff, "%d %d %d",nl, nl+1, nl+2) < 3 ) {
        if( !curele ) {
        _surface.pop_back();
        return -1;
      } else
        break;
      }
#define ELEINC 10000
	  if( !(curele%ELEINC) ) _surface.back()->num(curele+ELEINC);
	  _surface.back()->ele(curele) = new Triangle( &pt );
  	  _surface.back()->ele(curele)->define(nl);
	  _surface.back()->ele(curele++)->compute_normals(0,0);
	}while( gzgets(in,buff,bufsize)!=Z_NULL );
    _surface.back()->num(curele);
    _surface.back()->determine_vert_norms( pt );
    ostringstream slabel(fname);
    _surface.back()->label( slabel.str() );
    surfnum++;
  }
  return surfnum;
}


/** set if an object is shown in a region
 */
void Model :: showobj( Object_t obj, bool *r, bool f )
{
  for ( int i=0; i<_numReg; i++ )
    if ( r[i] )
      _region[i]->show( obj, f );
}


/** return color of object for surface s
 *
 *  \param obj object type
 *  \param s   region number
 */
GLfloat* Model:: get_color( Object_t obj, int s )
{
  return _region[s<0?0:s]->get_color(obj);
}


void Model:: set_color( Object_t obj, int s, float r, float g, float b, float a )
{
  if ( obj==Surface) {
    if ( s<0 )
      for ( int i=0; i<numSurf(); i++ )
        _surface[i]->fillcolor( r, g, b, a );
    else
      _surface[s]->fillcolor( r, g, b, a );
  } else if ( obj==SurfEle) {
    if ( s<0 )
      for ( int i=0; i<numSurf(); i++ )
        _surface[i]->outlinecolor( r, g, b, a );
    else
      _surface[s]->outlinecolor( r, g, b, a );
  } else {
    if ( s<0 )
      for ( int i=0; i<_numReg; i++ )
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
  for ( int i=s<0?0:s; i<(s<0?_numReg:s+1); i++ ) {
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
  if ( obj==Surface || obj==SurfEle )
    num = numSurf();
  else
    num = _numReg;

  for ( int i=0; i<num; i++ ) {
    GLfloat *c;
    if ( obj==Surface )
      c = surface(i)->fillcolor();
    else if ( obj==SurfEle )
      c = surface(i)->outlinecolor();
    else
      c = _region[i]->get_color( obj );
    c[0] = double(random())/double(maxl);
    c[1] = double(random())/double(maxl);
    c[2] = double(random())/double(maxl);
    float sum = c[0] + c[1] + c[2];
    if ( sum < min_brightness ) {		// ensure the surface is bright enough
      c[0] *= min_brightness/sum;
      c[1] *= min_brightness/sum;
      c[2] *= min_brightness/sum;
    }
  }
}



Model::~Model()
{
  delete    pt.pt();
  delete[] _cnnx;
  //delete[] _ptnrml;
  for ( int i=0; i<_numVol; i++ )
    delete _vol[i];
  delete _vol;
  delete _cable;
  for ( int i=0; i<numSurf(); i++ )
    delete _surface[i];
  _surface.clear();
}


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
  if ( _numVol ) {
    int hivol=hilight[VolEle];
    sprintf( txt, "@b@C%6dVolume Element: %d of %d", FL_RED, hivol, _numVol );
    hinfo->add( txt );
    hinfo->add( "nodes:\t" );
    const int* tet=_vol[hivol]->obj();
    for ( int i=0; i<_vol[hivol]->ptsPerObj(); i++ ) {
      if ( data != NULL )
        sprintf(txt, "%6d -> %f", tet[i], data[tet[i]] );
      else
        sprintf( txt, "%6d", tet[i] );
      if ( tet[i]==hilight[Vertex] )
        sprintf( txt,"@B%d%s", FL_GRAY, ts=strdup(txt) );
      hinfo->add( txt );
    }
  }
  hinfo->add("");
  if ( ts != NULL ) {
    free(ts);
    ts=NULL;
  }

  // SurfEles
  int elesurf, lele;
  if ( numSurf() ) {
    sprintf(txt, "@b@C%6dSurface Element: %d of %d", FL_BLUE, hilight[SurfEle],
            number(SurfEle) );
    hinfo->add( txt );
    hinfo->add( "nodes:\t" );
    lele = localElemnum( hilight[SurfEle], elesurf );
    for ( int i=0; i<surface(elesurf)->ele(lele)->ptsPerObj(); i++ ) {
      int node=surface(elesurf)->ele(lele)->obj()[i];
      if ( data != NULL )
        sprintf( txt,"\t%6d -> %f", node, data[node] );
      else
        sprintf( txt, "\t%6d", node );
      if ( node==hilight[Vertex] )
        sprintf( txt,"@B%d%s", FL_GRAY, ts=strdup(txt) );
      hinfo->add( txt );
    }
    if ( ts != NULL ) {
      free(ts);
      ts=NULL;
    }
    //normal
    const GLfloat* n=surface(elesurf)->ele(lele)->nrml();
    if ( n != NULL ) {
      hinfo->add( "normal:\t" );
      sprintf( txt, "(%f, %f, %f)", n[0], n[1], n[2] );
      hinfo->add( txt );
    }
    sprintf( txt, "in surface %6d", elesurf );
    hinfo->add( txt );
    hinfo->add( "" );
  }

  // Vertex info
  sprintf(txt, "@b@C%6dVertex: %d of %d", FL_GREEN, hilight[Vertex],
          pt.num() );
  hinfo->add( txt );
  if ( data != NULL ) {
    sprintf( txt, "value: %f", data[hilight[Vertex]] );
    hinfo->add( txt );
  }
  const GLfloat*offset = pt.offset();
  sprintf( txt, "( %.6g, %.6g, %.6g )", pt.pt()[hilight[Vertex]*3]+offset[0],
           pt.pt()[hilight[Vertex]*3+1]+offset[1],
           pt.pt()[hilight[Vertex]*3+2]+offset[2]);
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
  if ( _cable->num() ) {
    const int* cab=_cable->obj();
    for ( int cabnum=0; cabnum<_cable->num();cabnum++ )
      if (cab[cabnum]<=hilight[Vertex] && cab[cabnum+1]>=hilight[Vertex]) {
        sprintf(txt, "in cable: %d", cabnum );
        hinfo->add( txt );
        break;
      }
  }
  if ( _cnnx->num() ) {
    int cnum;
    for ( cnum=0; cnum<_cnnx->num();cnum++ ) {
      const int* conn = _cnnx->obj(cnum);
      if ( conn[0]==hilight[Vertex] || conn[1]==hilight[Vertex] ) {
        sprintf(txt, "in connection: %d", cnum );
        hinfo->add( txt );
        break;
      }
    }
  }

  set<int> att_nodes;	// list nodes in attached tri's and tet's

  if ( _cable->num() ) {
    sprintf( txt, "Attached cables:" );
    hinfo->add( txt );
    const int* cab = _cable->obj();
    for ( int i=0; i<_cable->num(); i++ )
      if ( cab[i]<=hilight[Vertex] && cab[i+1]>hilight[Vertex]) {
        sprintf( txt, "\t%6d", i );
        if ( i==hilight[Cable] )
          sprintf( txt,"@B%d%s", FL_GRAY, ts=strdup(txt) );
        hinfo->add( txt );
      }
    if ( ts != NULL ) {
      free(ts);
      ts=NULL;
    }
  }
  if ( numSurf() ) {
    sprintf( txt, "Attached elements:" );
    hinfo->add( txt );
    for ( int i=0; i<numSurf(); i++ ) {
      vector<SurfaceElement*> ele=surface(i)->ele();
      for ( int j=0; j<surface(i)->num(); j++ ) {
        const int* nl = ele[j]->obj();
        for ( int k=0; k<ele[j]->ptsPerObj(); k++ ) {
          if ( nl[k]==hilight[Vertex] ) {
            sprintf( txt, "\t%6d", j );
            if ( j==hilight[SurfEle] )
              sprintf( txt,"@B%d%s", FL_GRAY, ts=strdup(txt) );
            hinfo->add( txt );
            for ( int m=0; m<ele[j]->ptsPerObj(); m++ )
              if ( nl[m]!=hilight[Vertex] ) att_nodes.insert(nl[m]);
          }
        }
      }
    }
    if ( ts != NULL ) {
      free(ts);
      ts=NULL;
    }
  }
  if ( _numVol ) {
    sprintf( txt, "Attached volume elements:" );
    hinfo->add( txt );
    for ( int i=0; i<_numVol; i++ ) {
      const int* tet=_vol[i]->obj();
      for ( int j=0; j<_vol[i]->ptsPerObj(); j++ )
        if ( tet[j]==hilight[Vertex] ) {
          sprintf( txt, "\t%d", i );
          if ( tet[j]==hilight[VolEle] )
            sprintf( txt,"@B%d%s", FL_GRAY, ts=strdup(txt) );
          hinfo->add( txt );
          for ( int k=0; k<_vol[i]->ptsPerObj(); k++ ) {
            if ( tet[k]!=hilight[Vertex] ) att_nodes.insert(tet[k]);
          }
        }
    }
    if ( ts != NULL ) {
      free(ts);
      ts=NULL;
    }
  }
  sprintf( txt, "Attached nodes:" );
  hinfo->add( txt );
  for ( set<int>::iterator p=att_nodes.begin(); p!=att_nodes.end(); p++ ) {
      sprintf( txt, "\t%d", *p );
      hinfo->add( txt );
    }
  hinfo->add( "" );
  //Cables
  if ( _cable->num() ) {
    sprintf(txt, "@b@C%6dCable: %d of %d", FL_CYAN, hilight[Cable],
            _cable->num() );
    hinfo->add( txt );
    hinfo->add( "nodes:" );
    sprintf( txt, "\t%6d", *(_cable->obj(hilight[Cable])) );
    if ( _cable->obj(hilight[Cable])[0]==hilight[Vertex] )
      sprintf( txt,"@B%d%s", FL_GRAY, ts=strdup(txt) );
    hinfo->add( txt );
    sprintf( txt, "\t%6d", *(_cable->obj(hilight[Cable]+1))-1 );
    if ( _cable->obj(hilight[Cable]+1)[0]-1==hilight[Vertex] )
      sprintf( txt,"@B%d%s", FL_GRAY, ts=strdup(txt) );
    hinfo->add( txt );
    hinfo->add( "" );
    if ( ts != NULL ) {
      free(ts);
      ts=NULL;
    }
  }
  //Connections
  if ( _cnnx->num() ) {
    sprintf(txt, "@b@C%6dConnection: %d of %d", FL_MAGENTA, hilight[Cnnx],
            _cnnx->num() );
    hinfo->add( txt );
    hinfo->add( "nodes:\t" );
    sprintf( txt, "\t%6d", _cnnx->obj(hilight[Cnnx])[0] );
    if ( _cnnx->obj(hilight[Cnnx])[0] ==hilight[Vertex] )
      sprintf( txt,"@B%d%s", FL_GRAY, ts=strdup(txt) );
    hinfo->add( txt );
    sprintf( txt, "\t%6d", _cnnx->obj(hilight[Cnnx])[1] );
    if ( _cnnx->obj(hilight[Cnnx])[1]==hilight[Vertex] )
      sprintf( txt,"@B%d%s", FL_GRAY, ts=strdup(txt) );
    hinfo->add( txt );
    hinfo->add("");
    if ( ts != NULL ) {
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
  if ( (in=gzopen(fn, "r" )) != NULL || (in=gzopen(strcat(fn,".gz"),"r")) != NULL ) {
    char buff[bufsize];
    gzgets(in, buff, bufsize);
    sscanf( buff, "%d", &numNewReg );
    _numReg += numNewReg;
    _region = (RRegion **)realloc( _region, _numReg*sizeof(RRegion*) );
    for ( int i=_numReg-numNewReg; i<_numReg; i++ ) {
      int firstpt, lastpt, label;
      gzgets(in, buff, bufsize);
      sscanf( buff, "%d %d %d", &firstpt, &lastpt, &label );
      _region[i] = new RRegion( pt.num(), label, false );
      for ( int e=firstpt; e<=lastpt; e++ )
        _region[i]->pt_member(e,true);
    }
    gzclose(in);
  }
}


int Model::number(Object_t a )
{
  int nele=0;

  switch ( a ) {
    case Vertex:
      return pt.num();
    case Cnnx:
      return _cnnx->num();
    case Cable:
      return _cable->num();
    case Surface:
      return _surface.size();
    case SurfEle:
      for ( int s=0; s<_surface.size(); s++ )
        nele += surface(s)->num();
      return nele;
    case VolEle:
      return _numVol;
    case RegionDef:
      return _numReg;
  }
}


/** set vertex normals for a surface
 *
 * \param sp pointer to surface
 */
const GLfloat* 
Model::vertex_normals(Surfaces *sp)
{
  sp->get_vert_norms( _vertnrml );
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
  char eletype[3] = "Tt"; //default assumes tetrahedral file
  const int bufsize=1024;
  char buf[bufsize];

  try {
    in   = openFile( fname, "tetras" );
    tets = true;
  } catch (...) {
    try {
      in = openFile( fname, "elem" );
    } catch (...) {return false;}
  }

  if ( gzgets(in, buf, bufsize) == Z_NULL ) return false;
  sscanf( buf, "%d", &_numVol );

  _vol = new VolElement*[_numVol];

  gzFileBuffer file(in);
  int nele = _numVol;
  int ne    = 0;
  int surfe = 0;
  for( int i=0; i<nele; i++ ) {
    if( file.gets(buf, bufsize)==Z_NULL || !strlen(buf) ) break;
       int n[9];
       if( tets ) {
         //sscanf( buf, "%d %d %d %d %d", n, n+1, n+2, n+3, n+4 );
         char * p = buf;
         for (size_t i=0; *p && i<5; i++)
  	   n[i] = strtol(p, &p, 10);
       } 
       else 
       {
       //sscanf( buf, "%2s %d %d %d %d %d %d %d %d %d", eletype,
       // 		n, n+1, n+2, n+3, n+4, n+5, n+6, n+7, n+8 );
         eletype[0]=buf[0];
         eletype[1]=buf[1];
         char * p = buf+3;
         for (size_t i=0; *p && i<9; i++)
	   n[i] = strtol(p, &p, 10);
    }

    if( !strcmp( eletype, "Tt" ) ) {
	  _vol[ne] = new Tetrahedral( &pt );
	  _vol[ne]->add( n, n[4] );
	  ne++;
	} else if( !strcmp( eletype, "Hx" ) ) {
	  _vol[ne] = new Hexahedron( &pt );
	  _vol[ne]->add( n, n[8] );
	  ne++;
	} else if( !strcmp( eletype, "Py" ) ) {
	  _vol[ne] = new Pyramid( &pt );
	  _vol[ne]->add( n, n[5] );
	  ne++;
	} else if( !strcmp( eletype, "Pr" ) ) {
	  _vol[ne] = new Prism( &pt );
	  _vol[ne]->add( n, n[6] );
	  ne++;
	} else if( !strcmp( eletype, "Tr" ) || !strcmp( eletype, "Qd" )  ) {
	  // surface elements ignored
	  _numVol--;
      surfe++;
	} else {
	  fprintf(stderr, "Unsupported element type: %s\n", eletype);
	  delete[] _vol;
	  _numVol = 0;
	  _vol = NULL;
      gzclose(in);
      return false;
    }
  }
  if( ne+surfe<nele) {
	fprintf( stderr, "Warning: truncated element file? stated elements: %d, "
			         " surface elements read: %d, volume elements read: %d\n",
			            nele, surfe, ne );
	_numVol=ne;
	VolElement **nv =new VolElement*[_numVol]; 
	memcpy( nv, _vol, ne*sizeof(VolElement*) );
	delete[] _vol;
	_vol = nv;
  }
  gzclose(in);
}


/** find the local surface element form the global number
 *
 * \param gele global element number
 * \param surf surface containing the element
 *
 * \return the local element number in the surface,-1 if not in range
 */
int Model::localElemnum( int gele, int& surf )
{
  for ( surf=0; surf<_surface.size(); surf++ )
    if ( (gele-=_surface[surf]->num())<0) break;

  return gele+_surface[surf]->num();
}



/** delete a surface
 *
 *  \param s the number of the surface to delete
 */
void
Model::surfKill( int s )
{
  vector<Surfaces *>:: iterator it = _surface.begin();
  for( int i=0; i<s; i++ )
	it++;
  _surface.erase(it);
}


#define BUFSIZE 1024
/** read in one instant in time which is part of a element  time file
 *
 *  \param pt_in   already open point file
 *  \param elem_in already opened element file
 *
 *  \return 
 */
bool Model :: read_instance( gzFile pt_in, gzFile elem_in, 
                                       const GLfloat* offset )
{
  // read in points
  if( pt_in != NULL ) {
    int num_pt;
    sscanf( get_line(pt_in), "%d", &num_pt );
    GLfloat *p = new GLfloat[num_pt*3];
    for( int i=0; i< num_pt; i++ ) {
      sscanf( get_line(pt_in), "%f %f %f", p+i*3, p+i*3+1, p+i*3+2 );
      sub( p+i*3, offset, p+i*3 ); // center wrt fixed model
    }
    pt.add( p, num_pt );
    pt.offset( offset );
	pt.setVis( true );
    delete[] p;
  }

  // read in elements
  _surface.push_back( new Surfaces( &pt ) );
  _cnnx = new Connection( &pt );
  if( elem_in != NULL ) {
    int num_elem;
	sscanf( get_line(elem_in), "%d", &num_elem );
	int *Cxpt = (int *)malloc(num_elem*2*sizeof(int) ), numCx=0;
	_vol = new VolElement*[num_elem];

	int n[10];
	for( int i=0; i<num_elem; i++ ) {
      char type[10];
      sscanf( get_line(elem_in),"%s %d %d %d %d %d %d %d %d %d %d",type,
			  n, n+1, n+2, n+3, n+4, n+5, n+6, n+7, n+8, n+9 );
      if( !strcmp( type, "Ln") ) {
        Cxpt = (int *)realloc( Cxpt, numCx*sizeof(int)*2+2 );
        Cxpt[numCx*2] = n[0];
        Cxpt[numCx*2+1] = n[1];
        numCx++;
      } else if( !strcmp( type, "Tr" ) ) {
        _surface.back()->ele().push_back( new Triangle( &pt ) );
        _surface.back()->ele().back()->define( n );
      } else if( !strcmp( type, "Qd" ) ) {
        _surface.back()->ele().push_back( new Quadrilateral( &pt ) );
        _surface.back()->ele().back()->define( n );
        _surface.back()->ele().back()->compute_normals(0,0);
      } else if( !strcmp( type, "Tt" ) ) {
        _vol[_numVol] = new Tetrahedral( &pt );
        _vol[_numVol]->add( n, n[4] );
        _numVol++;
      } else if( !strcmp( type, "Hx" ) ) {
        _vol[_numVol] = new Hexahedron( &pt );
        _vol[_numVol]->add( n, n[8] );
        _numVol++;
      } else if( !strcmp( type, "Py" ) ) {
        _vol[_numVol] = new Pyramid( &pt );
        _vol[_numVol]->add( n, n[5] );
        _numVol++;
      } else if( !strcmp( type, "Pr" ) ) {
        _vol[_numVol] = new Prism( &pt );
        _vol[_numVol]->add( n, n[6] );
        _numVol++;
      }
    }
    // define connections
    _cnnx->define( Cxpt, numCx );
    free( Cxpt );
  }
  determine_regions();
  return true;
}



