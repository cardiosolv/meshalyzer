#ifndef DATAALLInMEM_H
#define DATAALLInMEM_H

#include "DataClass.h"
#include "IGBheader.h"
#include <zlib.h>
#include <string>
#include <typeinfo>
#ifdef USE_HDF5
#include "ch5/ch5.h"
#endif

/** reader for case when all data can fit in computer RAM at once */
template<class T>
class DataAllInMem : public DataClass<T>
{

    using DataClass<T> :: data;
    using DataClass<T> :: maxtm;
    using DataClass<T> :: last_tm;
    using DataClass<T> :: slice_size;
    using DataClass<T> :: filename;
    using DataClass<T> :: _dt;
    using DataClass<T> :: _t0;

  public:
    DataAllInMem( const char *fn, int, bool );
    ~DataAllInMem( );
    virtual T         max(int);	        // maximum value at a time instance
    virtual T         max();	        // maximum value
    virtual T         min(int);         // minimum value at a time
    virtual T         min();            // minimum value
    virtual T*        slice(int);       // return pointer to data slice
    virtual void      time_series( int, T* );  // time series for a point
    inline virtual void  increment(int){}};


template<class T>
T DataAllInMem<T>::max( int tm )
{
  if ( tm>maxtm )
    return 0;

  T* sliceptr = data + tm*slice_size;

  T maxval = sliceptr[0];
  for ( int i=1; i<slice_size; i++ )
    if ( sliceptr[i] > maxval ) maxval = sliceptr[i];

  return maxval;
}

// global maximum
template<class T>
T DataAllInMem<T>::max()
{
  T maxval = data[0];
  for ( int i=1; i<slice_size*(maxtm+1); i++ )
    if ( data[i] > maxval ) maxval = data[i];

  return maxval;
}


// global minimum
template<class T>
T DataAllInMem<T>::min()
{
  T minval = data[0];
  for ( int i=1; i<slice_size*(maxtm+1); i++ )
    if ( data[i] < minval ) minval = data[i];

  return minval;
}


template<class T>
T DataAllInMem<T>::min( int tm )
{
  if ( tm>maxtm )
    return 0;

  T* sliceptr = data + tm*slice_size;

  T minval = sliceptr[0];
  for ( int i=1; i<slice_size; i++ )
    if ( sliceptr[i] < minval ) minval = sliceptr[i];

  return minval;
}



template<class T>
T* DataAllInMem<T>::slice( int tm )
{
  if ( tm>maxtm )
    return NULL;
  return data+tm*slice_size;
}

template<class T>
void DataAllInMem<T>::time_series( int offset, T* buffer )
{
  for ( int i=0; i<=maxtm; i++ )
    buffer[i] = *(data + i*slice_size+offset);
}

#include<string>
#include<map>
/** open up a data file
 *
 *  Read all data into a single buffer
 *
 *  \param fn    the file name of the data file
 *  \param slsz  slice size
 *  \param base1 whether node numbering starts at one
 */
template<class T>
DataAllInMem<T>::DataAllInMem( const char *fn, int slsz, bool base1 )
{
  bool            IGBdata;
  int             j = 0;
  IGBheader       head;
  gzFile          in;
  string          fname=fn,
                        scanner;
  map<int,string> CGfiles;
  map<int,string>::iterator CGp;
#ifdef USE_HDF5
  hid_t                  hin;
  struct ch5s_nodal_grid info;
  unsigned int           index;
#endif

  _dt = 1;
  _t0 = 0;
  slice_size = slsz;

  fileType ftype=FileTypeFinder( fn );

  if ( ftype == FThdf5 )  {
#ifdef USE_HDF5
    if(ch5_open( fname.substr(0, fname.find_last_of(":")).c_str(), &hin ) )
      throw(1);
#endif
  } else {
    if ( (in=gzopen(fname.c_str(), "r")) == NULL ) {
      fname += ".gz";
      if ( (in=gzopen(fname.c_str(), "r")) == NULL )
        throw( 1 );
    }
  }

  filename = fn;

  // ugly but I don't know what else to do besides specialization which is ugly
  const char *scanstr;
  if (      typeid(T) == typeid(double) ) scanstr = "%lf";
  else if ( typeid(T) == typeid(float) )  scanstr = "%f";
  else if ( typeid(T) == typeid(int) )    scanstr = "%d";
  else if ( typeid(T) == typeid(short) )  scanstr = "%hd";
  else if ( typeid(T) == typeid(long) )   scanstr = "%ld";
  else exit(1);

  if ( ftype == FTIGB ) {
    head = IGBheader( in, true );
    if ( head.slice_sz() != slice_size ) {
      maxtm = -1;
      throw PointMismatch(slice_size,head.slice_sz());
    }
    _dt = head.inc_t();
    _t0 = head.org_t();
  } else if ( ftype == FTfileSeqCG ) {
    CG_file_list( CGfiles, fn );
    CGp = CGfiles.begin();
    scanner = "%*f %*f";
    scanner += scanstr;
  } else if( ftype == FThdf5 ) {
    string gtype;
#ifdef USE_HDF5
    if( parse_HDF5_grid( fn, gtype, index ) || gtype!="nodal" )
      throw(1);
    ch5s_nodal_grid_info( hin, index, &info );
    _dt = info.time_delta;
#endif
    _t0 = 0;
  }

  // read in data, one time slice at a time
  const int bufsize = 1024;
  char* buff = new char[bufsize];
  do {

    data = (T *)realloc( data, (maxtm+1)*slice_size*sizeof(T) );

    int i;

    switch ( ftype ) {
      case FTfileSeqCG:
        gzclose( in );
        in = gzopen( CGp->second.c_str(), "r" );
        gzgets(in, buff, bufsize ); // throw away time line
        gzgets(in, buff, bufsize ); // we need to record this datum twice
        sscanf( buff, scanner.c_str(), data+j );
        data[j+1] = data[j];
        j += 2;
        for ( i=2; i<slice_size; j++, i++ ) {
          if ( gzgets(in, buff, bufsize) == Z_NULL ) break;
          if ( sscanf( buff, scanner.c_str(), data+j ) != 1 ) break;
        }
        break;
      case FTascii:
        for ( i=0; i<slice_size; j++, i++ ) {
          if ( gzgets(in, buff, bufsize) == Z_NULL ) break;
          if ( sscanf( buff, scanstr, data+j ) != 1 ) break;
        }
        break;
      case FTIGB:
        i = read_IGB_data( data+maxtm*slice_size+base1, 1, &head );
        if ( base1 ) {
          data[maxtm*slice_size] = data[maxtm*slice_size+1];
          i++;
        }
        break;
      case FThdf5:
#ifdef USE_HDF5
        data = (T *)realloc( data, info.time_steps*slice_size*sizeof(T) );
        maxtm = info.time_steps;
        i = ch5s_nodal_read( hin, index, 0, info.time_steps-1, data );
#else
        assert(0);
#endif
        break;
    }

    if ( (ftype==FTIGB||ftype==FTascii) && i!=slice_size )
      break;

    if ( ftype==FThdf5 )
      break;

    maxtm++;

    if ( ftype==FTfileSeqCG && ++CGp==CGfiles.end() ) {
      gzclose( in );
      break;
    }
    if ( ftype==FTIGB && maxtm==head.t() )
      break;

  } while (1);
  delete[] buff;
  maxtm--;

  if ( maxtm == -1 ) {
    free( data );
    if (ftype == FTfileSeqCG ) CGfiles.~map();
    throw( 1 );
  }

#ifdef USE_HDF5
  if ( ftype==FThdf5 ) {
    ch5_close( hin );
    ch5s_nodal_free_grid_info( &info );
  }
#endif
}

template<class T>
DataAllInMem<T>::~DataAllInMem()
{
  free( data );
}

#endif
