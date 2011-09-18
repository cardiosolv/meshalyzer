/** \file Wrap up disparate data files into one HDF5 file
 */
#include <iostream>
#include <sstream>
#include <cstdio>
#include <ctype.h>
#include <string.h>
#include "IGBheader.h"
#include "ch5.h"
#include "cmdline.h"
using namespace std;

/** macro to check for possibly gzipped filename extension
 *
 *  \param A the flename
 *  \param B the extension
 */
#define GZIPPED_OR_NOT(A,B) \
    ( strstr(A+strlen(A)-strlen(B),   B      ) || \
    strstr(A+strlen(A)-strlen(B)-3, B".gz" )     )

#define OPT_START(A,B) (!strncmp(A,B,strlen(B)))

#define LABEL      "l",  'l'
#define UNITS      "u",  'u'
#define TM_UNITS   "tm", 's'
#define QUANTITIES "q",  'q'
#define DT         "dt", 'd'
#define T0         "t0", 'o'

/* try opening a gzipped file
 *
 * \param fn filename
 */
gzFile
open_gzip( const char *fn )
{
  gzFile f = gzopen( fn, "r" );
  if( f==NULL ) {
    printf( "Error opening file: %s\n\n", fn );
    exit(1);
  }
  return f;
}


/** return the first line not containing data, ignore commented lines
 *
 * \param in  input file
 * \param buf buffer
 * \param sz  size of buf
 *
 * \return NULL if error or EOF, buf o.w.
 * \post \p buf will contain data
 */
char *scan_line( gzFile in, char *buf, int sz )
{
  int i;
  do {
    if( gzgets( in, buf, sz )== NULL )
      return NULL;
    i=0;
    while( isspace(buf[i]) )
      i++;
  } while( buf[i]=='#' );
  return buf;
}


/** open an associated file which has the same basename but different suffix
 *  and possibly gzipped
 *  
 *  \note each file may or may not be gzipped
 *
 * \param fn          original filename
 * \param oldext      old extension
 * \param newext      new extension
 * \param[out] new_fn if requested, new filename
 *
 * \return file pointer to assciated file, NULL on failure
 */
gzFile
associated_file( const char *fn, const char *oldext, const char *newext,
                                             char ** new_fn=NULL )
{
  string vfn = fn;
  // remove ".gz" if present
  if( vfn.find( ".gz", vfn.size()-3 ) != string::npos )
    vfn = vfn.substr(0,vfn.size()-3);
  // remove old extension and add the new
  vfn = vfn.substr(0, vfn.size()-strlen(oldext)) + newext;

  gzFile vin = gzopen( vfn.c_str(),"r" );
  if( vin==NULL )
    vin  = gzopen( (vfn+=".gz").c_str(), "r" );
  
  if( new_fn )
    *new_fn = strdup( vfn.c_str() );
  return vin;
}


/** return the position of an option in word list
 *
 * \param argc number of words
 * \param argv words
 * \param opt  target option
 *
 * \return index if found, -1 o.w.
 */ 
int
find_index( int argc, char **argv, const char *opt)
{ 
  for( int i=1; i<argc; i++ ) {
    if( strstr( argv[i], opt ) )
      return i;
  }
  return -1;
}


/** determine if a new data file specified in option list
 *
 * \param opt option being examined
 *
 * \return true if a new file keyword, false o.w.
 */
bool
file_start( const char *opt )
{
  if( OPT_START(opt,"--no") ||   // --nodal
      OPT_START(opt,"-n")   ||
      OPT_START(opt,"--v")  ||   // --vector
      OPT_START(opt,"-v")   ||    
      OPT_START(opt,"--ti") ||   // --time-series
      OPT_START(opt,"-T")   || 
      OPT_START(opt,"--au") ||   // --auxgrid
      OPT_START(opt,"-a")   ||
      OPT_START(opt,"--dy") ||   // --dynpts
      OPT_START(opt,"-d")   ||
      OPT_START(opt,"--te") ||   // --text
      OPT_START(opt,"-s")     )
    return true;
  else
    return false;
}


/** return the option value if present for specified file
 *
 * \param long    long version of option
 * \param short   short option version
 * \param argc    number of words
 * \param argv    words
 * \param fn_index[in/out] on input, index file under consideration,
 *                         on output, word in which opton value found
 * \param empty   on failure, return an empty string instead of NULL
 *
 * \return option if present,\p NULL or "" o.w.
 * \Note   on failure, \p fn_index is unchanged
 * \note   searching begins at the word \p fn_index+1
 */
const char*
get_option_val( const char *lng, char shrt, int argc,
                             char **argv, int &fn_index, bool empty=false )
{
  if( fn_index==-1 )
    return NULL;

  char lg[1024];
  sprintf( lg, "--%s", lng );
  char sh[1024];
  sprintf( sh, "-%c", shrt );

  for( int i=fn_index+1; i<argc; i++ ) {
    if( file_start( argv[i] ) )
      return empty?"":NULL;
    if( OPT_START( argv[i], lg ) ) { // --l
      if( index( argv[i], '=' ) ){   // --l=
        fn_index = i;
        return index( argv[i], '=' )+1;
      } else {
        return argv[i+1];            // --l by itself
      }
    }
    if( !strcmp( argv[i], sh ) ) {// -s by itself
      fn_index = i+1;
      return argv[i+1];
    }
    if( OPT_START( argv[i], sh ) ) {// -s(opt)
      fn_index = i;
      return argv[i]+2;
    }
  }
  return empty?"":NULL;
}


/** count noncommented lines in a file 
 *
 * \param fn filename
 *
 * \return number of lines
 *
 * \note commented lines have '#" as the first byte and are not counted
 */
int
count_lines( const char *fn )
{
  gzFile in = open_gzip( fn );
  int count = 0;

  char buf[2048];
  while( gzgets( in, buf, 2048 ) != NULL && buf[0]!='#' )
    count++;
  gzclose( in );

  return count;
}


/** return the number of nodes, going right, and if none, going left
 *
 * \param argc number of words
 * \param argv words
 * \param fn   fle under consideration
 *
 * \return number of nodes if found, -1 o.w.
 */
int
find_numnodes( int argc, char **argv, const char* fn )
{
  for( int i=find_index(argc, argv, fn)+1; i<argc; i++ ) {
    if( file_start( argv[i] ) )
      break;
    if( OPT_START( argv[i], "--nu") ) // --numnodes=
      return atoi(index( argv[i], '=' )+1);
    if( !strcmp( argv[i], "-N" ) )
      return atoi( argv[i+1] );
    if( OPT_START( argv[i], "-N" ) )
      return atoi(argv[i]+2);
  }
  for( int i=find_index(argc, argv, fn)-1; i>0; i-- ) {
    if( OPT_START( argv[i], "--nu") )
      return atoi(index( argv[i], '=' )+1);
    if( !strcmp( argv[i], "-N" ) )
      return atoi( argv[i+1] );
    if( OPT_START( argv[i], "-N" ) )
      return atoi(argv[i]+2);
  }
  return -1;
}


/** put an IGB file into an HDF5 file
 *
 * \param h5out
 * \param fn
 * \param argc
 * \param argv
 */
void
add_nodal_igb( hid_t h5out, const char *fn, const char* label, 
                                                   ch5s_nodal_type type ) 
{
  gzFile in = open_gzip( fn );
  IGBheader h(in);
  if( h.read() )
    exit(1);

  int grid_i =  
     ch5s_nodal_create_grid( h5out, h.x()*h.y()*h.z(), h.t(), h.org_t(),
             h.inc_t(), type, h.unites_t(), label,  h.unites(), NULL );
  if( grid_i < 0 ) {
    printf( "Cannot create grid: %s\n\n", fn );
    exit(1);
  }

  float *data = new float[h.x()*h.y()*h.z()*h.data_size()/sizeof(float)];
  for( int i=0; i< h.t(); i++ ) {
    read_IGB_data( data, 1, &h, NULL );
    ch5s_nodal_write( h5out, grid_i, i, i, data );
  }
  delete[] data;
  gzclose(in);
}


/** write data from an ASCII data file into the HDF5 file
 * 
 * \param h5out  HDF% file
 * \param fn     filename
 * \param argc
 * \param argv
 */
void
add_nodal_tdat( hid_t h5out, const char *fn, int argc, char **argv ) 
{
  int nnodes=find_numnodes( argc, argv, fn );
  if( nnodes<1 ) {
    printf( "\nError: numnodes not specified for non-IGB nodal file: %s", fn);
    exit(1);
  }  
  int numt = count_lines( fn )/nnodes;

  int fi, fn_i = find_index( argc, argv, fn );
  const char *units   = get_option_val( UNITS,    argc, argv, fi=fn_i ); 
  const char *label   = get_option_val( LABEL,    argc, argv, fi=fn_i ); 
  const char *t_units = get_option_val( TM_UNITS, argc, argv, fi=fn_i ); 
  float       t0      = atof(get_option_val( T0, argc, argv, fi=fn_i, true )); 

  int grid_i = ch5s_nodal_create_grid( h5out, nnodes, numt, t0, 1, CH5_SCALAR,
                                      label, t_units, units, NULL );

  if( grid_i < 0 ) {
    printf( "Cannot create grid: %s\n\n", fn );
    exit(1);
  }

  gzFile data_in = open_gzip( fn );
  float *data_buffer = new float[nnodes];
  char buf[2048];

  for( int i=0; i<numt; i++ ) {
    for( int j=0; j<nnodes; j++ ) {
      gzgets( data_in, buf, 2048 );
      sscanf( buf, "%f", data_buffer+j );
    }
    ch5s_nodal_write( h5out, grid_i, i, i, data_buffer );
  }
  gzclose( data_in );
  delete[] data_buffer;
}


/** write vector data to HDF5 file
 *
 * \param h5out  HDF5 data file
 * \param fn     data file (.vpts)
 * \param argc
 * \param argv
 */
void
add_vector_data( hid_t h5out, const char *fn, int argc, char **argv ) 
{
  int fi, fn_ind = find_index( argc, argv, fn );
  char buf[2048];
  
  // units may appear twice, first for vector then scalar data
  const char* v_units = get_option_val( UNITS, argc, argv, fi=fn_ind ); 
  const char *s_units = NULL;
  if( v_units )
    s_units = get_option_val( UNITS, argc, argv, fi ); 

  // labels may appear twice, first for vector then scalar data
  const char* vlabel = get_option_val( LABEL, argc, argv, fi=fn_ind );
  const char* slabel = NULL;
  if( vlabel )
    slabel = get_option_val( LABEL, argc, argv, fi );

  // open point file and get the number of nodes
  gzFile pt_in = open_gzip( fn );
  int numnodes;
  gzgets( pt_in, buf, 2048 );
  sscanf( buf, "%d", &numnodes );

  // open vector data file
  char *vec_fn=NULL;
  gzFile vin;
  if(!(vin=associated_file( fn, "vpts", "vec", &vec_fn ))) {
    printf( "Error: no matching vector file for pts file: %s\n\n", vec_fn );
    exit(1);
  }

  bool is_igb=false;
  int num_t, dt, num_data;
  const char* t_units;
  float t0;

  // determine whether vec file is IGB or ASCII
  IGBheader h(vin);
  if( h.read() ) {
    gzrewind( vin );
    num_t    = count_lines( vec_fn )/numnodes;
    dt       = atof(get_option_val( DT, argc, argv, fi=fn_ind, true )); 
    t_units  = get_option_val( TM_UNITS, argc, argv, fi=fn_ind ); 
    gzgets( vin, buf, 2048 );
    float a[4];
    num_data = sscanf( buf, "%f %f %f %f", a, a+1, a+2, a+3 );
    gzrewind( vin );
  } else {
    is_igb = true;
    num_t   = h.t();
    dt      = h.inc_t();
    t0      = h.org_t();
    t_units = h.unites_t();
    num_data= h.type()==IGB_VEC3_f ? 3 : 4;
  }
  if( vec_fn ) free( vec_fn );

  if( num_data==4 && !slabel )
    slabel = "scalar data";

  // output nodal positions
  float  *pt = new float[3*numnodes];
  for( int i=0; i<numnodes; i++ ) {
    scan_line( pt_in, buf, 2048 );
    sscanf( buf, "%f %f %f", pt+i*3, pt+3*i+1, pt+i*3+2 );
  }

  int grid_i =  ch5s_vector_create_grid( h5out, numnodes, num_t, t0, dt, pt, 
                  vlabel, num_data==3? NULL : slabel, t_units, v_units, NULL );  
  delete[] pt;

  //output vector data
  float *data = new float[numnodes*num_data];
  for( int i=0; i< num_t; i++ ) {
    if( is_igb ) {
      read_IGB_data( data, 1, &h, NULL );
    } else {
      float a[4];
      for( int j=0; j<numnodes; j++ ) {
        scan_line( vin, buf, 2048 );
        sscanf( buf, "%f %f %f %f", a, a+1, a+2, a+3 );
        for( int k=0; k<num_data; k++ )
          data[j*num_data+k] = a[k];
      }
    }
    ch5s_vector_write( h5out, grid_i, i, i, data );
  }
  delete[] data;

  gzclose( vin );
}


void
add_auxgrid( hid_t h5out, const char *fn, int argc, char **argv ) 
{
  int fi, fn_i = find_index( argc, argv, fn );
  const char *units    = get_option_val( UNITS, argc, argv, fi=fn_i ); 
  const char *tm_units = get_option_val( TM_UNITS, argc, argv, fi=fn_i ); 
  const char *label    = get_option_val( LABEL, argc, argv, fi=fn_i ); 
  float       dt       = atof(get_option_val( DT, argc, argv, fi=fn_i, true )); 
  float       t0       = atof(get_option_val( T0, argc, argv, fi=fn_i, true )); 
  
  gzFile ptin = open_gzip( fn );
  char buf[2048];
  scan_line( ptin, buf, 2048 );
  int num_t;
  sscanf( buf, "%d", &num_t );

  int ai = ch5s_aux_create_grid( h5out, t0, dt, label, tm_units, units, NULL );

  gzFile elem_in = associated_file( fn, "pts_t", "elem_t" );
  gzFile data_in = associated_file( fn, "dat_t", "dat_t"  );
  if( elem_in )
    scan_line( elem_in, buf, 2048 ); // throw away number of times
  if( data_in )
    scan_line( data_in, buf, 2048 ); // throw away number of times;

  for( int i=0; i<num_t; i++ ) {
    
    int             np, ne    = 0;
    float          *dat       = NULL;
    unsigned int   *ele_lst   = NULL;
    int             max_width = 0;

    scan_line( ptin, buf, 2048 );
    sscanf( buf, "%d", &np );
    float *pt = new float[3*np];
    for( int j=0; j<np; j++ ) {
      scan_line( ptin, buf, 2048 );
      sscanf( buf, "%f %f %f", pt+j*3, pt+j*3+1, pt+j*3+2 );
    }

    if( elem_in ) {
      int max_sz=0, cur_sz=0;
      scan_line( elem_in, buf, 2048 );
      sscanf( buf, "%d", &ne );
      for( int j=0; j<ne; j++ ) {
        scan_line( elem_in, buf, 2048 );
        istringstream oss( buf );
        string type;
        oss >> type;
        ch5m_element_type itype = 
          static_cast<ch5m_element_type>(ch5m_elem_get_type_by_prefix( type.c_str()));
        int sz=ch5m_elem_get_width_for_type(itype);
        if( sz>max_width )
          max_width = sz;
        if( cur_sz+sz+1>max_sz ) {
          max_sz += 1000000;
          ele_lst = (unsigned int *)realloc( ele_lst, max_sz*sizeof(int) );
        }
        ele_lst[cur_sz++] = itype;
        for(int k=0; k<sz; k++)
          oss >> ele_lst[cur_sz++];
      }
    }

    if( data_in ) {
      int nd;
      scan_line( data_in, buf, 2048 );
      sscanf( buf, "%d", &nd );
      if( nd != np ) {
        printf( "Aux grid #points != #data: %s\n\n", fn );
        exit(1);
      }
      dat = new float[np];
      for( int j=0; j<nd; j++ ) {
        scan_line( data_in, buf, 2048 );
        sscanf( buf, "%f", dat+j );
      }
    }

    ch5s_aux_write_next( h5out, ai, np, pt, ne, max_width, ele_lst, dat );

    free( ele_lst );
    delete[] pt;
    delete[] dat;
  }

  gzclose( ptin );
  if( elem_in )
    gzclose( elem_in );
  if( data_in )
    gzclose( data_in );
}


/** add free format text to the data file
 *
 * \param h5out HDF5 file
 * \param fn    data file
 * \param argc  number of words
 * \param argv  word list
 */
void
add_text( hid_t h5out, const char *fn, int argc, char **argv ) 
{
  int fi, fn_i = find_index( argc, argv, fn );
  const char *label = get_option_val( LABEL, argc, argv, fi=fn_i ); 

  gzFile in = open_gzip( fn );
  int ti = ch5_text_create( h5out, label );

  const int chunk=1000001;
  int       nread;
  char *buf = new char[chunk];
  do { 
    nread=gzread( in, buf, chunk-1);
    buf[nread] = '\0';
    ch5_text_append( h5out, ti, buf );
  } while( nread==chunk );

  gzclose( in );
}


/** add a timeseries to the data file
 *
 * \param h5out HDF5 file
 * \param fn    data file
 * \param argc  number of words
 * \param argv  word list
 */
void
add_timeseries( hid_t h5out, const char *fn, int argc, char **argv ) 
{
  int fi, fn_i = find_index( argc, argv, fn );
  const char *units = get_option_val( UNITS, argc, argv, fi=fn_i ); 
  const char *label = get_option_val( LABEL, argc, argv, fi=fn_i ); 
  const char *quant = get_option_val( QUANTITIES, argc, argv, fi=fn_i ); 

  // determine number of columns in data
  gzFile in = open_gzip( fn );
  bool white     = true; // true if last char read whitespace
  int  numfields = 0;
  int  byte;
  while( (byte=gzgetc(in)) != '\n' && byte!='#' ) {
    bool read_white = isspace( byte );
    if( white && !read_white ) 
      numfields++;
    white = read_white;
  } 
  gzrewind( in );

  int num_t = count_lines(fn);

  int si = ch5s_series_create( h5out, num_t, numfields-1, label, quant, 
                                                             units, NULL );
  
  for( int i=0; i<num_t; i++ ) {
    double data[numfields];
    char   buf[2048];
    gzgets( in, buf, 2048 );
    char *lr=buf;
    for( int j=0; j<numfields; j++ ) {
     int nr;
     sscanf( lr, "%lf %n", data+j, &nr );
     lr += nr;
    }
    ch5s_series_write( h5out, si, i, i, data );
  }
  gzclose( in );
}


/** store original command line 
 *
 * \param argc 
 * \param argv
 *
 * \return a copy of argv
 */
char ** 
store_orig( int argc, char **argv )
{
  char **o = new char*[argc];
  for( int i=0; i<argc; i++ )
    o[i] = strdup( argv[i] );
  return o;
}


int 
main( int argc, char *argv[] )
{
  gengetopt_args_info args_info;

  char **orig = store_orig( argc, argv );

  // let's call our cmdline parser 
  if (cmdline_parser (argc, argv, &args_info) != 0)
    exit(1);

  if( argc == 1 || args_info.inputs_num != 1 ) {
    cmdline_parser_print_help();
    exit(0);
  }

  hid_t hdf_out;
  if( args_info.append_flag )
    ch5_open( args_info.inputs[0], &hdf_out );
  else
    ch5_create( args_info.inputs[0], &hdf_out );

  for( unsigned int i=0; i<args_info.nodal_given; i++ ) {
    int fi = find_index( argc, orig, args_info.nodal_orig[i]);
    const char*lbl = get_option_val( LABEL, argc, orig, fi );
    if( GZIPPED_OR_NOT( args_info.nodal_arg[i], "igb") )
      add_nodal_igb( hdf_out, args_info.nodal_arg[i], lbl, CH5_SCALAR );
    else {
      add_nodal_tdat( hdf_out, args_info.nodal_arg[i], argc, argv );
    }
  }

  for( unsigned int i=0; i<args_info.vector_given; i++ ) {
     add_vector_data( hdf_out, args_info.vector_arg[i], argc, orig );
  }
                   
  for( unsigned int i=0; i<args_info.dynpts_given; i++ ) {
    int fi = find_index( argc, orig, args_info.dynpts_orig[i]);
    const char*lbl = get_option_val( LABEL, argc, orig, fi );
    add_nodal_igb( hdf_out, args_info.dynpts_arg[i], lbl, CH5_DYN_PTS );
  }
  
  for( unsigned int i=0; i<args_info.time_series_given; i++ ) {
    add_timeseries( hdf_out, args_info.time_series_arg[i], argc, argv );
  }

  for( unsigned int i=0; i<args_info.auxgrid_given; i++ ) {
    add_auxgrid( hdf_out, args_info.auxgrid_arg[i], argc, argv );
  }

  for( unsigned int i=0; i<args_info.text_given; i++ ) {
    add_text( hdf_out, args_info.text_arg[i], argc, argv );
  }

  ch5_close( hdf_out );

  return 0;
}

