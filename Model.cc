#include "Model.h"
#include <set>
#include <unordered_set>
#include <map>
#include <vector>
#include <string>
#include "DataOpacity.h"
#ifdef HAVE_GL2PS
#include "gl2ps.h"
#endif
#include "VecData.h"
#include <sstream>
#include <FL/filename.H>
#include "gzFileBuffer.h"
#include "logger.hpp"
#include <algorithm>
#include <utility>
#include <queue>
#ifdef USE_VTK
#include <vtkSmartPointer.h>
#include<vtkUnstructuredGrid.h>
#include<vtkXMLUnstructuredGridReader.h>
#include<vtkCellData.h>
#endif


#ifdef _OPENMP
#include<omp.h>
#endif

struct Face {
  int nsort[MAX_NUM_SURF_NODES];  //!< sorted nodes
  int norig[MAX_NUM_SURF_NODES];  //!< nodes in original order
  int nnode;                      //!< number of nodes
};

int intcmp( const void *a, const void *b )
{
  return *(int *)a-*(int *)b; 
}

#ifdef USE_SET

/** compare 2 faces for sorting */
bool cmpface( const Face &A, const Face &B)
{
  if( A.nnode != B.nnode ) return  A.nnode < B.nnode;
  for(  int i=0; i< A.nnode; i++ )
    if( A.nsort[i] != B.nsort[i] ) return  A.nsort[i] < B.nsort[i];
  return false;
};

typedef set<Face, bool (*)(const Face&, const Face&)> faceset;

#else

/** compare if 2 faces equivalent */
bool operator==( const Face &A, const Face &B)
{
  if( A.nnode != B.nnode ) return  false;
  return !memcmp( A.nsort, B.nsort, A.nnode*sizeof(A.nsort[0]) );
}


/** generate hash value for a face */
const size_t hash_offset = 1000000000;
struct face_hash
{
  size_t operator() ( const Face &A ) const {
    unsigned long key = A.nnode;
    for( int i=0; i<A.nnode; i++ )
      key += A.nsort[i]*hash_offset*(i+1);
    return hash<unsigned long>()(key);
  }
};

typedef unordered_set<Face, face_hash> faceset;

#endif // USE_SET

/** make a face from a node list
 *
 * \param f new face
 * \param n number of nodes in surface face
 * \param orig nodes in original order
 */
void
make_face( Face &f, int n, int* orig )
{
  f.nnode = n;
  memcpy( f.norig, orig, n*sizeof(int) );
  memcpy( f.nsort, orig, n*sizeof(int) );
  qsort( f.nsort, n, sizeof(int), intcmp ); 
}


/** read in a line from a file, ignoring lines beginning with "#"
 *
 * \warning (GD) lines are limited to 2048 bytes.
 * \warning not threadsafe
 * \return pointer to a static buffer
 */
char *
get_line( gzFile in )
{
  const int bufsize=2048;
  static char buf[bufsize];
  char *retval;
  do {
	retval = gzgets( in, buf, bufsize );
	if (retval == Z_NULL)
	  {
	    int err;
	    char const * errd = gzerror(in, &err);
	    std::cerr << errd << std::endl;
	    continue;
	  }
  } while( retval != Z_NULL && buf[0]=='#' );
  
  return retval==Z_NULL?NULL:buf;
}


/** format string for hilight window output 
 *
 * \param i  index
 * \param hi is this a hilighted index
 *
 * \return a pointer to a constant string
 */
char * const
format_hilite( int i, bool hi ) 
{
  static char txt[256];
  if( !hi )
    sprintf( txt, "\t%6d", i );
  else
    sprintf( txt,"@B%d\t%6d", FL_GRAY, i);
  return txt;
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


Model::Model()
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
  char fn[bufsize];
  strcpy( fn, fnt );
  gzFile in;
  _base1 = base1;
  pt.base1( base1 );
  
  LOG_TIMER_RESET;
  try {
    pt.read( fn );
  } catch (...) {
    fprintf(stderr, "Unable to read proper points file\n" );
    exit(1);
  }
  LOG_TIMER("Read Points");
  
  allvis.resize(pt.num());
  allvis.assign(pt.num(), true );

  if ( !strcmp( ".gz", fn+strlen(fn)-3 ) ) // temporary
    fn[strlen(fn)-3] = '\0';
  if ( !strcmp( ".pts", fn+strlen(fn)-4 ) ) // temporary
    fn[strlen(fn)-3] = '\0';
  if ( fn[strlen(fn)-1] != '.' ) {
    fn[strlen(fn)+1] ='\0';
    fn[strlen(fn)] ='.';
  }
  _file = fn;

  LOG_TIMER_RESET;
  _cnnx   = new Connection( &pt );
  _cnnx->read( fn );
  add_cnnx_from_elem( fn );
  LOG_TIMER("Read Connections");
  
  _cable  = new ContCable( &pt );
  _cable->read( fn );
  //LOG_TIMER("cable->read()");
  
  if(!no_elems) {
    LOG_TIMER_RESET;
    read_elem_file( fn );
    LOG_TIMER("Read Elements");
  }

  read_region_file( in, fn );

  determine_regions();
  
  LOG_TIMER_RESET;
  add_surface_from_elem( fn );
  LOG_TIMER("Add Surface from Elements");
  
  LOG_TIMER_RESET;
  add_surface_from_tri( fn );
  LOG_TIMER("Add Surface from Tri");
  
  LOG_TIMER_RESET;
  add_surface_from_surf( fn );
  LOG_TIMER("Add Surface from Surf");

  //! \todo Figure out why we do this 
  //for( int s=0; s<numSurf(); s++ )
    //surface(s)->flip_norms();

  find_max_dim_and_bounds();

  return true;
}

#ifdef USE_HDF5
bool Model::read(hid_t hdf_file, bool base1, bool no_elem)
{
  LOG_TIMER_RESET;
  pt.read(hdf_file);
  LOG_TIMER("Read Points");
  
  allvis.resize(pt.num());
  allvis.assign(pt.num(), true);
  
  LOG_TIMER_RESET;
  _cnnx = new Connection(&pt);
  _cnnx->read(hdf_file);
  LOG_TIMER("Read Connections");
  
  LOG_TIMER_RESET;
  _cable = new ContCable(&pt);
  _cable->read(hdf_file);
  LOG_TIMER("Read Cables");
  
  LOG_TIMER_RESET;
  if (!no_elem) add_elements(hdf_file);
  LOG_TIMER("Read Elements");
  
  LOG_TIMER_RESET;
  add_regions(hdf_file);
  determine_regions();
  LOG_TIMER("Add + Determine Regions");
  
  LOG_TIMER_RESET;
  add_surfaces(hdf_file);
  LOG_TIMER("Add Surfaces");
  
  find_max_dim_and_bounds();
  
  return true;
}


void Model::add_surfaces(hid_t hdf_file) {
  int *elements;
  ch5_dataset dset_info;
  
  // From 2D elements
  if ( ch5m_elem_get_info(hdf_file, &dset_info) ){
    cerr << "Could not find elements dataset" << endl;
    return;
  }
  
  elements = (int*) malloc(sizeof(int) * dset_info.count * dset_info.width);
  if (elements == NULL) {
    cerr << "Could not allocate memory for elements" << endl;
    exit(1);
  }
  
  if ( ch5m_elem_get_all(hdf_file, elements) ){
    free(elements);
    cerr << "Could not read from elements dataset" << endl;
    return;
  }
  
  add_surfaces(elements, dset_info.count, dset_info.width, NULL);
  free(elements);
  
  // From explicitly defined surfaces
  int surfaceCount = ch5m_surf_get_count(hdf_file);
  for (int i = 0; i < surfaceCount; i++) {
    hid_t surf_id;
    ch5m_surf_open(hdf_file, i, &surf_id);
    
    ch5m_surf_get_elem_info(surf_id, &dset_info);
    
    elements = (int*) malloc(sizeof(int) * dset_info.count * dset_info.width);
    if (elements == NULL) {
      cerr << "Could not allocate memory for elements" << endl;
      exit(1);
    }

    char file_name_buf[255];
    char *surf_name;
    int name_result = ch5m_surf_get_name(surf_id, &surf_name);
    if (name_result != 1) {
      H5Fget_name(hdf_file, file_name_buf, 255);
      surf_name = (char*) malloc(sizeof(char) * 255);
      sprintf(surf_name, "%s:%d", fl_filename_name(file_name_buf), i);
    }
    if ( !ch5m_elem_get_all_by_dset(surf_id, elements) )
      add_surfaces(elements, dset_info.count, dset_info.width, surf_name);
    else
      cerr << "Could not read from surface " << i << " dataset" << endl;
    free(elements);
    free(surf_name);
    
    ch5m_surf_close(surf_id);
    
  }
}

void Model::add_surfaces(int *elements, int count, int max_width, char *name) {
  // Count up the number of elements in each region and build some lookup maps
  // for pairing regions to newly formed surfaces
  map<int,int> regionCounts;
  for (int i = 0; i < count; i++) {
    if ((elements[i * max_width] == CH5_TRIANGLE) || (elements[i * max_width] == CH5_QUADRILATERAL))
      regionCounts[elements[i * max_width + CH5_ELEM_REGION_OFFSET]]++;
  }
  if (regionCounts.size() == 0) return;
  
  map<int,int> regionToSurfaceIndex;
  map<int,int> elementCounts;
  map<int,int>::iterator regionCountsIter = regionCounts.begin();
  map<int,int>::iterator regionCountsEnd  = regionCounts.end();
  int newSurfaceCount = 0;
  int origSurfaceEnd  = _surface.size();
  while (true) {
    if (regionCountsIter->second != 0) {
      Surfaces *newSurf = new Surfaces(&pt);
      newSurf->num(regionCountsIter->second);
      if (name != NULL) newSurf->label(name);
      _surface.push_back(newSurf);
      regionToSurfaceIndex[regionCountsIter->first] = _surface.size() - 1;
      elementCounts[regionCountsIter->first] = 0;
      newSurfaceCount++;
    }
    if (++regionCountsIter == regionCountsEnd) break;
  }
  
  // Iterate through elements and populate the surfaces
  if (newSurfaceCount > 0) {
    for (int i = 0; i < count; i++) {
      int region = elements[i * max_width + CH5_ELEM_REGION_OFFSET];
      int surfaceIndex = regionToSurfaceIndex[region];
      int elementCount = elementCounts[region];
      switch (elements[i * max_width]) {
        case CH5_TRIANGLE:
          _surface[surfaceIndex]->addele(elementCount, new Triangle(&pt));
          break;
        
        case CH5_QUADRILATERAL:
          _surface[surfaceIndex]->addele(elementCount, new Quadrilateral(&pt));
          break;
        
        default:
          break;
      }
      if (_surface[surfaceIndex]->ele(elementCount) == NULL) continue;
      _surface[surfaceIndex]->ele(elementCount)->define(&elements[i * max_width + CH5_ELEM_DATA_OFFSET]);
      _surface[surfaceIndex]->ele(elementCount)->compute_normals(0,0);
      elementCounts[region]++;
    }
    
    for (int i = origSurfaceEnd; i < origSurfaceEnd + newSurfaceCount; i++) {
      _surface.at(i)->determine_vert_norms(pt);
    }
  }
}
#endif // USE_HDF5

#ifdef USE_VTK

const char *VTK2CARP_etype[] = {
  " ",      // VTK_EMPTY_CELL       = 0,
  "Pt",     // VTK_VERTEX           = 1,
  "PP",     // VTK_POLY_VERTEX      = 2,
  "Ln",     // VTK_LINE             = 3,
  "Ca",     // VTK_POLY_LINE        = 4,
  "Tr",     // VTK_TRIANGLE         = 5,
  "TS",     // VTK_TRIANGLE_STRIP   = 6,
  "PG",     // VTK_POLYGON          = 7,
  "Px",     // VTK_PIXEL            = 8,
  "Qd",     // VTK_QUAD             = 9,
  "Tt",     // VTK_TETRA            = 10,
  "Vx",     // VTK_VOXEL            = 11,
  "Hx",     // VTK_HEXAHEDRON       = 12,
  "Pr",     // VTK_WEDGE            = 13,
  "Py",     // VTK_PYRAMID          = 14,
  "Pr",     // VTK_PENTAGONAL_PRISM = 15,
  "HP",     // VTK_HEXAGONAL_PRISM  = 16,
};


/** read in all types of elements 
 *
 * \param grid     VTU mesh
 * \param no_velem do not read volum elements
 *
 * \return true if surface elements present
 */
bool
Model:: read_objects(vtkUnstructuredGrid* grid, bool no_velem )
{
  int numCell = grid->GetNumberOfCells();
  int nsurf=0,
      ncnnx=0;

  // count the element types
  for( int i=0; i<numCell; i++ ) {
    const char *eletype = VTK2CARP_etype[grid->GetCellType(i)];
    if(  !strcmp( eletype, "Tt" ) || !strcmp( eletype, "Hx" ) || 
         !strcmp( eletype, "Py" ) || !strcmp( eletype, "Pr" )   )
      _numVol++;
    else if( !strcmp( eletype, "Tr" ) ||  !strcmp( eletype, "Qd" ) )
      nsurf++;
    else if( !strcmp( eletype, "Ln" ) ) {
      ncnnx++;
    } else {
      fprintf(stderr, "Unsupported element type: %s\n", eletype);
    }
  }

  if( no_velem ) _numVol=0;
  _vol = new VolElement*[_numVol];
  _numVol = 0;

  if( nsurf ) {
    Surfaces *newSurf = new Surfaces(&pt);
    newSurf->label("default");
    _surface.push_back(newSurf);
    newSurf->num(nsurf);
    nsurf = 0;
  }

  vtkDataArray* regdata = grid->GetCellData()->GetArray("Regions");
  vector<int> cnnx;
  int reg = 0;

  // read in elements
  for( int i=0; i<numCell; i++ ) {

    const char *eletype = VTK2CARP_etype[grid->GetCellType(i)];

    // convert points to int
    vtkIdType *nvtk, nn;
    grid->GetCellPoints( i, nn, nvtk );
    int n[10];
    for( int j=0; j<nn; j++ )
      n[j] = nvtk[j];

    if( regdata ) reg = (int)( regdata->GetTuple1(i) );

    if( !no_velem && !strcmp( eletype, "Tt" ) ) {
      _vol[_numVol] = new Tetrahedral( &pt );
      _vol[_numVol]->add( n, reg );
      _numVol++;
    } else if( !no_velem && !strcmp( eletype, "Hx" ) ) {
      swap( n[5], n[7] );
      _vol[_numVol] = new Hexahedron( &pt );
      _vol[_numVol]->add( n, reg );
      _numVol++;
    } else if( !no_velem && !strcmp( eletype, "Py" ) ) {
      _vol[_numVol] = new Pyramid( &pt );
      _vol[_numVol]->add( n, reg );
      _numVol++;
    } else if( !no_velem && !strcmp( eletype, "Pr" ) ) {
      swap( n[1], n[2] );
      _vol[_numVol] = new Prism( &pt );
      _vol[_numVol]->add( n, reg );
      _numVol++;
    } else if( !strcmp( eletype, "Tr" ) ) {
     swap( n[1], n[2] );
	  _surface[0]->addele(nsurf, new Triangle( &pt ));
	  _surface[0]->ele(nsurf)->define(n);
  	  _surface[0]->ele(nsurf)->compute_normals(0,0);
      nsurf++;
    } else if ( !strcmp( eletype, "Qd" )  ) {
      swap( n[1], n[3] );
	  _surface[0]->addele(nsurf, new Quadrilateral( &pt ));
      _surface[0]->ele(nsurf)->define(n);
      _surface[0]->ele(nsurf)->compute_normals(0,0);
      nsurf++;
    } else if( !strcmp( eletype, "Ln" ) ) {
      cnnx.push_back(n[0]);
      cnnx.push_back(n[1]);
    } 
  }

  // add any connections found
  if( cnnx.size() )
    _cnnx->add( cnnx.size()/2., cnnx.data() );

  // add any surface elements found
  if( nsurf ) {
    _surface[0]->num(nsurf);
    _surface[0]->determine_vert_norms( pt );
    _surface[0]->label( "0" );
    return true;
  }

  return false;
}


bool 
Model::read_vtu( const char* filename, bool no_elem ) 
{
  vtkSmartPointer<vtkXMLUnstructuredGridReader> reader =
    vtkSmartPointer<vtkXMLUnstructuredGridReader>::New();
  reader->SetFileName(filename);
  reader->Update();
  vtkUnstructuredGrid* grid = reader->GetOutput();
  int np = grid->GetNumberOfPoints();

  GLfloat  *p = new GLfloat[np*3];
  double *x;
  for( int i=0;i<np; i++ ){
    x = grid->GetPoint(i);
    for( int j=0; j<3; j++ )
      p[3*i+j] = x[j];
  }
  pt.add( p, np );

  allvis.resize(pt.num());
  allvis.assign(pt.num(), true );

  _file = filename;
  _file.erase(_file.size()-4);

  _cnnx   = new Connection( &pt );
  _cable  = new ContCable( &pt );
  
  read_objects(grid, no_elem );
 
  determine_regions();
  
  find_max_dim_and_bounds();
  return true;
}


#endif


void Model::find_max_dim_and_bounds()
{
  const GLfloat *p = pt.pt();
  const GLfloat *offset = pt.offset();

  _maxdim = fabs(p[0]-offset[0]);
  for ( int i=0; i<pt.num()*3; i+=3 ) 
    for( int j=0; j<3; j++ )
      if ( fabs(p[i+j]-offset[j])>_maxdim ) 
        _maxdim = fabs(p[i+j]-offset[j]);
      
  int i=0;
  for( ; i<pt.num(); i++ )
    if( pt.pt(i)[2] )
      break;
  if( i==pt.num() )
    _2D = true;
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
        if ( _region[r]->pt_member(o[0])==true ) {
          _region[r]->first(Cnnx,i);
          break;
        }
      }
    }
  }
}


/** add conections by selecting them from .elem file
 *
 *  \param fname element file
 */
int Model::add_cnnx_from_elem( string fname )
{
  /* determine file name */
  gzFile in;
  if ( (in=gzopen( (fname+="elem").c_str(), "r" )) == NULL )
    if ((in=gzopen( (fname+=".gz").c_str(), "r" )) == NULL ) {
      return -1;
    }

  // Count the number of surface elements in each surface
  char buff[bufsize];
  gzgets(in,buff,bufsize);           // throw away first line

  gzFileBuffer file(in);
  int numcx=0, *cnnx=NULL;
  while ( file.gets(buff,bufsize) != Z_NULL ) {
	char etype[10],reg[10];
	if( !strncmp(buff,"Ln",2) ) {
      if( !(numcx%10000) )
        cnnx = (int *)realloc( cnnx, (numcx/10000+1)*2*sizeof(int) );
	  if(sscanf( buff,"%s %d %d %s", etype, cnnx+2*numcx, cnnx+2*numcx+1, reg)<4 )
		strcpy(reg,"EMPTY");
	} else
	  continue;  //ignore volume elements
  }
  _cnnx->add( numcx, cnnx );
  free( cnnx );

  return numcx;
}


/** add surface by selecting 2D elements from .elem file
 *
 *  A surface will be created for each different region specified.
 *  Also, all elements without a region specified will form a surface.
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
  map<string,string> surfNameMap;
  while ( file.gets(buff,bufsize) != Z_NULL ) {
    char surfnum[16]  ="";
    char surfname[256]="";
    if ( !strncmp(buff,"Tr",2) ) {
      if ( sscanf(buff, "%*s %*d %*d %*d %s %s", surfnum, surfname )<1 )
        strcpy( surfnum, "EMPTY" );
      else if( surfname[0] )
        surfNameMap[surfnum] = surfname;
    } else if ( !strncmp(buff,"Qd",2) ) {
      if ( sscanf(buff, "%*s %*d %*d %*d %*d %s %s", surfnum, surfname )<1 )
        strcpy( surfnum, "EMPTY" );
      else if( surfname[0] )
        surfNameMap[surfnum] = surfname;
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
    if( surfNameMap.count(iter->first) )
        _surface[s]->label( surfNameMap[iter->first] );
    else {
      string sname;
      if( iter->first != "EMPTY" )
        sname = iter->first+"-";
      sname.append( fn, strlen(fn)-1 );
      _surface[s]->label( sname );
    }
    surfmap[iter->first] = s;   // map region to surface index
  }

  surfs.clear();                // now use for current element in each surface
  gzrewind(in);

  gzFileBuffer file2(in);

  gzgets(in,buff,bufsize);      //throw away first line
  /*
   * note that we flip the node order to get the proper normal 
   * \todo Should not be this way!
   */
  while( file2.gets(buff,bufsize) !=Z_NULL ) {
	char etype[10],reg[10];
	int  idat[4];
	if( !strncmp(buff,"Tr",2) ) {
	  if(sscanf( buff,"%s %d %d %d %s", etype, idat, idat+2, idat+1, reg)<5 )
		strcpy(reg,"EMPTY");
	  _surface[surfmap[reg]]->addele(surfs[reg],new Triangle( &pt ));
	} else  if( !strncmp(buff,"Qd",2) ) {
	  if(sscanf( buff,"%s %d %d %d %d %s", etype, idat, idat+3, idat+2, 
				  idat+1, reg)<6 )
		strcpy(reg,"EMPTY");
	  _surface[surfmap[reg]]->addele(surfs[reg],new Quadrilateral( &pt ));
	} else
	  continue;  //ignore volume elements
	_surface[surfmap[reg]]->ele(surfs[reg])->define(idat);
	_surface[surfmap[reg]]->ele(surfs[reg])->compute_normals(0,0);
	surfs[reg]++;
  }
  gzclose( in );

  for ( int s=oldnumSurf; s<_surface.size(); s++ ) 
    _surface[s]->determine_vert_norms( pt );

  return _surface.size();
}


/** find bounding surfaces for all the regions
 *
 *  We do this by adding the faces of all elements into a tree and removing a face if
 *  it appears twice
 *
 *  \return number of new surfaces
 */
int Model::add_region_surfaces()
{
  int numNewSurf=0;

  for( int r=0; r<_numReg; r++ )  {

#ifdef _OPENMP
    int numthrd = omp_get_max_threads();
#else
    int numthrd = 1;
#endif

#ifdef USE_SET
    vector< faceset > facetree(numthrd, faceset(cmpface));
#else
    vector< faceset > facetree(numthrd);
#endif

#pragma omp parallel for 
    for( int e=0; e<_numVol; e++ ) {
#ifdef _OPENMP
      int thrd    = omp_get_thread_num();
#else
      int thrd    = 0;
#endif
      if( _region[r]->ele_member(e) ) {
        int faces[MAX_NUM_SURF][MAX_NUM_SURF_NODES+1];
        int ns = _vol[e]->surfaces( faces );
        for( int i=0; i<ns; i++ ) {
          Face newface;
          make_face( newface, faces[i][0], faces[i]+1 );
          faceset::iterator iter = facetree[thrd].find(newface);
          if (iter != facetree[thrd].end()) {
            facetree[thrd].erase(iter);
          } else {
            facetree[thrd].insert(newface);
          }
        }
      }
    }

#if 0
    // count the number of faces left in the list
    for( int s=1; s<numthrd; s++ ) {
      for(faceset::iterator sn=facetree[s].begin(); sn!=facetree[s].end(); sn++) { 
        faceset::iterator iter = facetree[0].find(*sn);
        if (iter != facetree[0].end()) {
          facetree[0].erase(iter);
        } else {
          facetree[0].insert(*sn);
        }
      }
    }
#else
    int nleft = numthrd;
    while( nleft>1 ) {
      int add = (nleft+1)/2;
      nleft /= 2;
#pragma omp parallel for num_threads(nleft)
      for( int s=0; s<nleft; s++ ) {
        for(auto sn=facetree[s+add].begin(); sn!=facetree[s+add].end(); sn++) { 
          faceset::iterator iter = facetree[s].find(*sn);
          if (iter != facetree[s].end()) {
            facetree[s].erase(iter);
          } else {
            facetree[s].insert(*sn);
          }
        }
      }
      nleft = add;
    }
#endif

    if( facetree[0].size() ) {   // convert the left over faces into a surface
      numNewSurf++;
      
      _surface.push_back( new Surfaces( &pt ) );
      _surface.back()->num( facetree[0].size() );

      int e=0;
      for(auto iter=facetree[0].begin(); iter!=facetree[0].end(); ++iter) {

        Face newface = *iter;
        if( newface.nnode == 3 ) {
          _surface.back()->addele(e, new Triangle( &pt ));
        } else if( newface.nnode==4 ) {
          _surface.back()->addele(e, new Quadrilateral( &pt ));
        } else {
          cout << newface.nnode;
          for( int n=0;n< newface.nnode; n++ )
            cout << "In element " << distance(facetree[0].begin(),iter) 
                        << " of " << facetree[0].size()
                        << ": " << newface.nnode << endl;
          assert(0);
        }

        _surface.back()->ele(e)->define( newface.norig );
        _surface.back()->ele(e)->compute_normals(0,0);
        e++;
      }
      _surface.back()->determine_vert_norms( pt );
      ostringstream regnum;
      regnum << "Reg " << r;
      _surface.back()->label( regnum.str() );

    }
  }
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
        _surface.back()->addele(i,new Triangle( &pt ));
      else if( !strcmp(etype, "Qd") )
        _surface.back()->addele(i,new Quadrilateral( &pt ));
      else {
        _surface.pop_back();
        return surfnum;
      }
      _surface.back()->ele(i)->define(nl);
      if( !check_element( _surface.back()->ele(i) ) )  {
        _surface.pop_back();
        return surfnum;
      }
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
        _surface.back()->addele(i,new Triangle( &pt ));
        int nl[3];
        if ( gzgets(in,buff,bufsize) == Z_NULL ||
             sscanf(buff, "%d %d %d", nl, nl+1, nl+2 ) < 3 ) {
          _surface.pop_back();
          return surfnum;
        }
        _surface.back()->ele(i)->define(nl);
        if( !check_element( _surface.back()->ele(i) ) ) {
          _surface.pop_back();
          return surfnum;
        }
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
	  _surface.back()->addele(curele,new Triangle( &pt ));
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
void Model::showobj( Object_t obj, bool *r, bool f )
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
GLfloat* Model::get_color( Object_t obj, int s )
{
  return _region[s<0?0:s]->get_color(obj);
}


void Model::get_mat( int s, float &diff, float &spec, float &shine, float &back )
{
  diff  = _surface[s]->diffuse( );
  spec  = _surface[s]->specular( );
  shine = _surface[s]->shiny( );
  back  = _surface[s]->backlight( );
}


void Model::set_mat( int s, float diff, float spec, float shine, float back )
{
  _surface[s]->diffuse( diff );
  _surface[s]->specular( spec );
  _surface[s]->shiny( shine );
  _surface[s]->backlight( back );
}


void Model::set_color( Object_t obj, int s, float r, float g, float b, float a )
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

void Model::visibility( int r, bool a )
{
  _region[r]->visible(a);
}

void Model::opacity( int s, float opac )
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

void Model::randomize_color( Object_t obj )
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
  if (_cnnx) {
    delete _cnnx;
    _cnnx = 0;
  }

  for ( int i=0; i<_numVol; i++ ) {
    delete _vol[i];
  }

  if (_vol) {
    delete _vol;
    _vol = 0;
  }

  if (_cable) {
    delete _cable;
    _cable = 0;
  }

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

  /////////////////////////////
  // Vertex info
  set<int> att_nodes;        	// list nodes in attached elements
  sprintf(txt, "@b@C%6dVertex: %d of %d", FL_DARK_GREEN, hilight[Vertex],
          pt.num() );
  hinfo->add( txt );
  if ( data != NULL ) {
    sprintf( txt, "value: %f", data[hilight[Vertex]] );
    hinfo->add( txt );
  }
  sprintf( txt, "( %.6g, %.6g, %.6g )", pt.pt()[hilight[Vertex]*3],
           pt.pt()[hilight[Vertex]*3+1], pt.pt()[hilight[Vertex]*3+2]);
  hinfo->add( txt );

  if ( _cable->num() ) {
    const int* cab=_cable->obj();
    for ( int cabnum=0; cabnum<_cable->num();cabnum++ )
      if (cab[cabnum]<=hilight[Vertex] && cab[cabnum+1]>=hilight[Vertex]) {
        sprintf(txt, "in cable: %d", cabnum );
        hinfo->add( txt );
        if( hilight[Vertex]>cab[cabnum] ) att_nodes.insert(hilight[Vertex]-1);
        if( hilight[Vertex]<cab[cabnum+1]-1 ) att_nodes.insert(hilight[Vertex]+1);
      }
  }
  if ( _cnnx->num() ) {
    hinfo->add( "Attached connections:" );
    for ( int cnum=0; cnum<_cnnx->num();cnum++ ) {
      const int* conn = _cnnx->obj(cnum);
      if ( conn[0]==hilight[Vertex] || conn[1]==hilight[Vertex] ) {
        hinfo->add( format_hilite(cnum,cnum==hilight[Cnnx]) );
        att_nodes.insert(conn[conn[0]==hilight[Vertex]?1:0] );
      }
    }
  }
  if ( numSurf() ) {
    hinfo->add( "Attached surface elements:" );
    int nglob=0;
    for ( int i=0; i<numSurf(); i++ ) {
      vector<SurfaceElement*> ele=surface(i)->ele();
      for ( int j=0; j<surface(i)->num(); j++ ) {
        const int* nl = ele[j]->obj();
        for ( int k=0; k<ele[j]->ptsPerObj(); k++ ) {
          if ( nl[k]==hilight[Vertex] ) {
            hinfo->add( format_hilite(j,j==hilight[SurfEle]) );
            for ( int m=0; m<ele[j]->ptsPerObj(); m++ )
              att_nodes.insert(nl[m]);
            break;
          }
        }
      }
      nglob += surface(i)->num();
    }
  }
  if ( _numVol ) {
    hinfo->add( "Attached volume elements:" );
    for ( int i=0; i<_numVol; i++ ) {
      const int* tet=_vol[i]->obj();
      for ( int j=0; j<_vol[i]->ptsPerObj(); j++ )
        if ( tet[j]==hilight[Vertex] ) {
          hinfo->add( format_hilite(i,i==hilight[VolEle]) );
          for ( int k=0; k<_vol[i]->ptsPerObj(); k++ ) {
            att_nodes.insert(tet[k]);
          }
        }
    }
  }
  hinfo->add( "Attached nodes:" );
  att_nodes.erase(hilight[Vertex]);
  for ( set<int>::iterator p=att_nodes.begin(); p!=att_nodes.end(); p++ ) {
      sprintf( txt, "\t%d", *p );
      hinfo->add( txt );
    }
  hinfo->add( "" );
  // data extrema
  if( data )  {
    hinfo->add( "Data Extrema" );
    DATA_TYPE *mind = min_element( data, data+pt.num() ); 
    sprintf( txt, "Minimum: %g @", *mind );
    hinfo->add( txt );
    hinfo->add( format_hilite( mind-data, mind-data==hilight[Vertex]) );
    DATA_TYPE *maxd = max_element( data, data+pt.num() ); 
    sprintf( txt, "Maximum: %g @", *maxd );
    hinfo->add( txt );
    hinfo->add( format_hilite( maxd-data, maxd-data==hilight[Vertex]) );
    hinfo->add( "" );
  }

  //End vertex info
  ///////////////////////////////////
 
  //Cables
  if ( _cable->num() ) {
    sprintf(txt, "@b@C%6dCable: %d of %d", FL_CYAN, hilight[Cable],
            _cable->num() );
    hinfo->add( txt );
    hinfo->add( "nodes:" );
    hinfo->add( format_hilite( *(_cable->obj(hilight[Cable])), _cable->obj(hilight[Cable])[0]==hilight[Vertex]) );
    hinfo->add( format_hilite( *(_cable->obj(hilight[Cable]+1))-1,  _cable->obj(hilight[Cable]+1)[0]-1==hilight[Vertex]));
    hinfo->add( "" );
  }

  // Volume ELements
  if ( _numVol ) {
    int hivol=hilight[VolEle];
    sprintf( txt, "@b@C%6dVolume Element: %d of %d", FL_RED, hivol, _numVol );
    hinfo->add( txt );
    hinfo->add( "nodes:\t" );
    const int* tet=_vol[hivol]->obj();
    for ( int i=0; i<_vol[hivol]->ptsPerObj(); i++ ) {
      char *line = format_hilite( tet[i], tet[i]==hilight[Vertex] ); 
      if ( data != NULL )
        sprintf(line+strlen(line), " -> %f", data[tet[i]] );
      hinfo->add( line );
    }
  }
  hinfo->add("");

  // SurfEles
  int elesurf, lele;
  if ( numSurf() ) {
    lele = localElemnum( hilight[SurfEle], elesurf );
    sprintf(txt, "@b@C%6dSurface Element: %d of %d (%d of %d in %s)",
            FL_BLUE, hilight[SurfEle], number(SurfEle),
            lele, surface(elesurf)->num(), surface(elesurf)->label().c_str());
    hinfo->add( txt );
    hinfo->add( "nodes:\t" );
    for ( int i=0; i<surface(elesurf)->ele(lele)->ptsPerObj(); i++ ) {
      int node=surface(elesurf)->ele(lele)->obj()[i];
      char *line = format_hilite( node, node==hilight[Vertex] ); 
      if ( data != NULL )
        sprintf(line+strlen(line), " -> %f", data[node] );
      hinfo->add( line );
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

  //Connections
  if ( _cnnx->num() ) {
    sprintf(txt, "@b@C%6dConnection: %d of %d", FL_MAGENTA, hilight[Cnnx],
            _cnnx->num() );
    hinfo->add( txt );
    hinfo->add( "nodes:\t" );
    for( int i=0; i<2; i++ ) {
      hinfo->add(format_hilite(_cnnx->obj(hilight[Cnnx])[i],
                         _cnnx->obj(hilight[Cnnx])[i]==hilight[Vertex]));
    }
    hinfo->add("");
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
      _region[i] = new RRegion( pt.num(), 0, label, false );
      for ( int e=firstpt; e<=lastpt; e++ )
        _region[i]->pt_member(e,true);
    }
    gzclose(in);
  }
}


#ifdef USE_HDF5
void Model::add_regions(hid_t hdf_file)
{
  ch5_dataset dset_info;
  if (ch5m_regn_get_info(hdf_file, &dset_info))
    return;
  
  int readRegions[dset_info.count * dset_info.width];
  if ( ch5m_regn_get_all(hdf_file, readRegions) ) {
    cerr << "Could not read regions from HDF file" << endl;
    exit(1);
  }
  
  int oldNumReg = _numReg;
  _numReg += dset_info.count;
  _region = (RRegion**) realloc(_region, _numReg * sizeof(RRegion*));
  if (_region == NULL) {
    cerr << "Could not allocate memory for regions" << endl;
    exit(1);
  }
  
  for (int i = 0; i < dset_info.count; i++) {
    _region[oldNumReg + i] = new RRegion(pt.num(), 0, 0, false);
    for (int e = readRegions[i * 2]; e <= readRegions[i * 2 + 1]; e++)
      _region[oldNumReg + i]->pt_member(e, true);
  }
}
#endif


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
bool Model::read_elem_file( const char *fname )
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
  queue<pair<int,int> > new_cnnx;

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
	} else if( !strcmp( eletype, "Ln" ) ) {
      new_cnnx.push( make_pair(n[0],n[1]) );
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
  if( ne+surfe+new_cnnx.size()<nele) {
	fprintf( stderr, "Warning: truncated element file? stated elements: %d, "
			         " surface elements read: %d, volume elements read: %d,"
                     "line elments read: %d\n",
			            nele, surfe, ne, new_cnnx.size() );
  }
  if( ne != _numVol ) { 
	_numVol=ne;
	VolElement **nv =new VolElement*[_numVol]; 
	memcpy( nv, _vol, ne*sizeof(VolElement*) );
	delete[] _vol;
	_vol = nv;
  }
  // add any line elements as connections
  if(  !new_cnnx.empty() ) {
    int  nc = new_cnnx.size();
    int *nl = new int[nc*2];
    for( int a=0; a<nc; a++ ) {
      nl[a*2]   = new_cnnx.front().first;
      nl[a*2+1] = new_cnnx.front().second;
      new_cnnx.pop();
    }
    _cnnx->add( nc, nl );
    delete[] nl;
  }
      
  gzclose(in);
}

#ifdef USE_HDF5
bool Model::add_elements(hid_t hdf_file)
{
  ch5_dataset info;
  if( ch5m_elem_get_info(hdf_file, &info) )
    return false;
  
  _numVol = info.count;
  _vol = new VolElement*[_numVol];
  int  surfe    = 0;
  int  ne       = 0;
  int *elements = (int*) malloc(sizeof(int) * info.count * info.width);
  ch5m_elem_get_all(hdf_file, elements);
  
  for (int i = 0; i < info.count; i++) {

    _vol[ne] = NULL;
    
    switch (elements[i * info.width + CH5_ELEM_TYPE_OFFSET]) {
      case CH5_TETRAHEDRON:
        _vol[ne] = new Tetrahedral(&pt);
        break;
      
      case CH5_PYRAMID:
        _vol[ne] = new Pyramid(&pt);
        break;
      
      case CH5_PRISM:
        _vol[ne] = new Prism(&pt);
        break;
      
      case CH5_HEXAHEDRON:
        _vol[ne] = new Hexahedron(&pt);
        break;
      
      // As in read_elem_file, other primitives are ignored
      case CH5_TRIANGLE:
      case CH5_QUADRILATERAL:
        _numVol--;
        surfe++;
        break;
      
      // As in read_elem_file, unsupported types cause the function to clean
      // up and return
      default:
        cerr << "Unsupported element type (ch5m_element_type): " <<
          elements[i * info.width] << endl;
    	  delete[] _vol;
    	  _vol = NULL;
    	  _numVol = 0;
        return false;
    }
    if ( _vol[ne] )
      _vol[ne++]->add(&elements[i * info.width + CH5_ELEM_DATA_OFFSET], 
              elements[i * info.width + CH5_ELEM_REGION_OFFSET]);
  }

  free(elements);

  // Shrink-fit elements array
  VolElement **nv = new VolElement*[_numVol]; 
  memcpy(nv, _vol, _numVol * sizeof(VolElement*));
  delete[] _vol;
  _vol = nv;

  return true;
}
#endif


/** find the local surface element from the global number
 *
 * \param gele global element number
 * \param surf surface containing the element
 *
 * \return the local element number in the surface,-1 if not in range
 */
int Model::localElemnum( int gele, int& surf )
{
  if( gele<0 )
    return -1;

  surf=0; 
  while( surf<_surface.size() && _surface[surf]->num()<=gele ) 
    gele -= _surface[surf++]->num();

  return surf==_surface.size() ? -1 : gele;
}



/** compute tyhe global element number from the local
 *
 * \param surf surface containing the element
 * \param lnum local element number
 *
 * \return the local element number in the surface,-1 if not in range
 */
int Model::globalElemnum( int surf, int lnum )
{
  if( !surf && !lnum ) 
    return 0;

  if( surf<0 || surf>=_surface.size() )
    return -1;

  int gnum = 0; 
  for( int i=0; i<surf; i++ )
    gnum += _surface[i]->num();

  return gnum+lnum;
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
bool Model :: read_instance( gzFile pt_in, gzFile elem_in )
{
  // read in points
  if( pt_in != NULL ) {
    int num_pt;
    sscanf( get_line(pt_in), "%d", &num_pt );
    GLfloat *p = new GLfloat[num_pt*3];
    for( int i=0; i< num_pt; i++ ) {
      sscanf( get_line(pt_in), "%f %f %f", p+i*3, p+i*3+1, p+i*3+2 );
    }
    pt.add( p, num_pt );
    pt.setVis( true );
    delete[] p;
  }

  // read in elements
  _surface.push_back( new Surfaces( &pt ) );
  _cnnx = new Connection( &pt );
  if( elem_in != NULL ) {
    int num_elem;
	sscanf( get_line(elem_in), "%d", &num_elem );
	int *Cxpt = NULL, numCx=0;
	_vol = new VolElement*[num_elem];

	int n[10];
	for( int i=0; i<num_elem; i++ ) {
      char type[10];
      sscanf( get_line(elem_in),"%s %d %d %d %d %d %d %d %d %d %d",type,
			  n, n+1, n+2, n+3, n+4, n+5, n+6, n+7, n+8, n+9 );
      if( !strcmp( type, "Ln") ) {
        Cxpt = (int *)realloc( Cxpt, (numCx+1)*sizeof(int)*2+2 );
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


bool
Model::check_element( SurfaceElement *e )
{
  const int *n=e->obj();
  for( int i=0; i<e->ptsPerObj(); i++ )
    if( n[i] >= pt.num() )
      return false;

  return true;
}


/** get maximum size of object in model 
 *
 * \param o object
 *
 * \return maximum size
 */
float
Model::max_size( Object_t o )
{
  float ms = _region[0]->size(o);
  for( int i=1; i<_numReg; i++ ) 
    if( _region[0]->size(o)>ms ) ms = _region[0]->size(o);
  return ms;
}


#ifdef USE_HDF5
/** read in one instant in time which is part of a element  time file
 *
 *  \param hid_t     already open HDF5 file
 *  \param indx      grid index 
 *  \param tm        time to fetch
 *  \param data[out] data buffer which will be allocated
 *
 *  \return true on success, falsoe o.w.
 */
bool Model :: read_instance( hid_t hin, unsigned int indx, unsigned int tm,
                                      float* &data )
{
  ch5s_aux_time_step info;
  ch5s_aux_time_step_info( hin, indx, tm, &info );

  GLfloat *p   = new GLfloat[info.num_points*3];
  int  datawidth = info.max_element_width+2; // 1 for element type+1 for region

  int *ele = info.num_elements ? 
              new int[info.num_elements*datawidth] : NULL;
  data = info.has_data ? new float[info.num_points] : NULL ;

  ch5s_aux_read( hin, indx, tm, p, reinterpret_cast<unsigned int *>(ele), data);

  pt.add( p, info.num_points );
  pt.setVis( true );
  delete[] p;

  // make elements
  _surface.push_back( new Surfaces( &pt ) );
  _cnnx = new Connection( &pt );
  if( info.num_elements ) {
	int *Cxpt = NULL, numCx=0;
	_vol = new VolElement*[info.num_elements];

	for( int i=0; i<info.num_elements; i++ ) {
      ch5m_element_type etype = 
                    static_cast<ch5m_element_type>(ele[i*datawidth]);
      if( etype==CH5_CONNECTION ) {
        Cxpt = (int *)realloc( Cxpt, (numCx+1)*sizeof(int)*2+2 );
        Cxpt[numCx*2]   = ele[i*datawidth+2];
        Cxpt[numCx*2+1] = ele[i*datawidth+3];
        numCx++;
      } else if( etype==CH5_TRIANGLE ) {
        _surface.back()->ele().push_back( new Triangle( &pt ) );
        _surface.back()->ele().back()->define( ele+i*datawidth+2 );
      } else if( etype==CH5_QUADRILATERAL ) {
        _surface.back()->ele().push_back( new Quadrilateral( &pt ) );
        _surface.back()->ele().back()->define( ele+i*datawidth+2 );
        _surface.back()->ele().back()->compute_normals(0,0);
      } else if( etype==CH5_TETRAHEDRON ){
        _vol[_numVol] = new Tetrahedral( &pt );
        _vol[_numVol]->add( ele+i*datawidth+2 );
        _numVol++;
      } else if( etype==CH5_HEXAHEDRON ){
        _vol[_numVol] = new Hexahedron( &pt );
        _vol[_numVol]->add( ele+i*datawidth+2 );
        _numVol++;
      } else if( etype==CH5_PYRAMID ){
        _vol[_numVol] = new Pyramid( &pt );
        _vol[_numVol]->add( ele+i*datawidth+2 );
        _numVol++;
      } else if( etype==CH5_PRISM ){
        _vol[_numVol] = new Prism( &pt );
        _vol[_numVol]->add( ele+i*datawidth+2 );
        _numVol++;
      } else {
        //fl_alert( "\nUnknown element type: %d\n\n", etype );
        delete[] _vol;
        return false;
      }
    }
    // define connections
    _cnnx->define( Cxpt, numCx );
    free( Cxpt );
  }
  determine_regions();
  return true;
}
#endif
